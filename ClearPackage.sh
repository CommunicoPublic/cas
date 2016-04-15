#!/bin/sh

if [ -f Makefile ]; then
    make clean
fi

rm -f doc/html/*

rm -fr cas-config CMakeCache.txt CMakeFiles CTestTestfile.cmake DartTestfile.txt Makefile Testing install_manifest.txt cmake_install.cmake progress.make include/ASSysHeaders.h *.so
rm -rf debian/cas3.8 \
       debian/cas-bsonrpc-view3.8 \
       debian/cas-bsonrpc-view3.8-dbg \
       debian/cas-bson-view3.8 \
       debian/cas-bson-view3.8-dbg \
       debian/cas-cli3.8 \
       debian/cas-cli3.8-dbg \
       debian/cas-configs3.8 \
       debian/cas-ctpp-view3.8 \
       debian/cas-ctpp-view3.8-dbg \
       debian/cas-dev \
       debian/cas-example-handler3.8 \
       debian/cas-example-module3.8 \
       debian/cas-examples3.8 \
       debian/cas-example-view3.8 \
       debian/cas-fcgid3.8 \
       debian/cas-fcgid3.8-dbg \
       debian/cas-http-redirect-view3.8 \
       debian/cas-http-redirect-view3.8-dbg \
       debian/cas-jsonrpc-view3.8 \
       debian/cas-jsonrpc-view3.8-dbg \
       debian/cas-json-view3.8 \
       debian/cas-json-view3.8-dbg \
       debian/cas-plain-view3.8 \
       debian/cas-plain-view3.8-dbg \
       debian/cas-tabseparated-view3.8 \
       debian/cas-tabseparated-view3.8-dbg \
       debian/cas-xmlrpc-view3.8 \
       debian/cas-xmlrpc-view3.8-dbg \
       debian/cas-xml-view3.8 \
       debian/cas-xml-view3.8-dbg \
       debian/libapache2-mod-cas2 \
       debian/libapache2-mod-cas2-dbg \
       debian/libcas3.8 \
       debian/libcas3.8-dbg
rm -rf debian/*.log debian/*.substvars debian/files debian/*.debhelper debian/*.symbols debian/*.init
rm -rf debian/tmp
rm -rf build-stamp
rm -rf configure-stamp

