#!/bin/bash

exe=iccexamin
path="${0%/*}"
export RESOURCESPATH="${path%/*}/Resources"
binpath="$RESOURCESPATH/bin"
test -z $DYLD_FALLBACK_LIBRARY_PATH && \
export DYLD_FALLBACK_LIBRARY_PATH="$RESOURCESPATH/lib" ||
export DYLD_FALLBACK_LIBRARY_PATH="$RESOURCESPATH/lib":$DYLD_FALLBACK_LIBRARY_PATH
export PATH=$binpath:$PATH:/opt/local/bin

test -z $XDG_DATA_DIRS && \
XDG_DATA_DIRS="$RESOURCESPATH/share" ||
XDG_DATA_DIRS="$RESOURCESPATH/share:$XDG_DATA_DIRS"

test -z $OY_MODULE_PATHS && \
OY_MODULE_PATHS="$RESOURCESPATH/lib/color/cmms" ||
OY_MODULE_PATHS="$RESOURCESPATH/lib/color/cmms:$OY_MODULE_PATHS"

export XDG_DATA_DIRS OY_MODULE_PATHS



#set >> /tmp/test.txt
"$path/$exe" $@
