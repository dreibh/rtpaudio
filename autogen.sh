#!/bin/sh

./bootstrap && \
./configure --enable-kernel-sctp --enable-static --disable-shared --enable-qt $@ && \
( gmake || make )
