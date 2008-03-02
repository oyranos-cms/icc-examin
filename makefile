include config

CC=c++
MAKEDEPEND	= /usr/X11R6/bin/makedepend -Y
RM = rm -v

prefix		= /opt/local
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
datadir		= ${prefix}/share
includedir	= ${prefix}/include
libdir		= ${exec_prefix}/lib
mandir		= ${prefix}/man
srcdir		= .

DEBUG = -DDEBUG
ifdef FLTK
FLTK_H = -DHAVE_FLTK
endif
DL = --ldflags # --ldstaticflags


VRML_LIBS=$(FLTK_GL_LIBS) -lGL -lopenvrml -lopenvrml-gl -lpng -ljpeg \
 -lXinerama -lXft

X11_LIBS=-L/usr/X11R6/lib -lX11 -lXxf86vm -lXext

FLTK_LIBS=`fltk-config --use-images --use-gl --use-glut $(DL)`


ifdef FLU
FLU_LIBS=`flu-config $(DL)`
endif

FLTK_GL_LIBS=-lfltk_gl

ifdef APPLE
  OPTS=-Wall -g $(DEBUG)
  GLUT = -framework GLUT -lobjc
  OSX_CPP = icc_helfer_osx.cpp
  INCL=-I$(includedir) -I/usr/X11R6/include -I./ -I/opt/kai-uwe/include -I/usr/include/gcc/darwin/default/c++
else
  OPTS = -Wall  -Os -g $(DEBUG) #-fomit-frame-pointer -g
  GLUT = -lglut
  INCL=-I$(includedir) -I/usr/X11R6/include -I./ -I/opt/kai-uwe/include
endif

ifdef X11
  X_CPP = icc_helfer_x.cpp
endif

ifdef OY
  OY_LIBS = -loyranos -loyranos_moni
  OYRANOS_LIBS=-lkdb -loyranos
endif

CXXFLAGS=$(OPTS) $(INCL) $(FLU_H) $(FLTK_H) $(X_H) $(OSX_H) $(OY_H)

LDLIBS = -L$(libdir) -L./ -L/opt/kai-uwe/lib $(FLTK_LIBS) \
	$(X11_LIBS) -llcms $(OY_LIBS) $(GLUT) $(FLU_LIBS)

CPP_HEADERS = \
	agviewer.h \
	icc_betrachter.h \
	cccie64.h \
	ciexyz64_1.h \
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
	icc_info.h \
	icc_icc.h \
	icc_kette.h \
	icc_measurement.h \
	icc_modell_beobachter.h \
	fl_oyranos.h \
	icc_oyranos.h \
	icc_profile.h \
	icc_profile_header.h \
	icc_profile_tags.h \
	icc_speicher.h \
	icc_ueber.h \
	icc_utils.h \
	icc_version.h \
	icc_vrml.h \
	icc_vrml_parser.h
CPPFILES = \
	icc_cgats_filter.cpp \
	icc_draw.cpp \
	icc_examin.cpp \
	icc_fenster.cpp \
	icc_formeln.cpp \
	icc_gl.cpp \
	icc_helfer.cpp \
	icc_info.cpp \
	icc_kette.cpp \
	$(X_CPP) \
	$(OSX_CPP) \
	icc_main.cpp \
	icc_measurement.cpp \
	icc_modell_beobachter.cpp \
	icc_oyranos.cpp \
	icc_profile.cpp \
	icc_profile_header.cpp \
	icc_profile_tags.cpp \
	icc_ueber.cpp \
	icc_utils.cpp \
	icc_vrml.cpp \
	icc_vrml_parser.cpp \
	agviewer.cpp
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

release:	icc_alles.o
	echo Linking $@...
	$(CC) $(OPTS) -o $(TARGET) \
	icc_alles.o \
	$(LDLIBS)
	$(APPLE)
	$(RM) icc_alles.o

$(TARGET):	$(OBJECTS)
	echo Linking $@...
	$(CC) $(OPTS) -o $(TARGET) \
	$(OBJECTS) \
	$(LDLIBS)
	$(APPLE)

prof:	icc_profile.o icc_profilierer.o
	echo "Kopiliere icc_measurement_pur.o ..."
	c++ -I.. -Wall  -Os -DDEBUG  -I/opt/local/include -I/usr/X11R6/include -I./ -I/opt/kai-uwe/include -c icc_measurement.cpp -o icc_measurement_pur.o
	echo "Kopiliere icc_profilieren ..."
	c++ icc_profilieren.cpp -o icc_profilieren icc_profile_header.o icc_profile_tags.o icc_profile.o icc_cgats_filter.o icc_profilierer.o icc_helfer.o icc_utils.o icc_measurement_pur.o icc_formeln.o -llcms

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
	$(RM) test3
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
	$(RM) $(OBJECTS) $(TARGET)

it3:	t3
	cp test3 /opt/kai-uwe/bin/icc_examin

depend:
	$(MAKEDEPEND) -f makedepend -s "#Bitte stehen lassen" -I. $(CXXFLAGS) $(SOURCES)


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
	$(dir)/configure.sh \
	$(addprefix $(dir)/,$(TEST)) \
	$(addprefix $(dir)/,$(DOKU)) \
	$(addprefix $(dir)/,$(FLUID)) \
	| gzip > $(TARGET)_$(mtime).tgz
	mv -v $(TARGET)_*.tgz ../Archiv

# Abhängigkeiten
include makedepend

