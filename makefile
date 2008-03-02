CC=c++
MAKEDEPEND	= /usr/X11R6//bin/makedepend -Y
OPTS=-Wall -g -O2

prefix		= /opt/local
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
datadir		= ${prefix}/share
includedir	= ${prefix}/include
libdir		= ${exec_prefix}/lib
mandir		= ${prefix}/man
srcdir		= .


CXXFLAGS=$(OPTS) $(INCL)
INCL=-I$(includedir) -I/usr/X11R6/include -I./

VRML_LIBS=$(FLTK_GL_LIBS) -lGL -lopenvrml -lopenvrml-gl -lpng -ljpeg
X11_LIBS=-L/usr/X11R6/lib -lXinerama -lXft
FLTK_LIBS=`fltk-config --use-images --use-gl --ldstaticflags`
FLTK_GL_LIBS=-lfltk_gl
LDLIBS = -L./ $(FLTK_LIBS) \
	$(VRML_LIBS) \
	$(X11_LIBS) -llcms
# -llprof

CPP_HEADERS = \
	vFLGLWidget.h \
	ViewerFLTK.h \
	icc_draw.h \
	icc_examin.h \
	icc_utils.h \
	icc_profile.h \
	icc_vrml.h \
	cccie64.h \
	ciexyz64_1.h
CPPFILES = \
	vFLGLWidget.cpp \
	ViewerFLTK.cpp \
	icc_draw.cpp \
	icc_profile.cpp \
	icc_vrml.cpp
CXXFILES = \
	icc_examin.cxx
SOURCES = $(CPPFILES) $(CXXFILES) $(CPP_HEADERS)
OBJECTS = $(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o)
TARGET  = icc_examin

REZ     = /Developer/Tools/Rez -t APPL -o $(TARGET) /opt/local/include/FL/mac.r
APPLE   = #$(REZ)

topdir  = ..
dir     = $(TARGET)
timedir = $(topdir)/$(TARGET)
mtime   = `find $(timedir) -prune -printf %Ty%Tm%Td.%TT | sed s/://g`

#.SILENT:

all:	$(TARGET)

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

test:	icc_draw.o
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
	$(dir)/$(TARGET).fl \
	| gzip > $(TARGET)_$(mtime).tgz

