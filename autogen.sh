#!/bin/sh -e

rm -f CMakeCache.txt
cmake -DCMAKE_INSTALL_PREFIX=/usr -DWITH_QT=1 -DWITH_NEAT=0 .

cores=`getconf _NPROCESSORS_ONLN`
make -j${cores}
