include config

CC=c++
MAKEDEPEND	= /usr/X11R6/bin/makedepend -Y
RM = rm -v
COPY = cp -v

prefix		= /opt/local
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
datadir		= ${prefix}/share
includedir	= ${prefix}/include
libdir		= ${exec_prefix}/lib
mandir		= ${prefix}/man
srcdir		= .

DEBUG = -DDEBUG
DL = --ldflags # --ldstaticflags


ifdef APPLE
  OPTS=-Wall -g $(DEBUG)
  GLUT = -framework GLUT -lobjc
  OSX_CPP = icc_helfer_osx.cpp
  INCL=-I$(includedir) -I/usr/X11R6/include -I./ -I/usr/include/gcc/darwin/default/c++
else
  OPTS = -Wall  -Os -g $(DEBUG) #-fomit-frame-pointer -g
  GLUT = -lglut
  INCL=-I$(includedir) -I/usr/X11R6/include -I./
endif

ifdef FLTK
  FLTK_LIBS=`fltk-config --use-images --use-gl --use-glut $(DL)`
endif

ifdef FLU
  FLU_LIBS=`flu-config $(DL)`
endif

ifdef X11
  X_CPP = icc_helfer_x.cpp
  X11_LIBS=-L/usr/X11R6/lib -lX11 -lXxf86vm -lXext
endif

ifdef OY
  ifdef X11
    OY_LIBS = -loyranos_moni
  endif
  OYRANOS_LIBS = -lkdb -loyranos $(OY_LIBS)
  OYRANOS_H = -I/usr/include
endif

CXXFLAGS=$(OPTS) $(INCL) $(FLU_H) $(FLTK_H) $(X_H) $(OSX_H) $(OY_H)

LDLIBS = -L$(libdir) -L./ -L/opt/kai-uwe/lib $(FLTK_LIBS) \
	$(X11_LIBS) -llcms $(OYRANOS_LIBS) $(GLUT) $(FLU_LIBS)

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

timedir = .
mtime   = `find $(timedir) -prune -printf %Ty%Tm%Td.%TT | sed s/://g`

#.SILENT:

all:	mkdepend $(TARGET)

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

install:	$(TARGET)
	$(COPY) icc_examin $(bindir)

uninstall:
	$(RM) $(bindir)/icc_examin

clean:
	echo "mache sauber"
	$(RM) mkdepend config config.h
	$(RM) $(OBJECTS) $(TARGET)

config:
	configure.sh

mkdepend:
	echo "" > mkdepend
	$(MAKEDEPEND) -f mkdepend -s "#Bitte stehen lassen" -I. $(CXXFLAGS) $(SOURCES)


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
	mkdir icc_examin_$(VERSION)
	$(COPY) \
	$(SOURCES) \
	$(CPPFLTKFILES) \
	makefile \
	configure.sh \
	$(TEST) \
	$(DOKU) \
	$(FLUID) \
	icc_examin_$(VERSION)
	tar cf - icc_examin_$(VERSION)/ \
	| gzip > $(TARGET)_$(mtime).tgz
	test -d ../Archiv && mv -v $(TARGET)_*.tgz ../Archiv
	test -d icc_examin_$(VERSION) && \
	test `pwd` != `(cd icc_examin_$(VERSION); pwd)` && \
	rm -R icc_examin_$(VERSION) 

# Abhängigkeiten
include mkdepend

