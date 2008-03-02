CC=c++
MAKEDEPEND	= /usr/X11R6//bin/makedepend -Y
OPTS=-Wall -g #-O2

prefix		= /opt/local
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
datadir		= ${prefix}/share
includedir	= ${prefix}/include
libdir		= ${exec_prefix}/lib
mandir		= ${prefix}/man
srcdir		= .

#APPLE = 1
FLU = 1
DL = --ldflags # --ldstaticflags

ifdef FLU
FLU_H = -DHAVE_FLU
endif

CXXFLAGS=$(OPTS) $(INCL) $(FLU_H)
INCL=-I$(includedir) -I/usr/X11R6/include -I./

VRML_LIBS=$(FLTK_GL_LIBS) -lGL -lopenvrml -lopenvrml-gl -lpng -ljpeg \
 -lXinerama -lXft

X11_LIBS=-L/usr/X11R6/lib

FLTK_LIBS=`fltk-config --use-images --use-gl --use-glut $(DL)`

KDB_LIBS=#-lkdb

ifdef FLU
FLU_LIBS=`flu-config $(DL)`
endif

FLTK_GL_LIBS=-lfltk_gl

ifdef APPLE
  GLUT = -framework GLUT -lobjc
else
  GLUT = -lglut
endif

LDLIBS = -L$(libdir) -L./ $(FLTK_LIBS) \
	$(X11_LIBS) -llcms $(KDB_LIBS) $(GLUT) $(FLU_LIBS)

#	$(VRML_LIBS)

CPP_HEADERS = \
	agviewer.h \
	callback_simple.h \
	cccie64.h \
	ciexyz64_1.h \
	icc_betrachter.h \
	icc_draw.h \
	icc_formeln.h \
	icc_examin.h \
        icc_gl.h \
	icc_helfer.h \
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
	icc_draw.cpp \
	icc_examin.cpp \
	icc_formeln.cpp \
        icc_gl.cpp \
	icc_helfer.cpp \
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
CXXFILES = \
	icc_betrachter.cxx \
	fl_oyranos.cxx
TEST = \
	dE2000_test.cpp \
	ciede2000testdata.h
DOKU = \
        README \
        ChangeLog \
        COPYING \
        AUTHORS
FLUID = \
	icc_betrachter.fl \
	fl_oyranos.fl

SOURCES = $(CPPFILES) $(CXXFILES) $(CPP_HEADERS)
OBJECTS = $(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o)
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

prof:
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
	$(dir)/makefile \
	$(addprefix $(dir)/,$(TEST)) \
	$(addprefix $(dir)/,$(DOKU)) \
	$(addprefix $(dir)/,$(FLUID)) \
	| gzip > $(TARGET)_$(mtime).tgz
	mv -v $(TARGET)_*.tgz ../Archiv

