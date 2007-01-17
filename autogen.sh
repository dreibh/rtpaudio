#!/bin/sh
aclocal && autoconf && autoheader && automake --add-missing && ./configure --enable-kernel-sctp --enable-qt --enable-maintainer-mode $@ && make
