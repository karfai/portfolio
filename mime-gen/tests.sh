#!/bin/sh

. ./environs
./run.sh -s message.mg -o result.out
diff -uN message.cmp result.out
./run.sh -s message.multipart.mg -o result.out
diff -uN message.multipart.cmp result.out
./run.sh -s message.nested.headers.mg mg -o result.out
diff -uN message.nested.headers.cmp result.out
