include config

CC=c++
MAKEDEPEND	= /usr/X11R6/bin/makedepend -Y
RM = rm -v
COPY = cp -vp

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

X_CPPFILES = icc_helfer_x.cpp
OSX_CPPFILES = icc_helfer_osx.cpp
FLTK_CPPFILES = icc_helfer_fltk.cpp

ifdef APPLE
  OPTS=-Wall -g $(DEBUG)
  GLUT = -framework GLUT -lobjc
  OSX_CPP = $(OSX_CPPFILES)
  INCL=-I$(includedir) -I/usr/X11R6/include -I./ -I/usr/include/gcc/darwin/default/c++
else
  OPTS = -Wall  -Os -g $(DEBUG) #-fomit-frame-pointer -g
  GLUT = -lglut
  INCL=-I$(includedir) -I/usr/X11R6/include -I./
endif

ifdef FLTK
  TOOLKIT_FILES = $(FLTK_CPPFILES)
endif

ifdef X11
  X_CPP = $(X_CPPFILES)
  X11_LIBS=-L/usr/X11R6/lib -lX11 -lXxf86vm -lXext
endif

CXXFLAGS=$(OPTS) $(INCL) \
			$(FLU_H) $(FLTK_H) $(X_H) $(OSX_H) $(OYRANOS_H) $(LCMS_H)

LDLIBS = -L$(libdir) -L./ -L/opt/kai-uwe/lib $(FLTK_LIBS) \
	$(X11_LIBS) -llcms $(OYRANOS_LIBS) $(GLUT) $(FLU_LIBS) $(LCMS_LIBS)

CPP_HEADERS = \
	agviewer.h \
	Fl_Slot.H \
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
	icc_vrml_parser.h \
	icc_waehler.h
COMMON_CPPFILES = \
	agviewer.cpp \
	icc_cgats_filter.cpp \
	icc_draw.cpp \
	icc_examin.cpp \
	icc_examin_histogram.cpp \
	icc_examin_io.cpp \
	icc_examin_tagwahl.cpp \
	icc_fenster.cpp \
	icc_formeln.cpp \
	icc_gl.cpp \
	icc_helfer.cpp \
	icc_info.cpp \
	icc_kette.cpp \
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
	icc_vrml_parser.cpp
CPPFILES = \
	$(COMMON_CPPFILES) \
	$(TOOLKIT_FILES) \
	$(X_CPP) \
	$(OSX_CPP)
CXXFILES = \
	icc_betrachter.cxx \
	fl_oyranos.cxx
TEST = \
	dE2000_test.cpp \
	ciede2000testdata.h
ALL_CPPFILES = \
	$(COMMON_CPPFILES) \
	$(OSX_CPPFILES) \
	$(X_CPPFILES) \
	$(FLTK_CPPFILES) \
	$(CXXFILES) \
	$(TEST)
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

SOURCES = $(ALL_CPPFILES) $(CPP_HEADERS)
OBJECTS = $(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o)
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
	mkdir Entwickeln
	$(COPY) \
	$(SOURCES) \
	makefile \
	configure.sh \
	$(DOKU) \
	$(FLUID) \
	Entwickeln
	tar cf - Entwickeln/ \
	| gzip > $(TARGET)_$(mtime).tgz
	test -d ../Archiv && mv -v $(TARGET)_*.tgz ../Archiv
	test -d Entwickeln && \
	test `pwd` != `(cd Entwickeln; pwd)` && \
	rm -R Entwickeln

targz:
	mkdir icc_examin_$(VERSION)
	$(COPY) \
	$(SOURCES) \
	makefile \
	configure.sh \
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

