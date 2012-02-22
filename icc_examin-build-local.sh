#environment
export inst=$HOME/.local
export kde_prefix=$HOME/.kde4/
export PATH=$PATH:$inst//bin
export LIBDIR=$inst/lib64
if [ -z $PKG_CONFIG_PATH ]; then
  export PKG_CONFIG_PATH=$LIBDIR/pkgconfig
else
  export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$LIBDIR/pkgconfig
fi

# uncomment below the packages you are not interessted in
# elektra,sane,LibRaw,compicc,synnefo,kolor-manager,cinepaint
# export skip=elektra,openicc,basiccolor,sane,LibRaw,compicc,synnefo,kolor-manager,cinepaint

#uncomment the below line to see less messages
# export verbose=0

# run main script
export LD_LIBRARY_PATH=$LIBDIR:$LD_LIBRARY_PATH
export LDFLAGS=-L$LIBDIR
./icc_examin-build.sh --prefix=$inst --libdir=$LIBDIR

#test
if [ -f "icc_examin/iccexamin" ]; then
  icc_examin/iccexamin icc-profiles-basiccolor-printing2009-1.2.0/default_profiles/printing/ISOcoated_v2_bas.ICC
fi
