export PATH=$PATH:/home/kuwe/.local/bin
export LIBDIR=$HOME/.local/lib64
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$LIBDIR/pkgconfig
./icc_examin-build.sh --prefix=$HOME/.local --libdir=$LIBDIR
export LD_LIBRARY_PATH=$LIBDIR:$LD_LIBRARY_PATH
if [ -f "icc_examin/iccexamin" ]; then
  icc_examin/iccexamin icc-profiles-basiccolor-printing2009-1.2.0/default_profiles/printing/ISOcoated_v2_bas.ICC
fi
