# Abhaengigkeiten
-include config


BINTARGET = iccexamin

ifdef BUILD64_
CC =  cc-64
CXX = c++-64
else
CC = cc
CXX = c++
endif
COLLECT = ar cru
RANLIB = ranlib
MAKEDEPEND	= $(CXX) -MM
LINK = ln -s
RM = rm -vf
ifdef LINUX
COPY = cp -vdpa
else
  ifdef APPLE
  COPY = cp -v
  else
  COPY = cp
  endif
endif
INSTALL = install -v

exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
datadir		= ${prefix}/share
includedir	= ${prefix}/include
ifdef BUILD_64
  libdir		= ${exec_prefix}/lib64
else
  libdir		= ${exec_prefix}/lib
endif
mandir		= ${prefix}/man

DEBUG = -DDEBUG
DL = --ldflags # --ldstaticflags

X_CPPFILES = icc_helfer_x.cpp
OSX_CPPFILES = icc_helfer_osx.cpp
FLTK_CPPFILES = icc_helfer_fltk.cpp
I18N_CXXFILES = #fl_i18n.cxx fl_i18n_fl.cxx
I18N_HEADERS = #fl_i18n.H
MSGFMT = msgfmt -c --statistics
RPMARCH = `rpmbuild --showrc | awk '/^build arch/ {print $$4}'`

ifdef FLU
  FLU_FLTK_LIBS = $(FLU_LIBS) $(FLTK_LIBS)
else
  FLU_FLTK_LIBS = $(FLTK_LIBS)
endif

LIB_LIBS = -L$(libdir) -L./ $(FLU_FLTK_LIBS) \
	-llcms -L/lib \
	$(DBG_LIBS) -lintl

ifdef APPLE
  LIBEXT = .dylib
  OPTS=-Wall -g $(DEBUG) -DPIC -Wunused -fno-exceptions
  LIBLINK_FLAGS = -dynamiclib $(LIB_LIBS) 
  I18N_LIB = -lintl $(LIBNAME) #-liconv
  I18N_LIBSTAT = /opt/local/lib/libintl.a $(LIBNAME) #-liconv
  OSX_CPP = $(OSX_CPPFILES)
  INCL=-I$(includedir) -I/usr/X11R6/include -I./ -I/usr/include/gcc/darwin/default/c++
  REZ = \
      fltk-config --post $(BINTARGET);
  #MAKEDEPEND = /usr/X11R6/bin/makedepend -Y
  DBG_LIBS = #-lMallocDebug
  MSGMERGE = msgmerge
  XGETTEXT_OPTIONS = \
	--keyword=gettext \
	--keyword=_ \
	--keyword=N_
else
  SO = .so
    MSGMERGE = msgmerge
    MSGMERG_OPTS = --update
    XGETTEXT_OPTIONS = \
	--keyword=gettext --flag=gettext:1:pass-c-format \
	--keyword=_ --flag=_:1:pass-c-format \
	--keyword=N_ --flag=N_:1:pass-c-format \
	--copyright-holder='Kai-Uwe Behrmann' \
	--msgid-bugs-address='ku.b@gmx.de' \
	--from-code=utf-8
  ifdef LINUX
    OPTS = -Wall -g $(DEBUG) -Wunused -fPIC -fno-exceptions #-Os -fomit-frame-pointer
    INCL=-I$(includedir) -I/usr/X11R6/include -I. -I/usr/include/g++ -I/usr/include
    LIBLINK_FLAGS = -shared -ldl -L.
    LIBSONAME = lib$(TARGET)$(SO).$(VERSION_A)
    LINK_NAME = -Wl,-soname -Wl,$(LIBSONAME)
    LINK_LIB_PATH = -Wl,--rpath -Wl,$(libdir)
    LINK_SRC_PATH = -Wl,--rpath -Wl,$(srcdir)
    I18N_LIB = $(LIBNAME)
    I18N_LIBSTAT = $(LIBNAME)
  else
    OPTS=-Wall -O2 -g $(DEBUG) -L. -Wunused -fno-exceptions -lc -lm
    RM = rm -f
    LIBLINK_FLAGS = -shared -ldl
    I18N_LIB = $(ICONV) -lintl $(LIBNAME)
    I18N_LIBSTAT = $(ICONV) -lintl $(LIBNAME)
  endif
endif

LIBSONAMEFULL = lib$(TARGET)$(SO).$(VERSION_L)$(LIBEXT)
LIBSONAME = lib$(TARGET)$(SO).$(VERSION_A)$(LIBEXT)
LIBSO = lib$(TARGET)$(SO)$(LIBEXT)
LIBNAME = 
LIBFL_I18N = fl_i18n/libfl_i18n.a

ifdef FLTK
  TOOLKIT_FILES = $(FLTK_CPPFILES)
endif

ifdef X11
  X_CPP = $(X_CPPFILES)
  ifdef BUILD_64
    X11_LIB_PATH=-L/usr/X11R6/lib64
  else
    X11_LIB_PATH=-L/usr/X11R6/lib
  endif
  ifdef XF86VMODE
  XF86VMODE_LIB = -lXxf86vm
  endif
  ifdef XIN
  XINERAMA_LIB = -lXinerama
  endif
  X11_LIBS=$(X11_LIB_PATH) -lX11 $(XF86VMODE_LIB) -lXext -lXpm $(XINERAMA_LIB)
endif

INCL_DEP = $(INCL) $(X_H) $(OSX_H) $(OYRANOS_H) \
			$(FLU_H) $(FLTK_H) $(FTGL_H) $(LCMS_H) $(ALL_SOURCEFILES)
ALL_INCL = $(INCL) \
			$(FLU_H) $(FLTK_H) $(X_H) $(OSX_H) $(OYRANOS_H) $(LCMS_H) $(FTGL_H)

CXXFLAGS=$(OPTS) $(ALL_INCL)
CFLAGS = $(OPTS) $(ALL_INCL)

LDLIBS = -L$(libdir) -L./ $(FLU_FLTK_LIBS) \
	$(X11_LIBS) -llcms -L/lib $(OYRANOS_LIBS) $(LCMS_LIBS) \
	$(FTGL_LIBS) $(I18N_LIB) $(DBG_LIBS) $(LIBNAME) $(LIBFL_I18N)

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
	icc_gamut.h \
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
	icc_oyranos_extern.h \
	icc_profile.h \
	icc_profile_header.h \
	icc_profile_tags.h \
	icc_speicher.h \
	icc_ueber.h \
	icc_utils.h \
	icc_version.h \
	icc_vrml.h \
	icc_vrml_parser.h \
	icc_waehler.h \
	threads.h
#	Fl_Slot.H

COMMON_CPPFILES = \
	agviewer.cpp \
	icc_cgats_filter.cpp \
	icc_draw.cpp \
	icc_examin.cpp \
	icc_examin_farbraum.cpp \
	icc_examin_io.cpp \
	icc_examin_tagwahl.cpp \
	icc_fenster.cpp \
	icc_formeln.cpp \
	icc_gamut.cpp \
	icc_gl.cpp \
	icc_helfer.cpp \
	icc_info.cpp \
	icc_kette.cpp \
	icc_main.cpp \
	icc_measurement.cpp \
	icc_modell_beobachter.cpp \
	icc_oyranos.cpp \
	icc_oyranos_extern.cpp \
	icc_profile.cpp \
	icc_profile_header.cpp \
	icc_profile_tags.cpp \
	icc_schnell.cpp \
	icc_speicher.cpp \
	icc_ueber.cpp \
	icc_utils.cpp \
	icc_vrml.cpp \
	icc_vrml_parser.cpp \
	threads.cpp

LINGUAS := \
	$(shell ls po/*.po 2> /dev/null | sed -n 's/\.po//p' | sed -n 's%po\/%%p')
LING := \
	$(shell ls po/*.po 2> /dev/null)

COMMON_CFILES = 
CFILES = \
	$(COMMON_CFILES)
CPPFILES = \
	$(COMMON_CPPFILES) \
	$(TOOLKIT_FILES) \
	$(X_CPP) \
	$(OSX_CPP)
CXXFILES = \
	icc_betrachter.cxx \
	fl_oyranos.cxx
TEST_CPP = \
	dE2000_test.cpp
TEST_H = \
	ciede2000testdata.h

ALL_SOURCEFILES = \
	$(COMMON_CFILES) \
	$(COMMON_CPPFILES) \
	$(OSX_CPPFILES) \
	$(X_CPPFILES) \
	$(FLTK_CPPFILES) \
	$(I18N_CXXFILES) \
	$(CXXFILES) \
	$(TEST_CPP)

ALL_HEADERFILES = \
	$(CPP_HEADERS) \
	$(I18N_HEADERS) \
	$(TEST_H)

DOKU = \
	TODO \
	README \
	ChangeLog \
	COPYING \
	BUGS \
	AUTHORS \
	INSTALL \
	icc_examin.desktop \
	icc_examin.png \
	icc_examin.xpm \
	icc_examin.spec.in \
	icc.xml
FLUID = \
	icc_betrachter.fl \
	fl_oyranos.fl
FONT = FreeSans.ttf

SOURCES = $(ALL_SOURCEFILES) $(ALL_HEADERFILES)
OBJECTS = $(CPPFILES:.cpp=.o) $(CXXFILES:.cxx=.o)
I18N_OBJECTS =  $(I18N_CXXFILES:.cxx=.o)
LINGUAS_SRC = $(LING:.po=.gmo)

POT_FILE = po/$(TARGET).pot

APPL_FILES = \
	ICC\ Examin.app/ \
	ICC\ Examin.sh \
	Info.plist

ALL_FILES =	$(SOURCES) \
	makefile \
	configure \
	$(DOKU) \
	$(FONT) \
	$(APPL_FILES) \
	$(FLUID)

timedir = .
mtime   := $(shell find $(timedir) -prune -printf %Ty%Tm%Td.%TT | sed s/://g)

.SILENT:

all:	dynamic
	
BASE = config mkdepend 
	
release:	icc_alles.o
	echo Verknuepfen $@...
	$(CXX) $(OPTS) -o $(BINTARGET) \
	icc_alles.o \
	$(LDLIBS)
	$(REZ)
	$(RM) icc_alles.o

dynamic:	$(BINTARGET)

$(BINTARGET):	$(BASE) $(OBJECTS) $(LINGUAS_SRC) $(I18N_OBJECTS) $(LIBNAME) $(LIBFL_I18N)
	echo Verknuepfen $@ ... $(OBJECTS)
	$(CXX) $(OPTS) -o $(BINTARGET) \
	$(OBJECTS) \
	$(LDLIBS) $(LINK_LIB_PATH) $(LINK_SRC_PATH)
	$(REZ)

$(LIBSONAMEFULL):	$(I18N_OBJECTS)
	echo Verknuepfen $@ ...
	$(CXX) $(OPTS) $(LIBLINK_FLAGS) $(LINK_NAME) -o $(LIBSONAMEFULL) \
	$(I18N_OBJECTS)  icc_utils.o icc_helfer.o icc_speicher.o
	$(REZ)
	$(RM)  $(LIBSONAME)
	$(LINK) $(LIBSONAMEFULL) $(LIBSONAME)
	$(RM)  $(LIBSO)
	$(LINK) $(LIBSONAMEFULL) $(LIBSO)

$(LIBNAME):	$(I18N_OBJECTS)
	echo Verknuepfen $@ ...
	$(COLLECT) $(LIBNAME) $(I18N_OBJECTS)
	$(RANLIB) $(LIBNAME)

$(LIBFL_I18N):
	(cd fl_i18n/; make)

static:	$(BINTARGET)
	echo Verknuepfen $@ ...
	$(CXX) $(OPTS) -o $(BINTARGET) $(OBJECTS) \
	-L./ \
	`test -f /opt/local/lib/libfltk_images.a && echo /opt/local/lib/libfltk_images.a \
    /opt/local/lib/libpng.a /opt/local/lib/libjpeg.a -lz || \
	fltk-config --use-gl --use-images --ldstaticflags` \
	`fltk-config --use-gl --ldstaticflags` \
	-L/opt/local/lib \
	`oyranos-config --ld_x_staticflags` -L/$(libdir) \
	-lftgl \
	`test -f /usr/X11R6/lib/libfreetype.a \
	  && echo /usr/X11R6/lib/libfreetype.a || (test -f /usr/lib/libfreetype.a \
	    && echo /usr/lib/libfreetype.a || echo -lfreetype)` \
	$(I18N_LIBSTAT) $(X11_LIBS) \
	$(DBG_LIBS) \
	`test -f /opt/kai-uwe/lib/liblcms.a && echo /opt/kai-uwe/lib/liblcms.a || \
	  (test -f /usr/lib/liblcms.a && echo /usr/lib/liblcms.a || \
	  (test -f /opt/local/lib/liblcms.a && echo /opt/local/lib/liblcms.a || \
        (test -f /usr/local/lib/liblcms.a && echo /usr/local/lib/liblcms.a || \
	      (pkg-config --libs lcms))))` $(LIBFL_I18N) #/usr/lib/libelektra.a # Hack for static lcms
	$(REZ)

strip: $(BINTARGET)
	echo strip $(BINTARGET)
	strip $(BINTARGET)

static_static:	$(OBJECTS)
	$(CXX) $(OPTS) -o $(BINTARGET) \
	$(OBJECTS) \
	$(LDLIBS) -static -ljpeg -lpng -lX11 -lpthread -lz -ldl \
	-lfreetype -lfontconfig -lXrender -lGLU -lXext -lexpat \
	-L/opt/local/lib liblcms.a
	$(REZ)

test:	icc_formeln.o icc_utils.o
	$(CXX) $(OPTS) $(INCL) -o dE2000_test.o -c dE2000_test.cpp
	$(CXX) $(OPTS) -o dE2000_test dE2000_test.o icc_formeln.o icc_utils.o \
	-L$(libdir) -llcms
	$(REZ)

$(LINGUAS_SRC):	$(POT_FILE)
	echo Linguas ... $(@:.gmo=)
	for ling in $(@:.gmo=); do \
      lingua=`echo $${ling} | sed s%po\/%%g`; \
	  echo "update $${ling}.gmo ..."; \
	  test -f $${ling}.po \
        && ($(MSGFMT) -o $${ling}.gmo $${ling}.po; \
            if [ ! -d $${ling} ]; then \
              mkdir $${ling}; \
            fi; \
            if [ ! -d ./$${ling}/LC_MESSAGES ]; then \
              mkdir -v $${ling}/LC_MESSAGES; \
            fi; \
            test -L $${ling}/LC_MESSAGES/$(TARGET).mo \
              && $(RM) $${ling}/LC_MESSAGES/$(TARGET).mo \
              || echo $${ling}/LC_MESSAGES/$(TARGET).mo nicht gefunden; \
            ln -s ../../$${lingua}.gmo $$ling/LC_MESSAGES/$(TARGET).mo;) \
        || (echo $${ling}.po is not yet ready ... skipping) \
	done;

$(POT_FILE):	$(ALL_SOURCEFILES)
	test `which xgettext` && xgettext --force-po $(XGETTEXT_OPTIONS) \
	-d $(TARGET) \
	-o $(POT_FILE) \
	$(SOURCES) || echo -e "\c"
	for ling in $(LINGUAS); do \
            test `which $(MSGMERGE)` && \
                 $(MSGMERGE) $(MSGMERG_OPTS) po/$${ling}.po \
                 $(POT_FILE) || \
	        echo -e "\c"; \
	done;
	echo $(POT_FILE) fertig


clean:	unbundle unpkg
	echo mache sauber $@ ...
	-$(RM) mkdepend config config.h
	-$(RM) $(OBJECTS) $(I18N_OBJECTS) $(BINTARGET) \
	$(LIBNAME) $(LIBSO) $(LIBSONAME) $(LIBSONAMEFULL)
	-for ling in $(LINGUAS); do \
	  test -f po/$${ling}.gmo \
        && $(RM) po/$${ling}.gmo; \
	done;
	echo ... $@ fertig

config:
	./configure

mkdepend:	config $(ALL_HEADERFILES)
	echo "schaue nach Abhaengikeiten ..."
	echo "MAKEDEPEND_ISUP = 1" > mkdepend
	echo "#nicht editieren/dont edit - automatisch generiert" >> mkdepend
	-$(MAKEDEPEND) -I./ $(INCL_DEP) >> mkdepend


# The extension to use for executables...
EXEEXT		= 

# Build commands and filename extensions...
.SUFFIXES:	.0 .1 .3 .c .cxx .h .fl .man .o .z $(EXEEXT)

.c.o:
	echo Compiling $< ...
	$(CC) $(CFLAGS) -c $<

.cxx.o:
	echo Compiling $< ...
	$(CXX) $(CXXFLAGS) -c $<

.cpp.o:
	echo Compiling $< ...
	$(CXX) $(CXXFLAGS) -c $<

.po:
	echo Generating $@ ...
	msgfmt $<

tgz:
	make DESTDIR=Entwickeln copy_files
	tar cf - Entwickeln/ \
	| gzip > $(TARGET)_$(mtime).tgz
	test -d Entwickeln && \
	test `pwd` != `(cd Entwickeln; pwd)` && \
	$(RM) -R Entwickeln
	test -d ../Archiv && mv -v $(TARGET)_*.tgz ../Archiv

targz:
	make DESTDIR=$(TARGET)-$(VERSION) copy_files
	tar cf - $(TARGET)-$(VERSION)/ \
	| gzip > $(TARGET)_$(mtime).tgz
	test -d $(TARGET)-$(VERSION) && \
	test `pwd` != `(cd $(TARGET)-$(VERSION); pwd)` && \
	$(RM) -R $(TARGET)-$(VERSION) 
	test -d ../Archiv && mv -v $(TARGET)_*.tgz ../Archiv || echo no copy

copy_files:
	mkdir $(DESTDIR)
	(cd fl_i18n; \
	 make DESTDIR=../$(DESTDIR)/fl_i18n copy_files; \
	 cd ..)
	$(COPY) -R \
	$(ALL_FILES) \
	$(DESTDIR)
	mkdir $(DESTDIR)/po
	$(COPY) $(POT_FILE) $(DESTDIR)/po
	for ling in $(LINGUAS); do \
	  test -f po/$${ling}.po \
        && $(COPY) po/$${ling}.po $(DESTDIR)/po/; \
	done;


dist: $(BASE) targz
	test -f && $(COPY) ../Archiv/$(TARGET)_$(mtime).tgz $(TARGET)-$(VERSION).tar.gz || $(COPY) $(TARGET)_$(mtime).tgz $(TARGET)-$(VERSION).tar.gz

$(TARGET).spec:
	./configure

rpm:	$(TARGET).spec dist
	mkdir -p rpmdir/BUILD \
	rpmdir/SPECS \
	rpmdir/SOURCES \
	rpmdir/SRPMS \
	rpmdir/RPMS/$(RPMARCH)
	cp -f $(TARGET)-$(VERSION).tar.gz rpmdir/SOURCES
	rpmbuild --clean -ba $(srcdir)/$(TARGET).spec --define "_topdir $$PWD/rpmdir"
	@echo "============================================================"
	@echo "Beendet - die Packete befinden sich in rpmdir/RPMS and rpmdir/SRPMS!"

install:
	echo Installation ...
	mkdir -p $(DESTDIR)$(bindir)
	$(INSTALL) -m 755 $(BINTARGET) $(DESTDIR)$(bindir)
	strip $(DESTDIR)$(bindir)/$(BINTARGET)
	fltk-config --post $(DESTDIR)$(bindir)/$(BINTARGET)
	mkdir -p $(DESTDIR)$(datadir)/fonts/
	$(INSTALL) -m 644 $(FONT) $(DESTDIR)$(datadir)/fonts/$(FONT)
	mkdir -p $(DESTDIR)$(datadir)/applications/
	$(INSTALL) -m 644 icc_examin.desktop $(DESTDIR)$(datadir)/applications/icc_examin.desktop
	mkdir -p $(DESTDIR)$(datadir)/mime/packages/
	$(INSTALL) -m 644 icc.xml $(DESTDIR)$(datadir)/mime/packages/icc.xml
	mkdir -p $(DESTDIR)$(datadir)/pixmaps/
	$(INSTALL) -m 644 icc_examin.png $(DESTDIR)$(datadir)/pixmaps/icc_examin.png
	echo  Linguas ...
	for ling in $(LINGUAS); do \
	  echo "installiere po/$${ling}.gmo ..."; \
      mkdir -p $(DESTDIR)$(datadir)/locale/$${ling}/LC_MESSAGES; \
      test -f po/$${ling}.gmo \
		&& (mkdir -p $(DESTDIR)$(datadir)/locale/$${ling}/LC_MESSAGES; \
            $(INSTALL) -m 644 po/$${ling}.gmo $(DESTDIR)$(datadir)/locale/$${ling}/LC_MESSAGES/$(TARGET).mo ) \
		|| (echo $${ling}.gmo is not yet ready ... skipping); \
	done;
	echo ... Installation beendet

uninstall:
	echo deinstalliere ...
	$(RM) $(DESTDIR)$(bindir)/$(BINTARGET)
	$(RM) $(DESTDIR)$(datadir)/applications/icc_examin.desktop
	$(RM) $(DESTDIR)$(datadir)/mime/packages/icc.xml
	$(RM) $(DESTDIR)$(datadir)/pixmaps/icc_examin.png
	$(RM) $(DESTDIR)$(datadir)/fonts/$(FONT)
	for ling in $(LINGUAS); do \
	  $(RM) $(DESTDIR)$(datadir)/locale/$${ling}/LC_MESSAGES/$(TARGET).mo; \
	done;
	echo ... $@ fertig

bundle:
	echo bündeln ...
	test -d ICC\ Examin.app/Contents/MacOS/ || mkdir -p ICC\ Examin.app/ ICC\ Examin.app/Contents/ ICC\ Examin.app/Contents/MacOS/ ICC\ Examin.app/Contents/Resources/  ICC\ Examin.app/Contents/Resources/bin/ ICC\ Examin.app/Contents/Resources/lib/ ICC\ Examin.app/Contents/Resources/share/
	$(INSTALL) -m 644 Info.plist ICC\ Examin.app/Contents/Info.plist
	$(INSTALL) -m 755 ICC\ Examin.sh ICC\ Examin.app/Contents/MacOS/ICC\ Examin
	$(INSTALL) -m 755 $(BINTARGET) ICC\ Examin.app/Contents/MacOS/
	strip ICC\ Examin.app/Contents/MacOS/$(BINTARGET)
	echo  Bibliotheken ...
	for bib in jpeg png lcms intl iconv fltk freetype; do \
	  echo "bündele lib$${bib}$(LIBEXT) ..."; \
      test -f /opt/local/lib/lib$${bib}$(LIBEXT) \
		&& ($(COPY) -R /opt/local/lib/lib$${bib}*$(LIBEXT) ICC\ Examin.app/Contents/Resources/lib/ ) \
		|| ( test -f /usr/X11R6/lib/lib$${bib}$(LIBEXT) \
		    && ($(COPY) -R /usr/X11R6/lib/lib$${bib}*$(LIBEXT) ICC\ Examin.app/Contents/Resources/lib/ ) \
	        || (echo lib$${bib}$(LIBEXT) not found ... skipping)); \
	done;
	test -f ~/bin/iccgamut \
	  && $(INSTALL) -m 755 ~/bin/iccgamut ICC\ Examin.app/Contents/Resources/bin/ \
	  || test -f iccgamut && $(INSTALL) -m 755 iccgamut ICC\ Examin.app/Contents/Resources/bin/ \
	    || echo iccgamut nicht gefunden
	$(INSTALL) -m 644 $(FONT) ICC\ Examin.app/Contents/Resources/share/
	echo  Linguas ...
	for ling in $(LINGUAS); do \
	  echo "bündele po/$${ling}.gmo ..."; \
      test -f po/$${ling}.gmo \
		&& (mkdir -p ICC\ Examin.app/Contents/Resources/locale/$${ling}/LC_MESSAGES; \
            $(INSTALL) -m 644 po/$${ling}.gmo ICC\ Examin.app/Contents/Resources/locale/$${ling}/LC_MESSAGES/$(TARGET).mo ) \
		|| (echo $${ling}.gmo is not yet ready ... skipping); \
	done;
	echo ... bündeln beendet

unbundle:
	echo mache sauber $@ in $(DESTDIR) ...
	$(RM) $(DESTDIR)ICC\ Examin.app/Contents/MacOS/*
	$(RM) $(DESTDIR)ICC\ Examin.app/Contents/Resources/share/$(FONT)
	$(RM) $(DESTDIR)ICC\ Examin.app/Contents/Resources/bin/*
	$(RM) $(DESTDIR)ICC\ Examin.app/Contents/Resources/lib/*
	$(RM) -R $(DESTDIR)ICC\ Examin.app/Contents/Resources/locale
	echo ... $@ fertig

.PHONY: install pkg bundle uninstall unbundle unpkg

pkg:	bundle
	echo Vorbereiten des osX Packetes ...
	$(RM) -r installation/*
	make DESTDIR=`pwd`/installation install
	mkdir -p installation/Applications/
	$(COPY) -R ICC\ Examin.app installation/Applications/
	make DESTDIR=`pwd`/installation/Applications/ unbundle
	test -f ~/bin/iccgamut \
	  && $(INSTALL) -m 755 ~/bin/iccgamut installation$(bindir) \
	  || test -f iccgamut && $(INSTALL) -m 755 iccgamut installation$(bindir) \
	  || test -f /opt/local/bin/iccgamut && $(INSTALL) -m 755 /opt/local/bin/iccgamut installation$(bindir) \
	    || echo iccgamut nicht gefunden
	$(LINK) $(bindir)/$(BINTARGET) installation/Applications/ICC\ Examin.app/Contents/MacOS/ICC\ Examin
	open /Developer/Applications/Utilities/PackageMaker.app ICC\ Examin.pmsp
	echo sudo: chown -R root:admin `pwd`/installation  -- bitte nicht vergessen
	echo ... Packet vorbereitet

unpkg:
	echo mache sauber $@ ...
	$(RM) -R installation/
	echo ... $@ fertig

-include mkdepend

