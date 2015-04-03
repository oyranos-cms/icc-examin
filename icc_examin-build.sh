#!/bin/sh
# -xv

if [ $# -gt 1 ] ; then
  if [ $1 = "-h" ] || [ $1 = "--help" ] || [ $1 = "-?" ]; then
    echo ""
    echo "$0 help:"
    echo "any argument is passed to the configure scripts"
    echo "e.g. --prefix=/usr => ./configure --prefix=/usr"
    echo ""
    exit 0
  fi
fi


# options

# uncomment below the packages you are not interessted in
# elektra,openicc,basiccolor,sane,LibRaw,compicc,synnefo,kolor-manager,cinepaint
# export skip=elektra,openicc,basiccolor,sane,LibRaw,compicc,synnefo,kolor-manager,cinepaint

# uncomment the below line to see less messages
# export verbose=0



top="`pwd`"
stop_build=0
target=iccexamin


### Testing the system ###

UNAME_=`uname`
if [ $UNAME_ = "Darwin" ]; then
  MD5SUM=md5
  SHA1SUM=shasum

  OSX_ARCH="-arch `uname -p`"

  # allow for fat binaries
  export configure_opts_extra=--disable-dependency-tracking
  export SKIPBARCH=1

  if [ "$OSX_ARCH" = "ppc" ]; then
    OSX_ARCH_CARBON="-arch ppc -arch i386"
    OSX_ARCH_LIBRARY="-arch i386 -arch ppc"
    #OSX_ARCH_LIBRARY=-DCMAKE_OSX_ARCHITECTURES="i386 ppc"
  else
    OSX_ARCH_CARBON="-arch i386"
    OSX_ARCH_COCOA="-arch x86_64"
    OSX_ARCH_LIBRARY="-arch i386 -arch x86_64"
    #OSX_ARCH_LIBRARY=-DCMAKE_OSX_ARCHITECTURES="i386 x86_64"
  fi

else
  MD5SUM=md5sum
  if [ "`which sha1`" != "" ]; then
    SHA1SUM=sha1
  else
    SHA1SUM=sha1sum
  fi
  if [ $UNAME_ = "MINGW32_NT-6.1" ]; then
    url=ftp://ftp.gnupg.org/gcrypt/binary
    packet_file=sha1sum.exe
    checksum=10cf8f3c437f979309c42941f21f4023
    if [ -f $packet_file ]; then
      echo $packet_file already here
    else
      echo downloading $url/$packet_file
      which curl && curl -L $url/$packet_file -o $packet_file || wget $url/$packet_file
    fi
    if [ `$MD5SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
      echo downloaded $packet_file
    else
      rm $packet_file
      echo download of $packet_file incomplete
    fi
    cmake_target="-GMSYS Makefiles"
  fi
fi

# get host 64-bit capabilities
echo \
"#include <stdio.h>

int main(int argc, char**argv)
{
  fprintf(stdout, \"%d\", (int)sizeof(int*));
  return 0;
}" > ptr_size.c
gcc -Wall -g ptr_size.c -o ptr-size
BARCH=""
INTPTR_SIZE=`./ptr-size`
  if [ $INTPTR_SIZE -eq 4 ]; then
    echo_="32-bit build            detected"
  elif [ $INTPTR_SIZE -gt 4 ]; then
    echo_="64-bit build            detected"
    if [ -z "$SKIPBARCH" ]; then
      BARCH=64
    else
      echo_="ignore 64-bit build"
    fi
    FPIC=-fPIC
    test -n "$ECHO" && $ECHO "BUILD_64 = 1" >> $CONF
  elif [ $INTPTR_SIZE -ne 0 ]; then
    echo_="$INTPTR_SIZE-byte intptr_t          detected"
  else
    echo_="CPU bus width not         detected"
  fi
echo "$echo_"
LIB=lib$BARCH


UNAME_=`uname`
if [ $UNAME_ = "MINGW32_NT-6.1" ]; then
  PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig
fi


# get processor count
echo \
"#include <stdio.h>
#include <omp.h>

int main(int argc, char**argv)
{
  int i;
  #pragma omp parallel
  for(i = 0; i < 2; ++i) ;
  fprintf(stdout, \"%d\", omp_get_num_procs());
  return 0;
}" > omp.c
gcc -fopenmp -Wall -g omp.c -o processors
cpus=`./processors`
if [ -n "$cpus" ]; then
  MAKE_CPUS="-j$cpus"
  echo will add MAKE_CPUS=$MAKE_CPUS
fi


# environment

export kde_prefix=$HOME/.kde4/
if [ -z $prefix ]; then
  export prefix=$HOME/.local
fi
switch=prefix
if [ "`echo \"$1\" | sed s/\"--$switch=\"//`" != "$1" ]; then
  prefix="`echo \"$1\" | sed s/\"--$switch=\"//`"
fi
if [ -z $libdir ]; then
  libdir=$prefix/$LIB
fi
switch=libdir
if [ "`echo \"$1\" | sed s/\"--$switch=\"//`" != "$1" ]; then
  libdir="`echo \"$1\" | sed s/\"--$switch=\"//`"
fi
conf_opts="--prefix=$prefix --libdir=$libdir $configure_opts_extra"

if [ -z "$LDFLAGS" ]; then
  export LDFLAGS=-L$libdir
fi
if [ -z "$LD_LIBRARY_PATH" ]; then
  LD_LIBRARY_PATH=$libdir
else
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$libdir
fi
if [ -z "$PKG_CONFIG_PATH" ]; then
  PKG_CONFIG_PATH=$libdir/pkgconfig
else
  PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$libdir/pkgconfig
fi
if [ -z "$PATH" ]; then
  PATH=$prefix/bin
else
  PATH=$PATH:$prefix/bin
fi
if [ -z "$kde_prefix" ]; then
  kde_prefix=$HOME/.kde4/
fi
if [ -z "$verbose" ]; then
  verbose=1
fi
if [ $verbose -eq 0 ]; then
  v=--disable-verbose
else
  v=--enable-silent-rules
fi
export PATH PKG_CONFIG_PATH LD_LIBRARY_PATH kde_prefix


### dependency testing ###

# pkg-config
packet=pkg-config
packet_dir=$packet-0.23
packet_file=$packet_dir.tar.gz
url=http://pkgconfig.freedesktop.org/releases/
packet_ready=0
checksum=b59dddd6b5320bd74c0f74b3339618a327096b2a
pkg-config --version
if [ $? -eq 0 ]; then
  echo "$packet found, skipping $packet build and installation"
  packet_ready=1
else
  if [ -d $packet_dir ]; then
    echo "$packet + $packet_dir found, skipping $packet download"
    packet_ready=1
  fi
  if [ -f $packet_file ]; then
    echo $packet_file already here
  else
    echo downloading $url/$packet_file
    which curl && curl -L $url/$packet_file -o $packet_file || wget $url/$packet_file
    if [ $verbose -gt 0 ]; then sleep 1; fi
  fi
  if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
    echo sha1sum for $packet_file passed
  else
    echo sha1sum for $packet_file failed
    exit 1
  fi
  if [ -d $packet_dir ]; then
    echo remove $packet_dir
    if [ $verbose -gt 0 ]; then sleep 1; fi
    rm -r $packet_dir
  fi
  tar xzf $packet_file
  cd $packet_dir
   make clean
  CFLAGS=-I./glib-1.2.10 ./configure $conf_opts $@
  make $MAKE_CPUS
  make install
fi
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"

  fltk="FLTK:           `fltk-config --version`"
if [ $? -gt 0 ]; then 
  fltk="!!! FLTK is missed"
  #stop_build=1

  # FLTK
  echo building FLTK ...
  packet=fltk
  url=http://www.fltk.org/pub/fltk/1.3.3/
  packet_dir=$packet-1.3.3
  packet_file="$packet_dir"-source.tar.gz
  checksum=9ccdb0d19dc104b87179bd9fd10822e3
  if [ -f $packet_file ]; then
    echo $packet_file already here
  else
    echo "downloading $url$packet_file"
    which curl && curl -L "$url$packet_file" -o $packet_file || wget "$url$packet_file"
    if [ $verbose -gt 0 ]; then sleep 1; fi
  fi
  if [ `$MD5SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
    echo md5sum for $packet_file passed
  else
    echo md5sum for $packet_file failed
    exit 1
  fi
  packet_ready=0
  pkg-config --atleast-version=1.1.7 $packet
  if [ $? -eq 0 ]; then
    if [ -d $packet_dir ]; then
      echo "$packet + $packet_dir found, skipping $packet build and installation"
      packet_ready=1
    fi
  else
    echo PKG_CONFIG_PATH=$PKG_CONFIG_PATH
    pkg-config --modversion $packet
  fi
  if [ $packet_ready -lt 1 ]; then
    if [ -d $packet_dir ]; then
      echo remove $packet_dir
      if [ $verbose -gt 0 ]; then sleep 1; fi
      rm -r $packet_dir
    fi
    echo unpacking $packet_file ...
    tar xzf $packet_file
    cd $packet_dir
    make clean
    fltk_conf_opts="`echo $conf_opts | sed s/--disable-dependency-tracking//g`"
    ./configure $fltk_conf_opts --enable-gl --enable-shared --enable-threads --enable-xinerama --enable-xft $@
    make $MAKE_CPUS
    make install
  fi
  if [ $verbose -gt 0 ]; then sleep 1; fi

  cd "$top"
fi
  ftgl="FTGL:           `pkg-config --modversion ftgl`"
if [ $? -gt 0 ]; then 
  ftgl="FTGL is missed or not ready"
#  stop_build=1
fi
pkg-config  --atleast-version=2.2 lcms2
if [ $? -eq 0 ]; then 
  lcms="littleCMS:      `pkg-config --modversion lcms2`"
else
  lcms="littleCMS version is too old; need at least lcms-1.14"
  #stop_build=1

  # lcms
  echo building lcms ...
  packet=lcms2
  packet_dir=$packet-2.2
  packet_file="$packet_dir".tar.gz
  url="http://sourceforge.net/projects/lcms/files/lcms/2.2/"
  checksum=55ae4884a92c7fbd491c118aa3b356814b1014df
  if [ -f $packet_file ]; then
    echo $packet_file already here
  else
    echo "downloading $url$packet_file"
    which curl && curl -L "$url$packet_file" -o $packet_file || wget "$url$packet_file"
    if [ $verbose -gt 0 ]; then sleep 1; fi
  fi
  if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
    echo sha1sum for $packet_file passed
  else
    echo sha1sum for $packet_file failed
    exit 1
  fi
  packet_ready=0
  pkg-config --atleast-version=1.17 $packet
  if [ $? -eq 0 ]; then
    if [ -d $packet_dir ]; then
      echo "$packet + $packet_dir found, skipping $packet build and installation"
      packet_ready=1
    fi
  else
    echo PKG_CONFIG_PATH=$PKG_CONFIG_PATH
    pkg-config --modversion $packet
  fi
  if [ $packet_ready -lt 1 ]; then
    if [ -d $packet_dir ]; then
      echo remove $packet_dir
      if [ $verbose -gt 0 ]; then sleep 1; fi
      rm -r $packet_dir
    fi
    echo unpacking $packet_file ...
    tar xzf $packet_file
    cd $packet_dir
    make clean
    CFLAGS="$CFLAGS $OSX_ARCH_LIBRARY" CXXFLAGS="$CXXFLAGS $OSX_ARCH_LIBRARY" LDFLAGS="$LDFLAGS $OSX_ARCH_LIBRARY" ./configure $conf_opts $@
    make $MAKE_CPUS
    make install
  fi
  if [ $verbose -gt 0 ]; then sleep 1; fi

  cd "$top"
fi

packet=libxml-2.0
pkg-config  --atleast-version=2.0 $packet
if [ $? -eq 0 ]; then 
  libxml2="$packet:     `pkg-config --modversion $packet`"
else
  libxml2="$packet version is too old; need at least $packet"

  echo building $packet ...
  packet_dir=libxml2-2.7.8.win32
  packet_file="$packet_dir".zip
  url="ftp://ftp.zlatkovic.com/libxml/"
  checksum=cc021bcb0b84a5e34c5aeed6df43c10ed4c15d35
  if [ -f $packet_file ]; then
    echo $packet_file already here
  else
    echo "downloading $url$packet_file"
    which curl && curl -L "$url$packet_file" -o $packet_file || wget "$url$packet_file"
    if [ $verbose -gt 0 ]; then sleep 1; fi
  fi
  if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
    echo sha1sum for $packet_file passed
  else
    echo sha1sum for $packet_file failed
    exit 1
  fi
  packet_ready=0
  pkg-config --atleast-version=1.0 $packet
  if [ $? -eq 0 ]; then
    if [ -d $packet_dir ]; then
      echo "$packet + $packet_dir found, skipping $packet build and installation"
      packet_ready=1
    fi
  else
    echo PKG_CONFIG_PATH=$PKG_CONFIG_PATH
    pkg-config --modversion $packet
  fi
  if [ $packet_ready -lt 1 ]; then
    if [ -d $packet_dir ]; then
      echo remove $packet_dir
      if [ $verbose -gt 0 ]; then sleep 1; fi
      rm -r $packet_dir
    fi
    echo unpacking $packet_file ...
    unzip -q $packet_file
    cd $packet_dir
    cp -a bin/* $prefix/bin/
    cp -a include/* $prefix/include/
    cp -a lib/* $prefix/lib/
  fi
  libpng="$packet:      `pkg-config --modversion $packet`"
  if [ $verbose -gt 0 ]; then sleep 1; fi

  cd "$top"
fi

packet=libpng
pkg-config  --atleast-version=1.0 $packet
if [ $? -eq 0 ]; then 
  libpng="$packet:         `pkg-config --modversion $packet`"
else
  libpng="$packet version is too old; need at least $packet 1.0"
  #stop_build=1

  echo building $packet ...
  packet_dir=$packet-1.5.2
  packet_file="$packet_dir".tar.gz
  url="http://sourceforge.net/projects/libpng/files/libpng15/older-releases/1.5.2/"
  checksum=71c30b9b23169a2dac5b0a77954d9d91f8d944fe
  if [ -f $packet_file ]; then
    echo $packet_file already here
  else
    echo "downloading $url$packet_file"
    which curl && curl -L "$url$packet_file" -o $packet_file || wget "$url$packet_file"
    if [ $verbose -gt 0 ]; then sleep 1; fi
  fi
  if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
    echo sha1sum for $packet_file passed
  else
    echo sha1sum for $packet_file failed
    exit 1
  fi
  packet_ready=0
  pkg-config --atleast-version=1.0 $packet
  if [ $? -eq 0 ]; then
    if [ -d $packet_dir ]; then
      echo "$packet + $packet_dir found, skipping $packet build and installation"
      packet_ready=1
    fi
  else
    echo PKG_CONFIG_PATH=$PKG_CONFIG_PATH
    pkg-config --modversion $packet
  fi
  if [ $packet_ready -lt 1 ]; then
    if [ -d $packet_dir ]; then
      echo remove $packet_dir
      if [ $verbose -gt 0 ]; then sleep 1; fi
      rm -r $packet_dir
    fi
    echo unpacking $packet_file ...
    tar xzf $packet_file
    cd $packet_dir
    make clean
    CFLAGS="$CFLAGS $OSX_ARCH_LIBRARY" CXXFLAGS="$CXXFLAGS $OSX_ARCH_LIBRARY" LDFLAGS="$LDFLAGS $OSX_ARCH_LIBRARY" ./configure $conf_opts $@
    make $MAKE_CPUS
    make install
  fi
  libpng="$packet:      `pkg-config --modversion $packet`"
  if [ $verbose -gt 0 ]; then sleep 1; fi

  cd "$top"
fi

packet=cmake
$packet --version
if [ $? -eq 0 ]; then 
  cmake="$packet:         `cmake --version`"
else
  cmake="$packet version is too old; need at least $packet 2.8"
  #stop_build=1

  echo building $packet ...
  packet_dir=$packet-2.8.10.2
  packet_file="$packet_dir".tar.gz
  url="http://www.cmake.org/files/v2.8/"
  checksum=2d868ccc3f9f2aa7c2844bd0a4609d5313edaaec
  if [ -f $packet_file ]; then
    echo $packet_file already here
  else
    echo "downloading $url$packet_file"
    which curl && curl -L "$url$packet_file" -o $packet_file || wget "$url$packet_file"
    if [ $verbose -gt 0 ]; then sleep 1; fi
  fi
  if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
    echo sha1sum for $packet_file passed
  else
    echo sha1sum for $packet_file failed
    exit 1
  fi
  packet_ready=0
  pkg-config --atleast-version=1.0 $packet
  if [ $? -eq 0 ]; then
    if [ -d $packet_dir ]; then
      echo "$packet + $packet_dir found, skipping $packet build and installation"
      packet_ready=1
    fi
  else
    $packet --version
  fi
  if [ $packet_ready -lt 1 ]; then
    if [ -d $packet_dir ]; then
      echo remove $packet_dir
      if [ $verbose -gt 0 ]; then sleep 1; fi
      rm -r $packet_dir
    fi
    echo unpacking $packet_file ...
    tar xzf $packet_file
    cd $packet_dir
    which c++
    ./configure --prefix=$prefix
    make
    make install
  fi
  cmake="$packet:      `cmake --version`"
  if [ $verbose -gt 0 ]; then sleep 1; fi

  cd "$top"
fi

echo "$fltk"
echo "$ftgl"
echo "$lcms"
echo "$libxml2"
echo "$libpng"
echo "$cmake"
echo ""
echo "$0 $@"
echo "PATH            = $PATH"
echo "LD_LIBRARY_PATH = $LD_LIBRARY_PATH"
echo "PKG_CONFIG_PATH = $PKG_CONFIG_PATH"
echo "prefix          = $prefix"
echo "kde_prefix      = $kde_prefix"
echo "skip            = $skip"

if [ $stop_build -gt 0 ]; then
  echo ""
  echo "some dependencies are missed; see above"
  echo ""
  exit 1
fi
if [ $verbose -gt 0 ]; then sleep 1; fi

# Git
packet=git
packet_dir=$packet-1.9.4
packet_file=$packet_dir.tar.gz
url=https://kernel.org/pub/software/scm/git/
packet_ready=0
git --version
if [ $? -eq 0 ]; then
  echo "$packet found, skipping $packet build and installation"
  packet_ready=1
else
  if [ -d $packet_dir ]; then
    echo "$packet + $packet_dir found, skipping $packet download"
  fi
  echo PKG_CONFIG_PATH=$PKG_CONFIG_PATH
  pkg-config --modversion $packet
  if [ -f $packet_file ]; then
    echo $packet_file already here
  else
    echo downloading $url/$packet_file
    which curl && curl -L $url/$packet_file -o $packet_file || wget $url/$packet_file
    if [ $verbose -gt 0 ]; then sleep 1; fi
  fi
  if [ -d $packet_dir ]; then
    echo remove $packet_dir
    if [ $verbose -gt 0 ]; then sleep 1; fi
    rm -r $packet_dir
  fi
  tar xzf $packet_file
  cd $packet_dir
  make clean
  ./configure $conf_opts $@
  make $MAKE_CPUS
  make install
fi
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"

# yajl
git_repo=yajl
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone git://github.com/lloyd/$git_repo $git_repo
    cd $git_repo
    git checkout master
    url=https://build.opensuse.org/source/home:bekun:devel/libyajl
    packet_file=lib_suffix.patch
    checksum=6a884be9da38be55afeab2c83759867bc2a73bff
    echo download and apply $packet_file
    if [ -f $packet_file ]; then
      a=1
    else
      which curl && curl -L https://build.opensuse.org/source/home:bekun:devel/libyajl/libyajl-lib_suffix.patch?rev=074e0464184ff3fb87ceafb5149c600f -o $packet_file || wget $url/$packet_file
    fi
    if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
      echo sha1sum for $packet_file passed
    else
      echo sha1sum for $packet_file failed
      exit 1
    fi
    patch -p1 < $packet_file

    mkdir build
  fi
  update_yajl=0
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  if [[ ! -d build ]]; then
    mkdir build
    update_yajl=1
  fi
  cd build
  old_git_version="`cat old_gitrev.txt`"
  update_oyranos=0
  pkg-config --atleast-version=0.8 $git_repo
  if [ $? -ne 0 ] || [ $update_yajl = 1 ] ||
     [ "$git_version" != "$old_git_version" ]; then
    echo "$git_repo `pkg-config --modversion $git_repo`"
    update_oyranos=1
    cmake "$cmake_target" -DCMAKE_C_FLAGS="$CFLAGS $OSX_ARCH_LIBRARY" -DCMAKE_CXX_FLAGS="$CXXFLAGS $OSX_ARCH_LIBRARY" -DCMAKE_LD_FLAGS="$LDFLAGS $OSX_ARCH_LIBRARY" -DCMAKE_INSTALL_PREFIX="$prefix" -DLIB_SUFFIX=$BARCH -DCMAKE_BUILD_TYPE=debugfull ..
    if [ $UNAME_ = "MINGW32_NT-6.1" ]; then
      make
    else
      make $MAKE_CPUS
    fi
    make install
    make check
  else
    echo no changes in git $git_version
  fi
  echo "$git_version" > old_gitrev.txt
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"

# Elektra
git_repo=libelektra
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone git://github.com/ElektraInitiative/$git_repo $git_repo
    cd $git_repo
    git checkout master
    mkdir build
  fi
  update_xcm=0
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  if [[ ! -d build ]]; then
    mkdir build
    update_xcm=1
  fi
  cd build
  old_git_version="`cat old_gitrev.txt`"
  update_oyranos=0
  pkg-config --atleast-version=0.8 elektra
  if [ $? -ne 0 ] || [ $update_xcm = 1 ] ||
     [ "$git_version" != "$old_git_version" ]; then
    echo "elektra `pkg-config --modversion elektra`"
    update_oyranos=1
    if [ $UNAME_ = "MINGW32_NT-6.1" ]; then
      cmake "$cmake_target" -DCMAKE_C_FLAGS="$CFLAGS" -DCMAKE_CXX_FLAGS="$CXXFLAGS" -DCMAKE_LD_FLAGS="$LDFLAGS" -DCMAKE_INSTALL_PREFIX="$prefix" -DXDG_CONFIG_DIR="$HOME/.local/xdg" -DLIB_SUFFIX=$BARCH -DCMAKE_BUILD_TYPE=debugfull ..
      make
    else
      cmake "$cmake_target" -DCMAKE_C_FLAGS="$CFLAGS $OSX_ARCH_LIBRARY" -DCMAKE_CXX_FLAGS="$CXXFLAGS $OSX_ARCH_LIBRARY" -DCMAKE_LD_FLAGS="$LDFLAGS $OSX_ARCH_LIBRARY" -DCMAKE_INSTALL_PREFIX="$prefix" -DLIB_SUFFIX=$BARCH -DCMAKE_BUILD_TYPE=debugfull -DINSTALL_SYSTEM_FILES=false -DBUILD_TESTING=false -DENABLE_TESTING=OFF -DTOOLS=DEFAULT -DPLUGINS="dump;resolver;yajl;rename;struct" ..
      make $MAKE_CPUS
    fi
    make install
    make check
  else
    echo no changes in git $git_version
  fi
  echo "$git_version" > old_gitrev.txt
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"

# Xcolor
git_repo=libxcm
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone git://github.com/oyranos-cms/$git_repo $git_repo
    cd $git_repo
    git checkout master
  fi
  if [ $verbose -gt 0 ]; then sleep 2; fi
  update_xcm=0
  git_version="`cat .git/refs/heads/master`"
  old_git_version="`cat old_gitrev.txt`"
  pkg-config --atleast-version=0.5 xcm
  if [ $? -ne 0 ] || [ "$git_version" != "$old_git_version" ]; then
    echo "xcm `pkg-config --modversion xcm`"
    update_xcm=1
    if [ $UNAME_ = "MINGW32_NT-6.1" ]; then
      xcolor_skip="--disable-libX11"
    fi
    CFLAGS="$CFLAGS $OSX_ARCH_LIBRARY" CXXFLAGS="$CXXFLAGS $OSX_ARCH_LIBRARY" LDFLAGS="$LDFLAGS $OSX_ARCH_LIBRARY" ./configure $v $conf_opts $xcolor_skip $@
    make $MAKE_CPUS
    make install
  else
    echo no changes in git $git_version
  fi
  echo "$git_version" > old_gitrev.txt
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"


# SANE
UNAME_=`uname`
if [ $UNAME_ = "Darwin" ] || [ `echo "$skip" | grep sane | wc -l` -ne 0 ]; then
  echo sane skipped
else
  git_repo=sane-backends
  if [ -d $git_repo ]; then
    cd $git_repo
    echo revert old patches ...
    patch -p1 -R < patch_old.patch
    cd "$top"
  fi
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    echo git://git.debian.org/git/sane/sane-backends.git
    git clone git://git.debian.org/git/sane/$git_repo.git
    cd $git_repo
    git checkout master
  fi
  if [ $verbose -gt 0 ]; then sleep 2; fi

  git_version="`cat .git/refs/heads/master`"

  url=http://alioth.debian.org/tracker/download.php/30186/410366/312641/3945
  packet_file=sane_cap_colour.patch
  checksum=4665a1e4b7b9b920a10b830b354ee32667eaefd6
  echo download and apply $packet_file
  if [ -f $packet_file ]; then
    a=1
  else
    which curl && curl -L $url/$packet_file -o $packet_file || wget $url/$packet_file
  fi
  if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
    echo sha1sum for $packet_file passed
  else
    echo sha1sum for $packet_file failed
    exit 1
  fi
  patch -p1 < $packet_file

  url=http://alioth.debian.org/tracker/download.php/30186/410366/312641/3946
  packet_file=sane_cap_colour_plustek.patch
  checksum=4198052440777e8697a9adf1c86844b4a143c6ba
  echo download and apply $packet_file
  echo download and apply $packet_file
  if [ -f $packet_file ]; then
    a=1
  else
    which curl && curl -L $url/$packet_file -o $packet_file || wget $url/$packet_file
  fi
  if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
    echo sha1sum for $packet_file passed
  else
    echo sha1sum for $packet_file failed
    exit 1
  fi
  patch -p1 < $packet_file

  url=http://alioth.debian.org/tracker/download.php/30186/410366/312641/3947
  packet_file=sane_cap_colour_backends.patch
  checksum=3dc60111bb371fc191387f144dc977a33b232b59
  echo download and apply $packet_file
  if [ -f $packet_file ]; then
    a=1
  else
    which curl && curl -L $url/$packet_file,gz -o $packet_file.gz || wget $url/$packet_file.gz
    gzip -d $packet_file.gz
  fi
  if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
    echo sha1sum for $packet_file passed
  else
    echo sha1sum for $packet_file failed
    exit 1
  fi
  patch -p1 < $packet_file

  git diff > patch_old.patch
  if [ $verbose -gt 0 ]; then sleep 1; fi

  if [ -f configure ]; then
    echo ""
  else
    aclocal -I m4
    libtoolize -f
    automake --add-missing
    autoconf
    if [ $verbose -gt 0 ]; then sleep 2; fi
  fi
  
  if [ -f config.log ]; then
    echo ""
  else
    CFLAGS="$CFLAGS $FPIC" CXXFLAGS="$CXXFLAGS $FPIC" ./configure --enable-pnm-backend $conf_opts $@
  fi

  old_git_version="`cat old_gitrev.txt`"
  if [ "$git_version" != "$old_git_version" ]; then
    echo "$git_version" != "$old_git_version"
    make $MAKE_CPUS
    make install
  else
    echo no changes in git $git_version
  fi
  echo "$git_version" > old_gitrev.txt
fi
if [ $verbose -gt 0 ]; then sleep 2; fi

cd "$top"


#LibRaw
git_repo=LibRaw
if [ `echo "$skip" | grep $git_repo | wc -l` -ne 0 ]; then
  echo $git_repo skipped
else
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    echo git://github.com/LibRaw/LibRaw.git
    git clone git://github.com/$git_repo/$git_repo.git
    cd $git_repo
    git checkout master
  fi
  if [ $verbose -gt 0 ]; then sleep 2; fi
  if [ -f configure ]; then
    echo ""
  else
    aclocal -I m4
    libtoolize -f
    automake --add-missing
    autoconf
  fi
  git_version="`cat .git/refs/heads/master`"
  old_git_version="`cat old_gitrev.txt`"
  if [ "$git_version" != "$old_git_version" ]; then
    CFLAGS="$CFLAGS $OSX_ARCH_LIBRARY $FPIC" CXXFLAGS="$CXXFLAGS $OSX_ARCH_LIBRARY $FPIC" LDFLAGS="$LDFLAGS $OSX_ARCH_LIBRARY" ./configure --enable-openmp --enable-lcms=no $conf_opts $@
    if [ $verbose -gt 0 ]; then sleep 2; fi
    make $MAKE_CPUS
    make install
    echo "$git_version" > old_gitrev.txt
  fi
fi
if [ $verbose -gt 0 ]; then sleep 2; fi

cd "$top"

#http://sourceforge.net/projects/openicc/files/OpenICC-Profiles/icc-profiles-openicc-1.3.0.tar.gz/download
packet=icc-profiles-openicc
packet_dir=$packet-1.3.1
packet_file=$packet_dir.tar.bz2
checksum=ddcad40c0e4805cb82d727aaea41a498c6a927e4
loc=http://downloads.sourceforge.net/project/openicc/OpenICC-Profiles/
if [ -f $packet_file ]; then
  echo $packet_file already here
else
  echo downloading http://downloads.sourceforge.net/project/openicc/OpenICC-Profiles/$packet_file
  which curl && curl -L $loc$packet_file -o $packet_file || wget $loc$packet_file
fi
if [ $verbose -gt 0 ]; then sleep 1; fi
if [ `echo "$skip" | grep openicc | wc -l` -ne 0 ]; then
  echo openicc skipped
else
if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
  echo sha1sum for $packet_file passed
  echo unpacking $packet_file ...
  tar xjf $packet_file
  if [ -d $packet_dir ]; then
    echo $packet_dir in place
    cd $packet_dir
    ./configure $v $conf_opts $@
    make
    make install
  fi
  echo hier: `pwd`
else
  echo sha1sum for $packet_file failed
  exit 1
fi
fi

cd "$top"

if [ $verbose -gt 0 ]; then sleep 2; fi

# OpenICC default profiles II
packet=icc-profiles-basiccolor-printing2009
packet_dir=$packet-1.2.0
packet_file=$packet_dir.tar.gz
checksum=74612fd2c5cced04001c5d42d8d7855e267d1d05
loc=http://downloads.sourceforge.net/project/openicc/basICColor-Profiles/
if [ -f $packet_file ]; then
  echo $packet_file already here
else
  echo downloading http://downloads.sourceforge.net/project/openicc/basICColor-Profiles/$packet_file
  which curl && curl -L $loc$packet_file -o $packet_file || wget $loc$packet_file
fi
if [ $verbose -gt 0 ]; then sleep 1; fi
if [ `echo "$skip" | grep basiccolor | wc -l` -ne 0 ]; then
  echo basiccolor skipped
else
if [ `$SHA1SUM $packet_file | grep $checksum | wc -l` -eq 1 ]; then
  echo sha1sum for $packet_file passed
  echo unpacking $packet_file ...
  tar xzf $packet_file
  if [ -d $packet_dir ]; then
    echo $packet_dir in place
    cd $packet_dir
    ./configure $v $conf_opts $@
    make
    make install
  fi
  echo hier: `pwd`
else
  echo sha1sum for $packet_file failed
  exit 1
fi
fi

cd "$top"

if [ $verbose -gt 0 ]; then sleep 2; fi


# Oyranos
git_repo=oyranos
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone git://github.com/oyranos-cms/$git_repo $git_repo
    cd $git_repo
    git checkout master
    mkdir build
  fi
  echo updated libXcm $update_xcm
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  if [[ ! -d build ]]; then
    mkdir build
    update_xcm=1
  fi
  cd build
  old_git_version="`cat old_gitrev.txt`"
  update_oyranos=0
  pkg-config --atleast-version=0.9 oyranos
  if [ $? -ne 0 ] || [ $update_xcm = 1 ] ||
     [ "$git_version" != "$old_git_version" ]; then
    echo "oyranos `pkg-config --modversion oyranos`"
    update_oyranos=1
    if [ $UNAME_ = "MINGW32_NT-6.1" ]; then
      x11_skip="--disable-libX11"
      cmake "$cmake_target" -DCMAKE_C_FLAGS="$CFLAGS" -DCMAKE_CXX_FLAGS="$CXXFLAGS" -DCMAKE_LD_FLAGS="$LDFLAGS" -DCMAKE_INSTALL_PREFIX="$prefix" -DXDG_CONFIG_DIR="$HOME/.local/xdg" -DLIB_SUFFIX=$BARCH -DCMAKE_BUILD_TYPE=debugfull ..
      make
    else
      CFLAGS="$CFLAGS $OSX_ARCH_LIBRARY" CXXFLAGS="$CXXFLAGS $OSX_ARCH_LIBRARY" LDFLAGS="$LDFLAGS $OSX_ARCH_LIBRARY" ../configure $conf_opts $@  $v --enable-debug --xdgsysdir="$HOME/.local/xdg" $x11_skip
      make $MAKE_CPUS
    fi
    make install
    make check
  else
    echo no changes in git $git_version
  fi
  echo "$git_version" > old_gitrev.txt
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"


# Xcm
git_repo=xcm
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone git://github.com/oyranos-cms/$git_repo $git_repo
    cd $git_repo
    git checkout master
  fi
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  old_git_version="`cat old_gitrev.txt`"
  if [ "$git_version" != "$old_git_version" ]; then
    ./configure $v $conf_opts $@
    make $MAKE_CPUS
    make install
  else
    echo no changes in git $git_version
  fi
  echo "$git_version" > old_gitrev.txt
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"


# CompIcc
git_repo=compicc
if [ `echo "$skip" | grep $git_repo | wc -l` -ne 0 ]; then
  echo $git_repo skipped
else
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone git://$git_repo.git.sourceforge.net/gitroot/$git_repo/$git_repo
    cd $git_repo
    git checkout master
  fi
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  old_git_version="`cat old_gitrev.txt`"
  if [ $update_oyranos = 1 ] || [ "$git_version" != "$old_git_version" ]; then
    ./configure $v $conf_opts $@
    make $MAKE_CPUS
    make install
  else
    echo no changes in git $git_version
  fi
  echo "$git_version" > old_gitrev.txt
fi
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"

# libCmpx
git_repo=bekus-libcmpx
if [ `echo "$skip" | grep $git_repo | wc -l` -ne 0 ]; then
  echo $git_repo skipped
else
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone https://gitorious.org/libcmpx/$git_repo.git
    cd $git_repo
    git checkout master
  fi
  if [[ ! -d build ]]; then
    mkdir build
    update_oyranos=1
  fi
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  old_git_version="`cat old_gitrev.txt`"
  if [ $update_oyranos = 1 ] || [ "$git_version" != "$old_git_version" ]; then
    cd build
    cmake "$cmake_target" -DCMAKE_C_FLAGS="$CFLAGS $OSX_ARCH_LIBRARY" -DCMAKE_CXX_FLAGS="$CXXFLAGS $OSX_ARCH_LIBRARY" -DCMAKE_LD_FLAGS="$LDFLAGS $OSX_ARCH_LIBRARY" -DCMAKE_INSTALL_PREFIX=$prefix -DLIBRARY_OUTPUT_PATH=$libdir -DCMAKE_BUILD_TYPE=debugfull ..
    make
    make install
  else
    echo no changes in git $git_version
  fi
  cd "$top/$git_repo"
  echo "$git_version" > old_gitrev.txt
fi
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"


# Synnefo
git_repo=bekus-synnefo
if [ `echo "$skip" | grep $git_repo | wc -l` -ne 0 ]; then
  echo $git_repo skipped
else
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone https://gitorious.org/synnefo/$git_repo.git
    cd $git_repo
    git checkout master
  fi
  if [[ ! -d build ]]; then
    mkdir build
    update_oyranos=1
  fi
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  old_git_version="`cat old_gitrev.txt`"
  if [ $update_oyranos = 1 ] || [ "$git_version" != "$old_git_version" ]; then
    cd build
    cmake "$cmake_target" -DCMAKE_C_FLAGS="$CFLAGS $OSX_ARCH_COCOA" -DCMAKE_CXX_FLAGS="$CXXFLAGS $OSX_ARCH_COCOA" -DCMAKE_LD_FLAGS="$LDFLAGS $OSX_ARCH_COCOA" -DCMAKE_INSTALL_PREFIX=$prefix -DCMAKE_BUILD_TYPE=debugfull ..
    make
    make install
  else
    echo no changes in git $git_version
  fi
  cd "$top/$git_repo"
  echo "$git_version" > old_gitrev.txt
fi
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"


# kolor-manager
git_repo=kolor-manager
if [ `echo "$skip" | grep $git_repo | wc -l` -ne 0 ]; then
  echo $git_repo skipped
else
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone git://anongit.kde.org/$git_repo.git
    cd $git_repo
    git checkout master
  fi
  if [[ ! -d build ]]; then
    mkdir build
    update_oyranos=1
  fi
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  old_git_version="`cat old_gitrev.txt`"
  if [ $update_oyranos = 1 ] || [ "$git_version" != "$old_git_version" ]; then
    cd build
    cmake "$cmake_target" -DCMAKE_INSTALL_PREFIX=$kde_prefix -DCMAKE_BUILD_TYPE=debugfull ..
    make
    make install
    kbuildsycoca4
  else
    echo no changes in git $git_version
  fi
  cd "$top/$git_repo"
  echo "$git_version" > old_gitrev.txt
fi
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"


# cinepaint
git_repo=cinepaint_ng
if [ `echo "$skip" | grep cinepaint | wc -l` -ne 0 ]; then
  echo cinepaint skipped
else
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone https://gitorious.org/$git_repo/$git_repo.git
    cd $git_repo
    git checkout master
  fi
  if [[ ! -f Makefile ]]; then
  if [ $verbose -gt 0 ]; then sleep 2; fi
    sleep 2
    ./configure --enable-debug --disable-icc_examin --disable-pygimp $conf_opts $@
  fi
  echo updated oyranos $update_oyranos
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  old_git_version="`cat old_gitrev.txt`"
  if [ $update_oyranos = 1 ] || [ "$git_version" != "$old_git_version" ]; then
    make $MAKE_CPUS
    make install
  else
    echo no changes in git $git_version
  fi
  echo "$git_version" > old_gitrev.txt
fi
if [ $verbose -gt 0 ]; then sleep 1; fi
sleep 1

cd "$top"


# ICC Examin
git_repo=icc-examin
  echo checkout $git_repo
  if [ -d $git_repo ]; then
    cd $git_repo
    git pull
  else
    git clone git://github.com/oyranos-cms/$git_repo $git_repo
    cd $git_repo
    git checkout master
  fi
  if [ $verbose -gt 0 ]; then sleep 2; fi
  git_version="`cat .git/refs/heads/master`"
  old_git_version="`cat old_gitrev.txt`"
  if [[ ! -d build ]]; then
    mkdir build
  fi
  if [ $update_oyranos = 1 ] || [ "$git_version" != "$old_git_version" ] ||
     [ ! -f "$target" ]; then
    cd build
    cmake "$cmake_target" -DCMAKE_C_FLAGS="$CFLAGS $OSX_ARCH_COCOA" -DCMAKE_CXX_FLAGS="$CXXFLAGS $OSX_ARCH_COCOA" -DCMAKE_LD_FLAGS="$LDFLAGS $OSX_ARCH_COCOA" -DCMAKE_INSTALL_PREFIX="$prefix" -DCMAKE_BUILD_TYPE=debugfull ..
    make $MAKE_CPUS
    if [ $? = 0 ] && [ $UNAME_ = "Darwin" ]; then
      make bundle
    fi
  else
    echo no changes in git $git_version
  fi
  echo "$git_version" > old_gitrev.txt
#make install
if [ $verbose -gt 0 ]; then sleep 1; fi

cd "$top"

echo ""
if [ -f "$git_repo/build/$target" ]; then
  echo ICC Examin is in $git_repo/$target
  echo You can test it now with one of:
  echo   $git_repo/build/$target icc-profiles-basiccolor-printing2009-1.2.0/default_profiles/printing/ISOcoated_v2_bas.ICC
  echo   hint: the 3D gamut hull is shown with the Ctrl-h shortcut
else
  echo Could not build $git_repo/build/$target
fi

