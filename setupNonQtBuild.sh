#!/bin/sh 
cp -r NonQtBuild/. .
echo now run aclocal
echo followed by autoheader
echo followed by automake
echo followed by autoconf
echo followed by ./configure
echo followed by make