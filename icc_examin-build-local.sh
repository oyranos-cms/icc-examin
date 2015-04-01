#!/bin/sh

UNAME_=`uname`
if [ $UNAME_ = "Darwin" ]; then
  export CFLAGS="-Wall -gdwarf-2 -I/Developer/SDKs/MacOSX10.6.sdk/usr/include/libxml2"
  export CXXFLAGS="-Wall -gdwarf-2 -I/Developer/SDKs/MacOSX10.6.sdk/usr/include/libxml2 -I/usr/X11R6/include"
  export LDFLAGS="-L/usr/X11R6/lib -L/opt/local/lib"
  export prefix=/opt/local
  export skip=openicc,basiccolor,sane,compicc,kolor-manager,cinepaint
fi

./icc_examin-build.sh

#test
if [ -f "icc_examin/iccexamin" ]; then
  icc-examin/iccexamin icc-profiles-basiccolor-printing2009-1.2.0/default_profiles/printing/ISOcoated_v2_bas.ICC
fi
