#!/bin/sh

# no makefile ... terrible of me, I know
. ./environs
PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$DEV_LIBS/lib/pkgconfig/
export PKG_CONFIG_PATH
gcc `pkg-config --cflags --libs gmime-2.6` ./message-gen.c -o mg