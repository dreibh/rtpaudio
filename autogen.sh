#!/bin/sh

./bootstrap && \
./configure --enable-kernel-sctp --enable-static --disable-shared --enable-qt $@ && \
( gmake -j2 || make -j2 )
