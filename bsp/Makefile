# automatically generated file by configure - do not edit


LINUX = 1
OSX_H = 
EXEC_END = 
MAKEFILE_ARCH = Makefile

CC = cc
CXX = c++
LIB=/lib
MAKEDEPEND	= cc -MM
COPY = cp  -dpa
RM = rm -f 
INSTALL = install 
COLLECT = ar cru
RANLIB = ranlib
LINK = ln -s

MSGMERGE = msgmerge
MSGMERG_OPTS = --update
XGETTEXT_OPTIONS = --keyword=gettext --flag=gettext:1:pass-c-format         --keyword=_ --flag=_:1:pass-c-format         --keyword=N_ --flag=N_:1:pass-c-format         --copyright-holder='Kai-Uwe Behrmann'         --msgid-bugs-address='ku.b@gmx.de'         --from-code=utf-8
REZ = 


LIBMID = .so
LIBEXT = 
LINK_LIB_PATH = 
LINK_SRC_PATH = 
I18N_LIB = $(LIBNAME)
I18N_LIBSTAT = $(LIBNAME)
INCL = -I$(includedir) -I/usr/X11R6/include -I. -I/usr/include/g++ -I/usr/include
OSX_CPP = 
LINK_FLAGS = -shared $(dl) -fPIC -L. -lpthread
CFLAGS   =   $(INCL) $(ARGYLL_H) $(DEBUG)
CXXFLAGS =   $(INCL) $(ARGYLL_H) $(DEBUG)
LDFLAGS  = -L.  $(dl) $(iconv)


dl = -ldl
c = -lc
m = -lm
XF86VMODE = 1
XF86VMODE_INC = -I/usr/X11R6/include
XF86VMODE_LIB = -lXxf86vm
XIN = 1
XINERAMA_INC = -I/usr/X11R6/include
XINERAMA_LIB = -lXinerama
X11_LIB_PATH = -L/usr/X11R6/lib$(BARCH) -L/usr/lib$(BARCH) -L$(libdir)
X11_INCL=$(XF86VMODE_INC) $(XINERAMA_INC)
X11_LIBS=$(X11_LIB_PATH) -lX11 $(XF86VMODE_LIB) -lXpm -lXext $(XINERAMA_LIB)
X11 = X11
X_H = -I/usr/X11R6/include -I/usr/include -I/usr/X11R6/include $(X11_INCL)
DEBUG = -Wall -g -DDEBUG --pedantic
DEBUG_SWITCH = -v
.SILENT:
src_dir = /home/kuwe/programme/icc_examin/icc_examin-0.45
srcdir = /home/kuwe/programme/icc_examin/Entwickeln
prefix=/opt/kai-uwe
exec_prefix=/opt/kai-uwe
bindir=/opt/kai-uwe/bin
sbindir=/opt/kai-uwe/sbin
libdir=/opt/kai-uwe/lib
includedir=/opt/kai-uwe/include
datadir=/opt/kai-uwe/share
mandir=/opt/kai-uwe/share/man
pixmapdir=/opt/kai-uwe/share/pixmaps
desktopdir=/opt/kai-uwe/share/applications


### End of automatic generated options ###


TARGET=bsp
VERSION=1.0.1

BINTARGET = $(TARGET)

HEADERS	= bsp.h GraphicsGems.h GGems.h
COMMON_CFILES  = bspAlloc.c bspCollide.c bspPartition.c \
bspTree.c bspUtility.c bspMemory.c
TARGET_CFILES = mainBsp.c

LIBNAME = lib$(TARGET).a

CXXFLAGS_= $(CXXFLAGS)
CFLAGS_  = $(CFLAGS)

LDLIBS_ = -L./ $(LDFLAGS) \
	-L/lib \
	$(DBG_LIBS) $(dl) $(m) $(c)

C_HEADERS = \
	$(HEADERS)

CFILES = \
	$(COMMON_CFILES)
CPPFILES = \
	$(COMMON_CPPFILES) \
	$(TOOLKIT_FILES)
CXXFILES = \
	$(COMMON_CXXFILES)
TEST_CPP =
TEST_H = 

ALL_SOURCEFILES = \
	$(COMMON_CFILES) \
	$(COMMON_CPPFILES) \
	$(FLTK_CPPFILES) \
	$(TARGET_CFILES) \
	$(CXXFILES) \
	$(TEST_CPP)

ALL_HEADERFILES = \
	$(C_HEADERS) \
	$(TEST_H)

DOKU = \
	TODO \
	README \
	ChangeLog \
	COPYING \
	BUGS \
	AUTHORS \
	INSTALL \
	foo.dat

SOURCES = $(ALL_SOURCEFILES) $(ALL_HEADERFILES)
OBJECTS = $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o)
TARGET_OBJECTS = $(TARGET_CFILES:.c=.o)


ALL_FILES =	$(SOURCES) \
	Makefile \
	makefile.in \
	$(DOKU)

# get time stamp
C := $(shell date +%C)
MTIME   := $(shell date +%Y%m%e.%H%M%S | sed s/$C//)


all:	dynamic
	
BASE =  
	

dynamic:	$(BINTARGET)

$(BINTARGET):	$(BASE) $(TARGET_OBJECTS) $(LIBNAME)
	echo Link $@ ...
	-$(CC) $(CXXFLAGS_) $(LDFLAGS) -o $(BINTARGET) \
	$(TARGET_OBJECTS) $(LIBNAME) \
	$(LDLIBS_) $(LINK_LIB_PATH) $(LINK_SRC_PATH)
	$(REZ)


$(LIBNAME):	$(OBJECTS)
	echo Link $@ ...
	$(COLLECT) $(LIBNAME) $(OBJECTS)
	$(RANLIB) $(LIBNAME)

strip: $(BINTARGET)
	echo strip $(BINTARGET)
	strip $(BINTARGET)

static_static:	$(OBJECTS)
	$(CXX) $(OPTS) -o $(BINTARGET) \
	$(OBJECTS) \
	$(LDLIBS_) -static  $(dl) $(m) $(c) \
	-L/opt/local/lib
	$(REZ)



clean:
	echo clean $(TARGET) ...
	-$(RM) $(OBJECTS) $(TARGET_OBJECTS) $(BINTARGET) \
	$(LIBNAME)
	echo ... $@ done


# Build commands and filename extensions...
.SUFFIXES:	.0 .1 .3 .c .cxx .h .fl .man .o .z $(EXEEXT)

.c.o:
	echo Compiling $< ...
	$(CC) $(CFLAGS_) -c $<

.cxx.o:
	echo Compiling $< ...
	$(CXX) $(CXXFLAGS_) -c $<

.cpp.o:
	echo Compiling $< ...
	$(CXX) $(CXXFLAGS_) -c $<

.fl.cxx:
	echo Expanding $< ...
	fluid -c $<

.po:
	echo Generating $@ ...
	msgfmt $<

tgz:
	make DESTDIR=Entwickeln copy_files
	tar cf - Entwickeln/ \
	| gzip > $(TARGET)_$(MTIME).tgz
	test -d Entwickeln && \
	test `pwd` != `(cd Entwickeln; pwd)` && \
	$(RM) -R Entwickeln
	test -d ../../Archiv && mv -v $(TARGET)_*.tgz ../../Archiv

targz:
	make DESTDIR=$(TARGET)_$(VERSION) copy_files
	tar cf - $(TARGET)_$(VERSION)/ \
	| gzip > $(TARGET)_$(MTIME).tgz
	test -d $(TARGET)_$(VERSION) && \
	test `pwd` != `(cd $(TARGET)_$(VERSION); pwd)` && \
	$(RM) -R $(TARGET)_$(VERSION) 
	test -d ../../Archiv && mv -v $(TARGET)_*.tgz ../../Archiv || echo no copy

copy_files:
	echo Copy all $(TARGET) files ...
	mkdir $(DESTDIR)
	$(COPY) -R \
	$(ALL_FILES) \
	$(DESTDIR)


dist: $(BASE) targz
	test -f && $(COPY) ../../Archiv/$(TARGET)_$(MTIME).tgz $(TARGET)_$(VERSION).tar.gz || $(COPY) $(TARGET)_$(MTIME).tgz $(TARGET)_$(VERSION).tar.gz

$(TARGET).spec:
	./configure


.PHONY: install uninstall


