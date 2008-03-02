CC=c++
MAKEDEPEND	= /usr/X11R6//bin/makedepend -Y
OPTS=-Wall -g -Os

prefix		= /opt/local
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
datadir		= ${prefix}/share
includedir	= ${prefix}/include
libdir		= ${exec_prefix}/lib
mandir		= ${prefix}/man
srcdir		= .

#APPLE = 1
FLTK = 1
ifdef FLTK
FLU = 1
FLTK_H = -DHAVE_FLTK
endif
DL = --ldflags # --ldstaticflags

ifdef FLU
FLU_H = -DHAVE_FLU
endif

VRML_LIBS=$(FLTK_GL_LIBS) -lGL -lopenvrml -lopenvrml-gl -lpng -ljpeg \
 -lXinerama -lXft

X11_LIBS=-L/usr/X11R6/lib

FLTK_LIBS=`fltk-config --use-images --use-gl --use-glut $(DL)`

OYRANOS_LIBS=-lkdb -loyranos

ifdef FLU
FLU_LIBS=`flu-config $(DL)`
endif

FLTK_GL_LIBS=-lfltk_gl

ifdef APPLE
  GLUT = -framework GLUT -lobjc
  OSX_CPP = icc_helfer_osx.cpp
  OSX_H  = -DHAVE_OSX
else
  GLUT = -lglut
  X_H  = -DHAVE_X
  X_CPP = icc_helfer_x.cpp
  OY_H = -DHAVE_OY
  OY_LIBS = -loyranos -loyranos_moni
endif

CXXFLAGS=$(OPTS) $(INCL) $(FLU_H) $(FLTK_H) $(X_H) $(OSX_H) $(OY_H)
INCL=-I$(includedir) -I/usr/X11R6/include -I./ -I/opt/kai-uwe/include

LDLIBS = -L$(libdir) -L./ -L/opt/kai-uwe/lib $(FLTK_LIBS) \
	$(X11_LIBS) -llcms $(OY_LIBS) $(GLUT) $(FLU_LIBS)

#	$(VRML_LIBS)

CPP_HEADERS = \
	agviewer.h \
	callback_simple.h \
	cccie64.h \
	ciexyz64_1.h \
	icc_betrachter.h \
	icc_cgats_filter.h \
	icc_draw.h \
	icc_draw_fltk.h \
	icc_fenster.h \
	icc_formeln.h \
	icc_examin.h \
	icc_gl.h \
	icc_helfer.h \
	icc_helfer_fltk.h \
	icc_helfer_ui.h \
	icc_helfer_x.h \
	icc_icc.h \
	icc_kette.h \
	fl_oyranos.h \
	icc_oyranos.h \
	icc_profile.h \
	icc_ueber.h \
	icc_utils.h \
	icc_version.h \
	icc_vrml.h
#	vFLGLWidget.h \
	ViewerFLTK.h 
CPPFILES = \
	icc_cgats_filter.cpp \
	icc_draw.cpp \
	icc_examin.cpp \
	icc_fenster.cpp \
	icc_formeln.cpp \
	icc_gl.cpp \
	icc_helfer.cpp \
	$(X_CPP) \
	$(OSX_CPP) \
	icc_kette.cpp \
	icc_main.cpp \
	icc_measurement.cpp \
	icc_oyranos.cpp \
	icc_profile.cpp \
	icc_ueber.cpp \
	icc_utils.cpp \
	icc_vrml.cpp \
	agviewer.cpp
#	vFLGLWidget.cpp \
	ViewerFLTK.cpp 
CPPFLTKFILES = \
	icc_helfer_fltk.cpp
CXXFILES = \
	icc_betrachter.cxx \
	fl_oyranos.cxx
TEST = \
	dE2000_test.cpp \
	ciede2000testdata.h
DOKU = \
	TODO \
	README \
	ChangeLog \
	COPYING \
	BUGS \
	AUTHORS
FLUID = \
	icc_betrachter.fl \
	fl_oyranos.fl

ifdef FLTK
TOOLKIT_FILES = $(CPPFLTKFILES)
endif
SOURCES = $(CPPFILES) $(CXXFILES) $(CPP_HEADERS)
OBJECTS = $(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o) $(TOOLKIT_FILES:.cpp=.o)
TARGET  = icc_examin

REZ     = /Developer/Tools/Rez -t APPL -o $(TARGET) /opt/local/include/FL/mac.r
ifdef APPLE
APPLE   = $(REZ)
endif

topdir  = ..
dir     = Entwickeln
timedir = $(topdir)/$(dir)
mtime   = `find $(timedir) -prune -printf %Ty%Tm%Td.%TT | sed s/://g`

#.SILENT:

all:	$(TARGET)

$(TARGET):	$(OBJECTS)
	echo Linking $@...
	$(CC) $(OPTS) -o $(TARGET) \
	$(OBJECTS) \
	$(LDLIBS)
	$(APPLE)

prof:	icc_profile.o icc_profilierer.o
	c++ icc_profilieren.cpp -o icc_profilieren icc_profile.o icc_profilierer.o icc_helfer.o icc_utils.o icc_measurement.o icc_formeln.o -llcms

static:		$(OBJECTS)
	echo Linking $@...
	$(CC) $(OPTS) -o $(TARGET) \
	$(OBJECTS) \
	$(LDLIBS) -static -ljpeg -lpng -lX11 -lpthread -lz -ldl \
	-lfreetype -lfontconfig -lXrender -lGLU -lXext -lexpat
	$(APPLE)

test:	icc_formeln.o icc_utils.o
	$(CC) $(OPTS) $(INCL) -o dE2000_test.o -c dE2000_test.cpp
	$(CC) $(OPTS) -o dE2000_test dE2000_test.o icc_formeln.o icc_utils.o \
	-L$(libdir) -llcms
	$(APPLE)

test1:	icc_draw.o
	$(CC) $(OPTS) $(INCL) -o horseshoe.o -c horseshoe.cxx
	$(CC) $(OPTS) -o horseshoe horseshoe.o icc_draw.o \
	`fltk-config --ldstaticflags` -L$(libdir) -llcms
	$(APPLE)

t3:
	rm test3
	make test3

test3:  ViewerFLTK.o vFLGLWidget.o
	$(CC) $(OPTS) $(INCL) test3.cxx -o test3 $(FLTK_LIBS) \
	ViewerFLTK.o vFLGLWidget.o \
	$(VRML_LIBS) $(X11_LIBS)
#vFLGLWidget.o

agv:    agviewer.o agv_example.o
	$(CC) $(OPTS) $(INCL) -o agv \
	agviewer.o  agv_example.o $(FLTK_LIBS) $(X11_LIBS) -lglut

cgats:	icc_utils.h icc_utils.cpp icc_cgats_filter.cpp icc_cgats_parser.cpp
	$(CC) $(OPTS) -o cgats icc_cgats_parser.cpp icc_utils.o icc_cgats_filter.o


install:	$(TARGET)
	cp icc_examin $(bindir)
clean:
	rm -v $(OBJECTS) $(TARGET)

it3:	t3
	cp test3 /opt/kai-uwe/bin/icc_examin

depend:
	$(MAKEDEPEND) -fmakedepend -I.. $(SOURCES) $(CFILES) $(CPP_HEADERS)


# The extension to use for executables...
EXEEXT		= 

# Build commands and filename extensions...
.SUFFIXES:	.0 .1 .3 .c .cxx .h .fl .man .o .z $(EXEEXT)

.o$(EXEEXT):
	echo Linking $@...
	$(CXX) -I.. $(CXXFLAGS) $< $(LINKFLTK) $(LDLIBS) -o $@
	$(POSTBUILD) $@ ../FL/mac.r

.c.o:
	echo Compiling $<...
	$(CC) -I.. $(CFLAGS) -c $<

.cxx.o:
	echo Compiling $<...
	$(CXX) -I.. $(CXXFLAGS) -c $<

.cpp.o:
	echo Compiling $<...
	$(CXX) -I.. $(CXXFLAGS) -c $<

tgz:
	tar cf - -C $(topdir) \
	$(addprefix $(dir)/,$(SOURCES)) \
	$(addprefix $(dir)/,$(CPPFLTKFILES)) \
	$(dir)/makefile \
	$(addprefix $(dir)/,$(TEST)) \
	$(addprefix $(dir)/,$(DOKU)) \
	$(addprefix $(dir)/,$(FLUID)) \
	| gzip > $(TARGET)_$(mtime).tgz
	mv -v $(TARGET)_*.tgz ../Archiv

