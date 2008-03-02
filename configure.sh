#/bin/bash


CONF_H=config.h
CONF=config
VERSION="0.24"
ZEIT="Mai 2004 - Maerz 2005"

test -f error.txt && rm -v error.txt
test -f config && make clean


echo -e "\c" > $CONF_H
echo -e "\c" > $CONF

echo ""

FLTK_=`fltk-config --cxxflags 2>>error.txt`
if [ $? == 0 ] && [ -n "$FLTK_" ]; then
  echo "FLTK                    detected"
  echo "#define HAVE_FLTK" >> $CONF_H
  echo "FLTK = 1" >> $CONF
  echo "FLTK_H = -DHAVE_FLTK" >> $CONF
else
  echo "FLTK is not detected; you need to install or download from www.fltk.org"
fi


FLU_=`flu-config --cxxflags 2>>error.txt`
if [ $? == 0 ] && [ -n "$FLU_" ] && [ -n "$FLTK_" ]; then
  echo "FLU                     detected"
  echo "#define HAVE_FLU" >> $CONF_H
  echo "FLU = 1" >> $CONF
  echo "FLU_H = -DHAVE_FLU" >> $CONF
else
  echo "no FLU detected, will not use it"
fi


UNAME_=`uname`
if [ $? == 0 ] && [ $UNAME_ == "Darwin" ]; then
  echo "Darwin PPC              detected"
  echo "APPLE = 1" >> $CONF
  echo "OSX_H = -DHAVE_OSX" >> $CONF
else
  if [ $UNAME_ == "Linux" ]; then
    echo "Linux system            detected"
  else
    echo "ICC Examin may or may not compile on your $UNAME_ system"
  fi
fi


OY_=`oyranos-config 2>>error.txt`
if [ $? == 0 ] && [ -n $OY_ ]; then
  echo "Oyranos                 detected"
  echo "#define HAVE_OY" >> $CONF_H
  echo "OY = 1" >> $CONF
  echo "OY_H = -DHAVE_OY" >> $CONF
else
  echo "no Oyranos detected"
fi


if [ -f /usr/X11R6/include/X11/extensions/xf86vmode.h ]; then
  echo "X VidMode extension     detected"
  echo "#define HAVE_X" >> $CONF_H
  echo "X11 = 1" >> $CONF
  echo "X_H = -DHAVE_X" >> $CONF
else
  if [ $UNAME_ == "Linux" ]; then
    echo "X VidMode extension not found in /usr/X11R6/include/X11/extensions/xf86vmode.h"
  fi
fi

echo "" >> $CONF
echo "VERSION = $VERSION" >> $CONF
echo "src_dir = `pwd`/icc_examin_$VERSION" >> $CONF

echo "#ifndef ICC_VERSION_H" > icc_version.h
echo "#define ICC_VERSION_H" >> icc_version.h
echo "" >> icc_version.h
echo -e "#define ICC_EXAMIN_V \c" >> icc_version.h
echo $VERSION >> icc_version.h
echo -e "#define ICC_EXAMIN_D _(\"\c" >> icc_version.h
echo -e "$ZEIT\")" >> icc_version.h
echo "" >> icc_version.h
echo "#endif //ICC_VERSION_H" >> icc_version.h

echo ""
echo ""


if [ `cat error.txt | wc -l` -lt 1 ]; then
  rm error.txt
fi
