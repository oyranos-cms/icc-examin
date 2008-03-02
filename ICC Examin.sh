#!/bin/bash

exe=iccexamin
path="${0%/*}"
export RESOURCESPATH="${path%/*}/Resources"
binpath="$RESOURCESPATH/bin"
test -z $DYLD_FALLBACK_LIBRARY_PATH && \
export DYLD_FALLBACK_LIBRARY_PATH="$RESOURCESPATH/lib" ||
export DYLD_FALLBACK_LIBRARY_PATH="$RESOURCESPATH/lib":$DYLD_FALLBACK_LIBRARY_PATH
export PATH=$binpath:$PATH:/opt/local/bin
#set >> /tmp/test.txt
"$path/$exe" $@
