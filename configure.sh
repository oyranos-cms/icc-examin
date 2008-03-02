#/bin/bash


CONF_H=config.h
CONF=config

echo -e "\c" > $CONF_H
echo -e "\c" > $CONF



FLTK_=`fltk-config --cxxflags`
if [ $? == 0 ] && [ -n "$FLTK_" ]; then
  echo "FLTK detected"
  echo "#define HAVE_FLTK" >> $CONF_H
  echo "FLTK = 1" >> $CONF
  echo "FLTK_H = -DHAiVE_FLTK" >> $CONF
else
  echo "FLTK is not detected; you need to install or download from www.fltk.org"
fi


FLU_=`flu-config --cxxflags`
if [ $? == 0 ] && [ -n "$FLU_" ] && [ -n "$FLTK_" ]; then
  echo "FLU detected"
  echo "#define HAVE_FLU" >> $CONF_H
  echo "FLU = 1" >> $CONF
  echo "FLU_H = -DHAVE_FLU" >> $CONF
fi


UNAME_=`uname`
if [ $? == 0 ] && [ $UNAME_ == "Darwin" ]; then
  echo "Darwin PPC detected"
  echo "APPLE = 1" >> $CONF
  echo "OSX_H = -DHAVE_OSX" >> $CONF
else
  if [ $UNAME_ == "Linux" ]; then
    echo "Linux system detected"
  else
    echo "ICC Examin may or may not compile on your $UNAME_ system"
  fi
fi


OY_=`oyranos-config`
if [ $? == 0 ] && [ -n $OY_ ]; then
  echo "Oyranos detected"
  echo "OY = 1" >> $CONF
  echo "OY_H = -DHAVE_OY" >> $CONF
fi


if [ -f /usr/X11R6/include/X11/extensions/xf86vmode.h ]; then
  echo "X VidMode extension found"
  echo "X11 = 1" >> $CONF
  echo "X_H = -DHAVE_X" >> $CONF
else
  if [ $UNAME_ == "Linux" ]; then
    echo "X VidMode extension not found in /usr/X11R6/include/X11/extensions/xf86vmode.h"
  fi
fi



