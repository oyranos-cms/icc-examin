#!/bin/bash

exe=iccexamin
path="${0%/*}"
export RESOURCESPATH="${path%/*}/Resources"
binpath="$RESOURCESPATH/bin"
export DYLD_LIBRARY_PATH="$RESOURCESPATH/lib":$DYLD_LIBRARY_PATH
export PATH=$binpath:$PATH:/opt/local/bin
#set >> /tmp/test.txt
"$path/$exe" $@
