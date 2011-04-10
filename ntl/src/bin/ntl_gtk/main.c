/*
 *  Part of "NTL" - a simple network logging system
 *
 *  Copyright 2011 Don Kelly <karfai@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "ntll.h"
#include <gtk/gtk.h>

/* a prettier, Gtk+ 2.0 listener client */

static gchar default_host[] = "localhost";
static gchar host_label_text[] = "Host:";

static void write_log(const ntl_Packet* pkt, gpointer d);

typedef struct {
    GtkWidget*     entry;
    GtkWidget*     label;
    GtkWidget*     connect;
    GtkWidget*     disconnect;
    GtkTextBuffer* text_buffer;
    ntl_Listener*  listener;
} ntl_Mediator;

static ntl_Mediator* ntl_mediator_new(void)
{
    ntl_Mediator* rv = g_new(ntl_Mediator, 1);
    rv->entry = NULL;
    rv->label = NULL;
    rv->connect = NULL;
    rv->disconnect = NULL;
    rv->text_buffer = NULL;
    rv->listener = NULL;
    return rv;
}

static void ntl_mediator_free(ntl_Mediator* m)
{
    ntl_listener_free(m->listener);
    g_free(m);
}

static void ntl_mediator_connect(ntl_Mediator* m)
{
    const gchar* host = gtk_entry_get_text(GTK_ENTRY(m->entry));
    gchar* msg = g_strdup_printf("Connected to %s", host);

    m->listener = ntl_listener_new(host, write_log, m);

    gtk_label_set_text(GTK_LABEL(m->label), msg);
    gtk_widget_hide(m->entry);
    gtk_widget_hide(m->connect);
    gtk_widget_show(m->disconnect);
    
    g_free(msg);
}

static void ntl_mediator_disconnect(ntl_Mediator* m)
{
    ntl_listener_free(m->listener);
    m->listener = NULL;

    gtk_widget_hide(m->disconnect);
    gtk_widget_show(m->connect);
    gtk_widget_show(m->entry);
    gtk_label_set_text(GTK_LABEL(m->label), host_label_text);
}

static gchar trace_tag[] = "trace";
static gchar debug_tag[] = "debug";
static gchar warning_tag[] = "warning";
static gchar error_tag[] = "error";
static gchar unknown_tag[] = "unknown";

static gchar* get_tag_for_level(ntl_TraceLevelT lvl)
{
    switch (lvl) {
        case ntl_tl_Trace:
            return trace_tag;

        case ntl_tl_Debug:
            return debug_tag;

        case ntl_tl_Warn:
            return warning_tag;

        case ntl_tl_Error:
            return error_tag;
            
        default:
            ;
    }
    return unknown_tag;
}

static void write_log(const ntl_Packet* pkt, gpointer d)
{
    ntl_Mediator* m = (ntl_Mediator*) d;
    gsize         wrote = 0;
    gchar*        tm = ntl_listener_default_time_format(pkt);
    gchar*        tag = g_strdup_printf("[%s]", pkt->tag);
    gchar*        proc = g_strdup_printf("[%s/%u/%u]", pkt->prog, pkt->pid, pkt->tid);
    gchar*        loc = g_strdup_printf("[%s/%s]", pkt->mod, pkt->fn);

    gchar* ln = g_strdup_printf("%24s %12s %20s %20s: %s\n",
        tm, tag, proc, loc, pkt->msg);
    {
        GtkTextIter it;
        GtkTextIter st;
        GtkTextMark* mrk = NULL;

        gtk_text_buffer_get_end_iter(m->text_buffer, &it);
        mrk = gtk_text_buffer_create_mark(m->text_buffer, NULL, &it, TRUE);
        gtk_text_buffer_insert_with_tags_by_name(m->text_buffer, &it, ln, -1, get_tag_for_level(pkt->lvl), NULL);
    }
    g_free(tm);
    g_free(tag);
    g_free(proc);
    g_free(loc);
    g_free(ln);
}

static void window_destroy(GtkWidget* w, gpointer d)
{
    gtk_main_quit();
}

static void connect_listener(GtkWidget* w, gpointer d)
{
    ntl_mediator_connect((ntl_Mediator*) d);
}

static void disconnect_listener(GtkWidget* w, gpointer d)
{
    ntl_mediator_disconnect((ntl_Mediator*) d);
}

static GtkWidget* build_textview(ntl_Mediator* m)
{
    PangoFontDescription* f = pango_font_description_from_string("Courier 10");
    GtkWidget* w = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(w), FALSE);
    gtk_widget_modify_font(GTK_WIDGET(w), f);
    pango_font_description_free(f);
    gtk_widget_show(w);

    m->text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));
    gtk_text_buffer_create_tag(m->text_buffer, error_tag, "foreground", "red", NULL);
    gtk_text_buffer_create_tag(m->text_buffer, warning_tag, "foreground", "orange", NULL);
    gtk_text_buffer_create_tag(m->text_buffer, debug_tag, "foreground", "blue", NULL);
    gtk_text_buffer_create_tag(m->text_buffer, trace_tag, "foreground", "black", NULL);
    gtk_text_buffer_create_tag(m->text_buffer, unknown_tag, "foreground", "gray", NULL);

    return w;
}

static GtkWidget* build_output(ntl_Mediator* m)
{
    GtkWidget* w = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(w),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(
        GTK_SCROLLED_WINDOW(w), GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(w), build_textview(m));
    gtk_widget_show(w);
    
    return w;
}

static GtkWidget* build_hostname_label(ntl_Mediator* m)
{
    GtkWidget* w = gtk_label_new(host_label_text);
    gtk_widget_show(w);

    m->label = w;

    return w;
}

static GtkWidget* build_hostname_entry(ntl_Mediator* m)
{
    GtkWidget* w = gtk_entry_new();

    gtk_entry_set_text(GTK_ENTRY(w), default_host);
    gtk_widget_show(w);

    m->entry = w;

    return w;
}

static GtkWidget* build_connect_button(ntl_Mediator* m)
{
    GtkWidget* w = gtk_button_new_from_stock(GTK_STOCK_CONNECT);
    g_signal_connect(
        G_OBJECT(w), "clicked",
        G_CALLBACK(connect_listener), m);
    gtk_widget_show(w);

    m->connect = w;

    return w;
}

static GtkWidget* build_disconnect_button(ntl_Mediator* m)
{
    GtkWidget* w = gtk_button_new_from_stock(GTK_STOCK_DISCONNECT);
    g_signal_connect(
        G_OBJECT(w), "clicked",
        G_CALLBACK(disconnect_listener), m);

    m->disconnect = w;

    return w;
}

static GtkWidget* build_connection(ntl_Mediator* m)
{
    GtkWidget* w = gtk_hbox_new(FALSE, 4);

    gtk_box_pack_start(GTK_BOX(w), build_hostname_label(m), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(w), build_hostname_entry(m), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(w), build_connect_button(m), FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(w), build_disconnect_button(m), FALSE, TRUE, 0);
    gtk_widget_show(w);

    return w;
}

static GtkWidget* build_parts(ntl_Mediator* m)
{
    GtkWidget* w = gtk_vbox_new(FALSE, 6);

    gtk_box_set_spacing(GTK_BOX(w), 6);
    gtk_box_pack_start(GTK_BOX(w), build_connection(m), FALSE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(w), build_output(m), TRUE, TRUE, 0);
    gtk_widget_show(w);

    return w;
}

static GtkWidget* build_main_window(ntl_Mediator* m)
{
    GtkWidget* w = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(w), "NTL");
    gtk_container_set_border_width(GTK_CONTAINER(w), 8);

    g_signal_connect(G_OBJECT(w), "destroy", G_CALLBACK(window_destroy), NULL);

    gtk_container_add(GTK_CONTAINER(w), build_parts(m));

    gtk_widget_show(w);

    return w;
}

int main(int argc, char* argv[])
{
    ntl_Mediator* med = ntl_mediator_new();

    gtk_init(&argc, &argv);
    gnet_init();
    build_main_window(med);

    gtk_main();

    ntl_mediator_free(med);

    return 0;
}


