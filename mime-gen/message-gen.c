/*
 *  Part of "MG" - a MIME generating DSL
 *
 *  Copyright 2011 Don Kelly <karfai@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include <string.h>
#include <glib.h>
#include <gmime/gmime.h>

typedef enum {
    at_Invalid,
    at_Message,
    at_Text,
    at_Header,
    at_Att,
    at_Empty,
    at_Multi,
} AtomT;

typedef struct {
    AtomT   type;
    guint   level;
    gchar** args;
} Atom;

static Atom* atom_new(AtomT t, guint l, gchar** args)
{
    Atom* rv = g_new0(Atom, 1);
    rv->type = t;
    rv->level = l;
    rv->args = args;

    return rv;
}

static void atom_free(Atom* a)
{
    g_strfreev(a->args);
    g_free(a);
}

static AtomT identify(const gchar* s)
{
    static struct {
        const gchar* s;
        AtomT        t;
    } id_tbl[] = {
        { "HDR",  at_Header },
        { "TEXT", at_Text },
        { "MSG",  at_Message },
        { "ATT",  at_Att },
        { "MLT",  at_Multi },
        { "",     at_Empty },
    };
    AtomT rv = at_Invalid;

    guint i = 0;
    for ( ; at_Invalid == rv && i < sizeof(id_tbl) / sizeof(*id_tbl); i++ ) {
        if ( 0 == g_strcmp0(s, id_tbl[i].s) ) {
            rv = id_tbl[i].t;
        }
    }
    if ( at_Invalid == rv ) {
        g_warning("'%s' unrecognized", s);
    }
}

static gchar** build_args(const char* s)
{
    return s ? g_strsplit(s, ";", -1) : NULL;
}

static Atom* parse_line(const gchar* ln)
{
    static GRegex* re = NULL;
    GMatchInfo*    mi = NULL;
    Atom*          rv = NULL;

    if ( !re ) {
        re = g_regex_new("(-*)([A-Z]+)(?:\\((.+)*\\))?", G_REGEX_OPTIMIZE, G_REGEX_MATCH_NOTEMPTY, NULL);
    }

    if ( g_regex_match(re, ln, 0, &mi) ) {
        gchar* dashes = g_match_info_fetch(mi, 1);

        rv = atom_new(
            identify(g_match_info_fetch(mi, 2)),
            dashes ? strlen(dashes) : 0,
            build_args(g_match_info_fetch(mi, 3)));

        g_match_info_free(mi);
    }
    
    return rv;
}

static GSList* parse(const gchar* fn)
{
    GMappedFile* f = g_mapped_file_new(fn, FALSE, NULL);
    GSList*      atoms = NULL;

    if ( f ) {
        gchar** lns = g_strsplit(g_mapped_file_get_contents(f), "\n", -1);
        guint   i = 0;
        
        for ( ; NULL != lns[i]; i++ ) {
            const gchar* ln = g_strstrip(lns[i]);
            Atom*        a = parse_line(ln);

            if ( a ) {
                atoms = g_slist_append(atoms, a);
            }
        }

        g_strfreev(lns);
        g_mapped_file_free(f);
    }
    
    return atoms;
}

static GMimeStream* create_stream(GMimeContentEncoding en, const gchar* fn)
{
    GMimeStream* rv = NULL;
    FILE*        f = fopen(fn, "r");
    GMimeStream* fst = NULL;

    if ( f ) {
        fst = g_mime_stream_file_new(f);
    }

    if ( fst ) {
        switch (en) {
            case GMIME_CONTENT_ENCODING_DEFAULT:
            {
                rv = fst;
            }
            break;
            
            default:
            {
                GMimeFilter* flt = g_mime_filter_basic_new(en, TRUE);
                
                rv = g_mime_stream_filter_new(fst);
                g_mime_stream_filter_add(GMIME_STREAM_FILTER(rv), flt);
                g_object_unref(flt);
                g_object_unref(fst);
            }
            break;
        }
    } else {
        g_warning("failed to open '%s'", fn);
    }

    return rv;
}

static void add_content(GMimeContentEncoding en, const gchar* fn, GMimePart* pt)
{
    GMimeStream* st = create_stream(en, fn);

    if ( pt ) {
        GMimeDataWrapper* co = g_mime_data_wrapper_new_with_stream(st, en);

        g_mime_data_wrapper_set_encoding(co, en);
        g_mime_part_set_content_object(pt, co);
        g_object_unref(co);
    }

    g_object_unref(st);
}

static GMimeObject* add_part_to_existing(GMimeObject* mp, GMimeObject* p)
{
    GMimeObject* rv = mp;
    /* there was a previous part, switch to multipart or just add if we already switched */
    /* g_debug("mp=%p (%s), p=%p (%s)", mp, G_OBJECT_TYPE_NAME(mp), p, G_OBJECT_TYPE_NAME(p)); */
    if ( GMIME_IS_MULTIPART(mp) ) {
        g_mime_multipart_add(GMIME_MULTIPART(mp), p);
    } else {
        GMimeMultipart* mlp = g_mime_multipart_new();

        g_mime_multipart_add(mlp, mp);
        g_mime_multipart_add(mlp, p);
        rv = GMIME_OBJECT(mlp);
    }

    return rv;
}

static void add_part(GSList** stack, GMimeObject* p)
{
    GMimeObject* top_part = GMIME_OBJECT((*stack)->data);

    if ( GMIME_IS_MESSAGE(top_part) ) {
        GMimeMessage* m = GMIME_MESSAGE(top_part);
        GMimeObject*  msg_part = g_mime_message_get_mime_part(m);
        GMimeObject*  add_part = p;
        if ( msg_part ) {
            add_part = add_part_to_existing(msg_part, p);
        }
        
        g_mime_message_set_mime_part(m, add_part);
    } else {
        (*stack)->data = add_part_to_existing(top_part, p);
    }    
}

static void interpret_header(GSList** stack, gchar** args)
{
    GMimeObject* o = GMIME_OBJECT((*stack)->data);
    if ( 0 == g_strcmp0("Date", args[0]) && GMIME_IS_MESSAGE(o) ) {
        g_mime_message_set_date_as_string(GMIME_MESSAGE(o), g_strchug(args[1]));
    } else {
        g_mime_object_set_header(o, args[0], g_strchug(args[1]));
    }
}

static void interpret_text(GSList** stack, gchar** args)
{
    GMimePart* pt = g_mime_part_new();

    g_mime_part_set_content_encoding(pt, GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE);
    g_mime_object_set_content_disposition(
        GMIME_OBJECT(pt), 
        g_mime_content_disposition_new_from_string(GMIME_DISPOSITION_INLINE));

    add_content(GMIME_CONTENT_ENCODING_QUOTEDPRINTABLE, args[0], pt);
    add_part(stack, GMIME_OBJECT(pt));
}

static void interpret_message(GSList** stack, gchar** args)
{
    *stack = g_slist_prepend(*stack, g_mime_message_new(FALSE));
}

static void stack_multipart(GSList** stack, const char* subtype)
{
    GMimeMultipart* mp = subtype ? g_mime_multipart_new_with_subtype(subtype) : g_mime_multipart_new();
    *stack = g_slist_prepend(*stack, mp);
    /* g_debug("mp=%p", mp); */
    /* g_debug("ct=%s", g_mime_content_type_to_string(g_mime_object_get_content_type(GMIME_OBJECT(mp)))); */
    /* g_debug("b=%s", g_mime_multipart_get_boundary(mp)); */
}

static void implicit_level_increase(GSList** stack)
{
    /* g_debug("implicit"); */
    stack_multipart(stack, NULL);
}

static void interpret_att(GSList** stack, gchar** args)
{
    GMimePart* pt = g_mime_part_new_with_type(args[1], args[2]);
    g_mime_part_set_filename(pt, args[0]);
    g_mime_part_set_content_encoding(pt, GMIME_CONTENT_ENCODING_BASE64);
    g_mime_object_set_content_id(GMIME_OBJECT(pt), args[0]);
    add_content(GMIME_CONTENT_ENCODING_BASE64, args[0], pt);

    add_part(stack, GMIME_OBJECT(pt));
}

static void interpret_mlt(GSList** stack, gchar** args)
{
    char* subtype = NULL;
    if ( g_strv_length(args) > 0 ) {
        subtype = args[0];
    }
    /* g_debug("st=%s", subtype); */
    stack_multipart(stack, subtype);
}

static void fold_to(GSList** stack, guint level)
{
    guint min_level = level ? level : 1;
    while ( g_slist_length(*stack) > min_level ) {
        /* pop and add to top */
        GSList*      top = *stack;
        GMimeObject* part = GMIME_OBJECT(top->data);
        

        if ( GMIME_IS_MESSAGE(part) ) {
            GMimeMessagePart* mgp = g_mime_message_part_new_with_message(
                "rfc822",
                GMIME_MESSAGE(part));
            part = GMIME_OBJECT(mgp);
        }

        *stack = g_slist_remove_link(*stack, top);

        add_part(stack, part);
        g_slist_free(top);
    }
}

static void interpret_one(gpointer d, gpointer ud)
{
    Atom*    a = (Atom*) d;
    GSList** stack = (GSList**) ud;

    /* g_debug("a->level=%i; e->last_level=%i", a->level, e->last_level); */
    if ( a->level < g_slist_length(*stack) ) {
        fold_to(stack, a->level);
    } else if ( a->level > g_slist_length(*stack) ) {
        /* implicit multipart */
        implicit_level_increase(stack);
    }

    switch (a->type) {
        case at_Header:
            interpret_header(stack, a->args);
            break;
        case at_Text:
            interpret_text(stack, a->args);
            break;
        case at_Message:
            interpret_message(stack, a->args);
            break;
        case at_Att:
            interpret_att(stack, a->args);
            break;

        case at_Multi:
            interpret_mlt(stack, a->args);
            break;

        case at_Empty:
        case at_Invalid:
        default:
            ;
    }
}

static void write_result(GMimeMessage* m, const char* fn)
{
    FILE* of = stdout;

    if ( fn ) {
        of = fopen(fn, "w");
    }

    GMimeStream* fst = g_mime_stream_file_new(of);
    GMimeFilter* flt = g_mime_filter_crlf_new(TRUE, FALSE);
    GMimeStream* st = g_mime_stream_filter_new(fst);
    time_t       dt;
    int          tz_off = 0;
    
    if ( !g_mime_message_get_message_id(m) ) {
        char* mid = g_mime_utils_generate_message_id("MG");
        g_mime_message_set_message_id(m, mid);
        g_free(mid);
    }
    g_mime_message_get_date(m, &dt, &tz_off);
    if ( !dt ) {
        g_mime_message_set_date(m, time(NULL), 0);
    }
    g_mime_stream_filter_add(GMIME_STREAM_FILTER(st), flt);
    g_mime_object_write_to_stream(GMIME_OBJECT(m), st);

    g_object_unref(st);
    g_object_unref(flt);
    g_object_unref(fst);
}

static void free_atom(gpointer d, gpointer ud)
{
    atom_free((Atom*) d);
}

static void interpret(GSList* atoms, const char* fn)
{
    GSList* stack = NULL;
    g_slist_foreach(atoms, interpret_one, &stack);

    fold_to(&stack, 1);
    write_result(GMIME_MESSAGE(stack->data), fn);

    g_slist_foreach(atoms, free_atom, NULL);
    g_object_unref(stack->data);
    g_slist_free(stack);
}

static gchar* generate_tmpfn(void)
{
    gchar tmpl[] = "mg.XXXXXX";
    int fd = g_mkstemp(tmpl);
    
    close(fd);
    return g_strdup(tmpl);
}

static gchar*   src_fn = NULL;
static gchar*   dst_fn = NULL;
static gboolean display = FALSE;
static gboolean tmp = FALSE;
static gboolean verbose = FALSE;

static GOptionEntry opts[] = {
    { "source",    's', 0,                          G_OPTION_ARG_FILENAME, &src_fn,  "Source file name",                      "SFN" },
    { "dest",      'o', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_FILENAME, &dst_fn,  "Destination file name",                 "DFN" },
    { "display",   'd', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_NONE,     &display, "Just display the output",               NULL },
    { "temp",      't', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_NONE,     &tmp,     "Generate a temporaty destination file", NULL },
    { "verbose",   'v', G_OPTION_FLAG_OPTIONAL_ARG, G_OPTION_ARG_NONE,     &verbose, "Verbose output",                        NULL },
    { NULL },
};

static gchar* generate_output_filename(const gchar* dfn, gboolean display, gboolean tmp)
{
    gchar* rv = NULL;
    if ( !display ) {
        if ( tmp ) {
            rv = generate_tmpfn();
        } else {
            rv = g_strdup(dfn);
        }
    }
    return rv;
}

static gboolean run(const gchar* sfn, const gchar* ofn)
{
    GSList*  atoms = parse(sfn);
    gboolean rv = FALSE;

    if ( atoms ) {
        g_mime_init(0);
        interpret(atoms, ofn);
        g_mime_shutdown();
        rv = TRUE;
    }
    return rv;
}

int main(int argc, char* argv[])
{
    int rv = 0;
    GError* err = NULL;
    GOptionContext* octx = g_option_context_new("- MIME message generator");

    g_option_context_add_main_entries(octx, opts, NULL);
    if ( g_option_context_parse(octx, &argc, &argv, &err) ) {
        gboolean go = FALSE;

        if ( !src_fn ) {
            g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "A source filename is required");
        } else if ( !dst_fn && !display && !tmp ) {
            g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "An output method must be selected");
        } else {
            go = TRUE;
        }

        if ( go ) {
            gchar* ofn = generate_output_filename(dst_fn, display, tmp);

            if ( verbose ) {
                g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "sfn=%s", src_fn);
                g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "ofn=%s", ofn ? ofn : "DISPLAY");
            }

            if ( !run(src_fn, ofn) ) {
                g_log(G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "Failed to interpret the message spec");
                rv = 1;
            }

            if ( ofn ) {
                fprintf(stdout, "%s\n", ofn);
                g_free(ofn);
            }
        } else {
            rv = 1;
        }
    }

    return rv;
}
