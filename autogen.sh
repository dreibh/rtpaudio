#!/bin/sh

./bootstrap && \
./configure --enable-kernel-sctp --enable-static --disable-shared --enable-qt --enable-pulseaudio $@ && \
( gmake -j2 || make -j2 )
