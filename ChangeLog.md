# Version 0.56

### Dmitry Marakasov (1):
* Don't create not used directory

### Kai-Uwe Behrmann (116):
* [build]: fix rpm docs
* [conf]: use the canonical local XDG config dir in build script
* [docu]: markdown the README
* [core]: improve config GUI detection
* [cine]: state data signdness
* [core]: add explicit type conversion for window macro
* [core]: use public Oyranos function declarations
* [conf]: add ftgl + freetype to build script for OS X
* [conf]: support gettext in build script
* [conf]: check iconv for OS X
* [conf]: check libintl for OS X
* [conf]: change into app-bundled ICC profile directory
* [core]: reenable DnD on OS X
* [conf]: update build script to improve libxml in win
* [conf]: plain copy of Oyranos Framework
* [conf]: use MACOSX_BUNDLE_EXECUTABLE_NAME again
* [conf]: add Info.plist.in to fix HiDPI on OS X
* [conf]: update packages in build script for OS X 10.10
* [conf]: switch to HOME dir builds by default in script
* [conf]: link only the libs which are needed
* [core]: replace CMGetGammaByAVID by CGDisplayGammaTableCapacity
* [core]: comment out appearently unused CS code
* [core]: fix if else abiguity
* [core]: skip X11 skaling code for __APPLE__
* [core]: fix header guard
* [core]: fix if else ambiguity warning from clang
* [core]: fix warning for possibly undefined return value
* [core]: fix missing define for non gettext builds
* [core]: detect pam
* [core]: use braces around oyXXX_Copy() macro
* [core]: update to Oyranos git API change
* [core]: show all languages for mluc tags
* [core]: use possibly translated profile name in top dislay
* [core]: add Ctrl+/- shortcuts for scaling
* [core]: use XResource:Xft.dpi in getSysScale()
* [core]: update devel month and translate
* [core]: fix help window label size scaling
* [core]: add font size+ and - buttons to main's view menu
* [build]: fix source code links to Synnefo, cinepaint-ng
* [build]: include all i18n related files
* [core]: add comment to DPI workaround
* [core]: compensate for high DPI displaying
* [build]: add xcalib to build script
* [conf]: debian/control wants fltk version decission
* [docu]: fix logo png
* [build]: rename icon and desktop files
* [conf]: fix cmake target names on linux
* [core]: scan for possible synnefo app names
* [build]: install ICCExamin.app
* [build]: add some OS X bundle files
* [build]: remove hard coded -loyranos link flag
* [core]: adapt to oyProfile_Install Oyranos API changes
* [core]: use new oyICCCmmDescription() API improve CMM display
* [conf]: set CMAKE_BUILD_TYPE=Debug for most projects
* [conf]: set correct iccexamin path for testing after build script
* [core]: omit X11 code on OS X
* [conf]: set correct OS X arch in build script
* [build]: correctly select x11 and cs source files
* [core]: fix macro name
* [bsp]: boolean type conflicts on CoreFoundation
* [conf]: fix FTGL not found
* [conf]: fix FindFTGL.cmake not found
* [conf]: omit make clean for cmake in build script
* [conf]: fix -libdir switch in build script
* [conf]: switch of dependency tracking for fltk in build script
* [conf]: enable fltk shared on OS X in build script
* [conf]: create universal binaries on OS X
* [conf]: add libelektra and yajl from git to build script
* [build]: build and install CinePaint plugin
* [conf]: add FindCinepaint.cmake
* [cine]: add X11 headers for HAVE_X11
* [conf]: fix cinepaint version detection
* [core]: update to Oyranos API change
* [update]: update makefile of fl_i18n
* [core]: use TEXTDOMAIN macro to find translations
* [core]: fix config log formatting
* [i18n]: update deutsch
* [conf]: fix recent syntax error in configure
* [conf]: add uninstall target to cmake
* [conf]: sync gmo file names with target name
* [core]: fix xrandr usage inside gama view
* [conf]: add cmake find modules
* [conf]: add cmake template headers
* [conf]: use oyranos logo for desktop file
* [conf]: detect X11 and fix target name
* [conf]: add minimal cmake support
* [core]: use _WIN32 macro
* [conf]: update date
* [core]: fix argument type
* [conf]: update local example build script
* [conf]: update build script for MSysGit on Windows
* [core]: use the configure time detect pthread
* [build]: fix lib link order for MSysGit builds
* [core]: define nissed mutex macro
* [core]: skip non posix errors on win32
* [core]: fix compile error Fl_Thread set to 0 II
* [core]: fix wrong argument to oyProfileTag_Set()
* [core]: fix compile error with Fl_Thread set to 0
* [i18n]: skip nl_langinfo()
* [conf]: fix some warnings
* [bsp]: boolean typedef conflicts on MinGW
* [core]: adapt to Oyranos key name macro changes
* [core]: sync FLTK widgets with Oyranos image-display
* [core]: adapt to Oy-API 0.9.6 changes
* [core]: rearrange basic file name handling
* [core]: dont touch to observed profiles
* [core]: use oyProfile_FromName() and support ID and wildcard loading
* [build]: rename package to icc-examin
* [build]: update debian packaging
* [core]: fix early colour binding under XCM
* [core]: update to Oy-0.9.6 changes
* [build]: make install depends on all
* [core]: support -2 and -4 cml options
* [conf]: fix clang build
* [conf]: bump version
* [conf]: update to 0.55 version

# Version 0.55

### Boris Samorodov (1):
* [conf]: adapt inline sed usage for FreeBSD

### Dominique Leuenberger (1):
* [conf]: Fix build on Factory - where 'pkg-config --cflags xcm' is empty

### Kai-Uwe Behrmann (22):
* [build]: sync spec file with OBS
* [build]: ignore packages in build script on Darwin
* [core]: update to Oyranos 0.9.5 API change
* [core]: do not rely on display name for fallback
* [core]: update to Oyranos 0.9.5 API changes
* [core]: update to oyranos FLTK widget
* [core]: fix DnD
* [core]: fix more compiler warnings
* [core]: fix compiler warning about C++11 syntax
* [core]: set non profile description
* [i18n]: update deutsch
* [core]: check for proper X11/XCM support
* [core]: fix warning after profile extraction from image
* [core]: fix for profiles without desc tag
* [core]: fix lagging UI on file open
* [core]: fix compiling
* [core]: fix Harald's crash on XFCE
* [core]: show missed monitor under vcgt window
* [core]: adapt xrandr to xinerama monitor order
* [core]: use available XRandR for monitor gamma
* [core]: use Oyranos device API in moniInfo()
* [conf]: detect and link XRandR

# Version 0.54

### Kai-Uwe Behrmann (8):
* [core]: fix warnings with Xcm
* [build]: update oyranos cmake in build script
* [build]: support cmake in build script
* [docu]: update and format copying files
* [conf]: bump version
* [core]: fix memory alloc/free pair
* [build]: update to icc-profiles-openicc-1.3.1
* [build]: add XDG dir to build script

# Version 0.53

### Kai-Uwe Behrmann (9):
* [build]: debian/ubuntu packages
* [build]: remove recommendation to oy-monitor-nvidia
* [core]: use editing space only with colour server
* [core]: check wide gamut editing suitability
* [core]: rename FLTK functions to internal names
* [core]: skip rewriting profile memory
* [build]: install from out-of-source configuration
* [conf]: bump version
* [conf]: fix compiling on Fedora 17

# Version 0.52

### Kai-Uwe Behrmann (48):
* [docu]: update ChangeLog
* [core]: reduce Oyranos alpha includes
* [core]: fix for negative signed chromaticities
* [core]: set www links
* [conf]: actualise date
* [core]: update to Oyranos API changes
* [core]: revert include in Oy_Fl_Group.h
* [build]: add libltdl to bundle target
* [core]: update osX bundle version
* [build]: update after removal of build dirs
* [build]: require Oyranos >= 0.9
* [core]: compile with Oyranos gsoc2011
* [core]: update Oy_Fl_ widgets
* [build]: add libltdl to bundle target
* [core]: update osX bundle version
* [build]: update after removal of build dirs
* [core]: detect lraw supported cameraRAW prefixes
* [core]: fix mAB/mBA table access
* [core]: update to oyImage_WritePPM API change
* [core]: add debug output for system commands
* [core]: adapt oyranos-profile-graph options
* [core]: add ICCEXAMIN_SKIP_GDB environment variable
* [core]: simplify the oyranos-profile-graph launch
* [build]: require Oyranos >= 0.5
* [core]: use ICC grapher for preview icon
* [core]: show internal profile name in main window
* [i18n]: update deutsch
* [core]: update to Oyranos v0.5 API changes
* [core]: add file formats to desktop file
* [core]: remove previous preview image
* [core]: add pfm to supported formats
* [i18n]: update deutsch
* [core]: use own instead of default image handler
* [core]: add iccImageCheck()
* [core]: partitial revert #4d4529d7
* [core]: support image profiles
* [core]: small code translation
* [core]: add -g option
* [core]: clean unused function
* [core]: use macro for config name
* [docu]: update web link
* [build]: remove unneeded text line in script
* [build]: fix elektra skipping in script
* [build]: move xcm behind Oyranos in script
* [build]: update skip list in build script
* [build]: fix skipping elektra in script
* [build]: require XCM supporting libraries in spec file
* [conf]: bump version

### Milan Knizek (2):
* [i18n]: update Czech for fl_i18n
* [i18n]: update Czech

# Version 0.51

### Kai-Uwe Behrmann (57):
* [core]: save GL preferences
* [core]: fix http file opening
* [i18n]: update deutsch
* [core]: translate missing about text
* [i18n]: scan version header for months
* [core]: adjust URLs
* [core]: support wget on linux
* [conf]: actualise date
* [conf]: adapt to openSUSE spec changes
* [build]: package missed files
* [core]: translate icc_gl.cpp
* [core]: translate icc_gl.h
* [core]: rename agviewer variables
* [core]: remove argyll file
* [core]: share Oy_Fl_Window_Base fix for osX
* [conf]: change elektra download url
* [conf]: adapt prefix more easily in custom build script
* [conf]: set build script unix executeable flag
* [iemn]: add observer plugin starter
* [conf]: allow to skip profile install in script
* [conf]: assist in elektra build on win32
* [conf]: disable X11 on win32 in build script
* [core]: remove unused variables
* [core]: remove monitor profile tracing
* [core]: set in 3D all colours by profile
* [conf]: skip xcolor/doc removal
* [conf]: add libcmpx to build script
* [build]: add verbose, skip and kde_prefix variables
* [conf]: fix typo in build script
* [build]: easier install directory in script
* [core]: skip zero size colour conversion
* [conf]: fix variable name in script
* [conf]: check for installation of pkg-config files
* [core]: add display helper file
* [core]: add Oy_Fl_Group class
* [core]: use the Oy_Fl_Group widget from Oyranos
* [build]: update script to FLTK-1.3.0
* [core]: show missed measurement message again
* [core]: disable Xcm region if the widget is hidden
* [build]: generate FLTK sources in source directory
* [core]: colour correct statusFarbe by a colour server
* [core]: let a colour server colour correct OpenGL
* [core]: wrap Xcm calls and add getEditingProfile()
* [conf]: require libXcm
* [core]: update to x_color_region_target in Oy-0.3.3
* [build]: add build script for a local installation
* [build]: fix small bug in changing working directory
* [build]: update build script to elektra-0.7.1
* [docu]: Confirm licensing of icc_examin 0.50
* [build]: update script to new KolorManager git
* [core]: fix regression in file observation
* [core]: set lcms2 adaption state to lcms 1 behaviour
* [build]: sync with openSUSE RPM spec
* [core]: build again without FTGL
* [build]: script skips unchanged git packages
* [build]: update script to lcms2 and check sane version
* [conf]: bump version

### Nicolas Chauvet (1):
* [core]: Fix compilation with gcc 4.7

# Version 0.50

### Boris Manojlovic (1):
* [build]: spec fixes for Factory

### Kai-Uwe Behrmann (22):
* [build]: fix for spec file changes
* [core]: fix Oyranos profile lookup
* [core]: fix file name splitting after Oyranos lookup
* [core]: use Oyranos for profile look up
* [docu]: update license to new adress
* [build]: update dowload & build script for new profile packages
* [core]: fix many compiler warnings
* [docu]: update F1 help text
* [conf]: set to version 0.5
* [docu]: add man page
* [core]: use actual ICC options for external colour
* [core]: expect 0...1 CIE*XYZ range from lcm2
* [conf]: add Synnefo to build script
* [conf]: switch to lcms2 for build script
* [core]: fix typo in previous #8fbb8c78
* [core]: look for COLOR_SELECTION events
* [core]: update event handler function to fluid UI
* [core]: rename general event handler function
* [build]: bundle profiles correctly
* [conf]: bump Oyranos requirement to 0.3.2
* [core]: support stdin with -i argument
* [conf]: bump version

### Togan Muftuoglu (1):
* [build]: spec cleanup + build with %optflags

# Version 0.49

### Kai-Uwe Behrmann (42):
* [core]: accept nLUT channels level
* [core]: set initial nLUT level at middle
* [core]: fix 4+n Lut navigation
* [core]: add Gl channels setter
* [build]: use RPM Recommends only in openSUSE
* [build]: script update to new profile packages
* [build]: script disable sane on osX
* [build]: add libxml and libpng to build script
* [build]: install new v0.3.0 Oyranos libraries
* [core]: use correct order for mAB matrix elements
* [core]: show mAB nLUTs in 3D viewer
* [docu]: GL_Ansicht::hineinTabelle
* [core]: getTagChannelNames() more tag types
* [core]: remove unneeded updating
* [core]: enable mBA
* [core]: redraw table choice
* [core]: prepend channel names before curve texts
* [core]: show complex tags icCurveType and matrix
* [core]: rename Mft to Table
* [core]: add ICCprofile::getPCS()
* [i18n]: update deutsch
* [core]: show parametric tag as curve
* [core]: adapt diagram raster brightness
* [core]: more details on curve in mAB
* [cine]: reduce debug output
* [cine]: translate ICC to Oyranos proof intent
* [core]: use profile intent for proof gamut
* [cine]: use generic values ranges
* [cine]: allocate buffers before context
* [cine]: fix options assignment
* [core]: fix string compile warnings
* [build]: script checks for cinepaint Makefile
* [build]: use gettext with new generated code
* [core]: include missed i18n macro
* [core]: detect oyStructList_s data from tags
* [core]: access tag data from Oyranos
* [core]: punkte renaming
* [core]: use waehleMft() for MftChoice again
* [core]: mAB support I
* [core]: allow explicite MftChoice settings
* [core]: add idendity curve
* [conf]: bump version

### Togan Muftuoglu (1):
* [build]: move nvidia to Recommends

# Version 0.48

### Kai-Uwe Behrmann (124):
* [core]: fix application search function
* [core]: ignore missed device channels in ncl2
* [core]: use profile headers channel count
* [core]: add channels count fallback
* [core]: prevent numerical exception
* [core]: use pixel access objects for optimisation
* [core]: fix declaration
* [core]: add Synnefo Oyranos front send
* [cine]: omit switch to Oyranos
* [core]: refresh after options change
* [core]: add absolute intent for initial mesh
* [core]: rename variables
* [core]: remove progress indicator for Oyranos
* [core]: convert colours only as needed
* [core]: initialise variables
* [core]: initialise variable
* [core]: correct XYZ value range on data set
* [core]: default absolute intent in core options
* [core]: remove unneeded conversion functions
* [core]: scale XYZ display to new value range
* [core]: use ui options again
* [core]: omit measurement scaling
* [core]: add full rendering options name
* [core]: fix single buffer pixel count
* [core]: set drawing to double buffers for lcms
* [core]: convert complete data
* [core]: fix access to multiple pixels
* [core]: omit unused code
* [core]: cache one pixel transforms
* [core]: fix temorary buffer usage
* [core]: adapt to Oyranos new alpha API change
* [core]: set correct buffers
* [core]: omit unused buffer
* [conf]: update
* [core]: catch unexpected display names
* [core]: fix Oyranos option keys
* [core]: fix wrong context assignment
* [core]: clean up intermediate debugging code
* [core]: fix drawing with Oyranos colours
* [core]: simplify holeRGBRampen()
* [core]: compile fixes
* [core]: use Oyranos' ICC definitions
* [core]: convert from lcms to Oyranos 1
* [core]: fontconfig searches DejaVuSans.ttf
* [conf]: sync with Oyranos script
* [build]: fix sane build
* [build]: switch to new profile packages
* [build]:  reduce downloads and rebuilding
* [core]: zoom with mouse wheel
* [core]: move cut plane by shift + mouse wheel
* [core]: mouse plane move
* [core]: show mouse move cursor and zoom
* [build]: compress package with bzip2
* [core]: update to Oyranos API change
* [conf]: sync with Oyranos
* [build]: script disable lcms in LibRaw
* [build]: script LibRaw, kolor-manager, CinePaint
* [build]: add build script to git
* [conf]: desktop Categories=Graphics;Viewer;
* [conf]: use -fPIC only for 64bit builds
* [conf]: desktop Applications;Graphics;Publishing
* [conf]: add configure --fltk-config-flags option
* [conf]: move link flag to proper variable
* [conf]: set paths to freetype on osX
* [core]: support pser user temp file name
* [core]: add update option to intent()
* [core]: fix recent wget fallback #dad898c4
* [build]: update RPM spec
* remove elektra requirement
* [core]: beautify code
* [core]: symetrical activate net
* [core]: ICCnetz::aktiv variable is private
* [core]: allow fallback for non wget
* [core]: poll for changed files
* [core]: reenable file observer thread
* [core]: comment net-color spec profile inclusion
* [conf]: reenable threading in CinePaint plugin
* [core]: careful update
* [core]: not always update in ICCexamin::intent()
* [core]: reenable ICCexamin::lade()
* [core]: revert #d7f3e5f52
* [core]: fix push_back()
* [core]: fix compile warning
* [core]: add intermediate title
* [core]: initialise titel string
* [core]: one more request
* [conf]: update devel date
* [docu]: add Fedora requirements
* [core]: load harmonic patch sets or skip
* [core]: omit static variable
* [core]: enable loading again
* [core]: remove many lcms references
* [core]: remove threads
* [core]: omit line break for singe line texts
* [core]: send arch and distribution name during download
* [conf]: store arch and distribution names
* [core]: support download without TMP variable
* [core]: fix DISPLAY string crash
* [core]: dont crash with missed default profiles
* [core]: use explicite ColorSync on osX
* [build]: install OpenICC profiles
* [i18n]: adapt app bundle i18n path
* [conf]: detect osX version not Darwin's
* [core]: just GL widget
* [core]: hide mft_gl widget on osX
* [conf]: set release version and date
* [build]: on osX, bundle libXcm
* [core]: better format the dict tag type text
* [i18n]: update deutsch
* [cine]: load net-color monitor profile
* [i18n]: update deutsch
* [docu]: ICC profile <-> calibration data window
* [build]: add suse_update_desktop_file RPM macro
* [core]: turn ndin into drawing
* [core]: better format ndin and mmod text output
* [core]: draw first three XYZ points as triangle
* [core]: detect new tag types
* [core]: avoid cieShoe drawing conflicts
* [core]: disable incremental updates
* [build]: elder FLTK compile fix
* [core]: fix access to NULL ptr array
* [build]: install the png icon correctly
* [conf]: bump version
* [docu]: update changelog

### Nicolas Chauvet (1):
* [conf]: apply Fedora pkg-config patch

# Version 0.47

### Kai-Uwe Behrmann (171):
* [add]: GDB output on error
* [docu] update ChangeLog file
* [add]: set OY_LOCALEDIR to point to Oyranos message files
* [and]: formatting
* [fix]: compiler warning
* [fix]: move remaining OpenGL call to Fl_GL_Window::draw()
* [fix] several compiler warnings
* [build]: trace bsp changes
* [fix]: missed Oyranos tag parsing fallback
* [and]: attach gdb only for ICCEXAMIN_DEBUG>0
* [alpha]: update to Oyranos API change
* [build] fix for Mandriva2007
* [fix]: memory error in leseTag()
* [fix]: Null Xinerama monitor name string
* [alpha]: update to new oyGetMonitorInfo()
* [build]: move configure.sh -> configure_tests.sh
* [and]: update to v0.47
* [conf]: remove configure.sh
* [alpha]: update to Oyranos API changes
* [and]: rename some internal variables
* [i18n]: update de
* [build]: translate messages to international en_GB
* [i18n]: add comments to translation files
* [and]: add compressed VRML's to file selector
* [i18n]: update german
* [alpha]: update to Oyranos' oyOptions_SetFromText()
* [fix]: init OpenGL with mapped window
* [fix]: memory crash on XRandR
* [and]: disable FLTK cli argument processing
* [fix]: tokenise uppercase in HTML output
* [add]: info tab to about window
* [and]: dateiwahl changed from pointer to a function
* [fix]: too dark status colour regression
* [and]: check every thread for running after a window was shown
* [and]: highlight patch selection with a circle
* [build]: add Flmm_Tabs files
* [and]: migrate window code to my_fl_double_window.cxx
* [fix]: move GL list creation inside draw()
* [fix]: revert ab5b42e0c mode() inside draw()
* [fix]: correct a mismatched delete []
* [and]: fix missed GL menue regression
* [build]: track bsp source changes
* [fix]: compatibility to Oyranos-0.1.9
* [build]: return zero on configure success
* [and]: add SVG icon
* [fix]: free leaked memory from thread code
* [and]: switch to oyConversion_s
* [and]: update mime and desktop infos
* [build]: remove 6ed73e31 files
* [and]: adapt bsp to x86_64
* [and]: include <oyranos_icc.h>
* [and]: undo previous commit
* [and]: replace icMaxEnumTag by (icTagSignature)0xFFFFFFFF
* [conf]: sync configure script with Oyranos
* [and]: substitute icMaxEnumData
* [conf]: sync with Oyranos
* [build]: out of source tree support
* [build]: out of source rpm target
* [conf]: fix warning about non existing fltk-test
* [and]: reduce overhead while debugging
* [fix]: log window crash during debug
* [and]: dump out CLI args in debug mode
* [fix]: BPC option in gamut window bug
* [and]: remove expensive debug output
* [fix]: thread concurrency during debugging
* [conf]: beautify src_dir variable
* [and]: omit okButton deactivation and some file checking ...
* [fix]: avoid to create a FLTK window from outside the main thread
* [and]: load profile data into a Oyranos object
* [fix]: use own file load capabilities ...
* [and]: allow external access to Oyranos profile object
* [new]: show device link source profile in the status bar
* [and]: remove oyColourConversion_s references in VRML writing
* [and]: pass options to oyNamedColour_s
* [and]: short source file names for debug messages
* [add]: isolines for table view
* [add]: mesh for table view
* [and]: place the table mesh correctly
* [and]: improve isolines in 3D table view
* [and]: always smoothed lines in 3D table view
* [alpha]: update to oyConversion_CreateBasicPixels()
* [conf]: update to Oyranos
* [core]: update to Oyranos API
* [core]: revert previous
* [core]: fix abstract proofing profile
* [core]: fix compile error
* [core]: fix compile errors
* [core]: initialise variables
* [core]: update to Oyranos API
* [docu]: add build script hints
* [core]: robuster CGATS DATA_FORMAT parsing
* [core]: add Monaco CGATS RGB scaling
* [core]: switch to coloured table mesh by default
* [core]: avoid event conflicts
* [core]: fix the multi table window regression
* [core]: fix OpenGL related bug
* [fix]: wrong memory deallocation
* [build]: give a new bundle version
* [core]: try to avoid cocoa issues
* [conf]: update date; omit library guessing
* [fix]: a non gettext build issue
* [fix]: compiler warnings
* [core]: update to newer osX ColorSync APIs
* [build]: search for moved binary
* [docu]: hint about missing BPC information
* [core]: fix compile issue
* [core]: show single display profile
* [core]: economic monitor profile update
* [fix]: message related thread issues
* [docs]: to UTF-8
* [core]: add font directory fix
* [conf]: sync with Oyranos script
* [conf]: enable OpenMP support
* [build]: desktop, mime and pixmap fixes
* [build]: create missing directory
* [build]: create missing directory
* [bundle]: adapt internal library paths
* [UI]: add background widget for hiding white
* [build]: copy Oyranos modules for osX bundle
* [build]: update mime to Oyranos
* [core]: fix compiler warning
* [core]: fix osX with missed X11 compiling
* [core]: build fix
* [core]: FL/mac.H can not be used directly
* [build]: correct copy variable
* [build]: update spec files to changed icon
* [core]: avoid use of uninitialised valgrind warn
* [build]: add missed icons again
* [docu]: document actual keys and 'psid'
* [core]: use native Oyranos profile
* [core]: sync with Oyranos
* [fix]: wrong list access
* [core]: GL dbg messages
* [fix]: cgats variants
* [build]: add dist-gzip target to make ...
* [build]: change rpm pixmap dir
* [core]: fix buffer overflow
* [conf]: add %dir cp_dir to spec file
* [core]: fix another buffer overflow
* [build]: reduce rpm generatin warnings
* [conf]: format configure output somewhat
* [conf]: correct FLAGS for CAIRO
* [fix]: security string warnings
* [conf]: sync with Oyranos
* [conf]: remove unused section
* [conf]: add glu test
* [build]: add glu flags
* [conf]: check for pthread
* [conf]: move GLU into X11 section
* [core]: remove oyranos namespace
* [conf]: sync with Oyranos configure script
* [build]: dont remove RPM buildroot
* [conf]: remove x-world/x-vrml mime
* [build]: sync RPM spec file with OSC
* [build]: fix shell call in makefile
* [build]: update to Oyranos alpha header changes
* [conf]: check FTGL version
* [core]: convert to utf8 for FTGL
* [core]: complete previous commit
* [core]: convert to wchar_t for FTGL
* [core]: fall back to codeset UTF-8
* [i18n]: osX SL seems to expect MACROMAN
* [i18n]: start help page translation
* [i18n]: more help page translations
* [i18n]: complete the help page translation
* [core]: dont exit from bsp
* [core]: fix compiler warning
* [conf]: detect cinepaint path for RPM spec
* [core]: fix format string error
* [build]: update mandriva dependencies
* [i18n]: fix format string errors

### Lars Tore Gustavsen (1):
* [docu]: Debian build hints

### kwizart (1):
* [core]: apply fedora patches


# Version 0.46

### I�aki Larra�aga Murgoitio (1):
* [po]: Basque translations

### Kai-Uwe Behrmann (86):
* show distribution
* git maintainance
* remove std::vector
* fix for GMB measurement line marking
* add more thread debugging infos
* correct typo (thanks to Guy K. Kloss for spotting)
* add copy contructors and hex displaying
* repair list class copy
* support n-dimensional mft2 table displaying
* complete for mft1
* repair ICCThreadList::copy regression
* add Flmm widgets
* add log window (Ctrl-L)
* add Flmm
* window access fix
* add fl_i18n_codeset in fl_i18n.H for checking
* fix various compiler warnings
* more osX configure verbosity
* remove oyranos_alpha reference
* half way osX bundle reordering
* bundle elektra backends
* fix elektra backend install
* clean recursive
* switch first rendering intent naming back to perceptual
* allow for non i18n builds
* remove wcstombs
* remove mluc decoding as it is covered by Oyranos
* add propriarity TGL2 to target tag names (ICC: "targ")
* rename to oyNamedColours_Get
* clean the ICC header of intermediate ncl2 profile
* avoid crash of zero length ncl2 colour lists
* remove ambiguous html table attribute
* update
* switch to verbose compiling by default
* update to Oyranos new message function type
* avoid FL:: thread dependency for message logging; use a local mutex
* obtain the correct filename for a Oyranos profile
* update
* update nl_langinfo() compatibility
* update to last Oyranos git changes
* during debugging direct log messages to the command line
* update to Oyranos oyImage_s data API
* some cleaning und updating
* more detailed date support in configure and the About window
* fix write on static memory
* fix constant warnings
* version increase to 0.46
* keep some more place for text allocation
* dump warnings to console
* convert to new function type naming and remove HAVE_OY macros
* update
* set configure to error on failed Oyranos detection
* initialise some ICClist arrays
* fix for the last (f2dd3f92b...) commit to the CinePaint plug-ins
* update to Oyranos renaming for oyGetMonitorProfileNameFromDB()
* switch to Oyranos' new include path propagation
* update to a removed struct member in Oyranos
* omit monitor profile name
* update
* support new CinePaint version scheme
* sync configure.sh mit ICC Examin
* repair RPM building
* hope to not break normale drawing
* search for i18n files more robust
* change to new include scheme
* update
* adapt the older CinePaint detection
* FTGL deprecates old include files
* add intptr test file
* detect compiler settings for 32/64-bit builds suring configure
* [docu]: give credit to translators and contributors of data
* [oyra]: adapt to Oyranos changed oyColourConversion_Create()
* [new]: add OpenMP support for vrml parsing
* [build]: ifdef backward compatible to Oyranos v0.1.8
* [fix]: rendering intent selection for 3D view
* [oyra]: add oyOptions_s to ICCmeasurement and ICCexamin
* [and]: prohibit accidential ICCexamin copies
* [and]: rework the debug system
* [and]: rework the debug system
* [and]: rework the debug system
* [conf]: update to Oyranos configure.sh
* [build]: check for configuration file changes
* [conf]: remove GNU "echo -e" extension
* [build]: fix a compile warning for a empty source file
* [conf]: update to Oyranos
* [build]: configure and build Cin�Paint plug-in rpm

### Milan Kn�zek (2):
* [po]: changes to cs.po
* [po]: add czesh translation

### Pascal LACROIX (1):
* [po]: french translations



2007-11-08
* neu: add --with-rpath configure option

2007-09-11
* fix: #1792127 quality report not savable  in new file

2007-09-03
* und: move some helper functions to icc_helfer.cpp
* neu: create oyValuexxx byte ordering utilities in icc_oyranos_extern.x
* neu: make icc_oyranos_extern.c/h and others C compatible
* neu: continue with oyNamedColour_s struct and access functions

2007-06-20
* neu: switch to v0.45
* fix: Fl_Window::set_modal() is not undoable: remove
* fix: fix saving: filechooser dateiwahl callback is not sete by default
* fix: bug#1745530 FL_WHEN_CHANGED prevents double clicks on directories in filechooser: solution fileList->when(FL_WHEN_CHANGED) -> when(3)

2007-06-29
* neu: ncl2 loockup from 2D view
* und: user defined selection order in file browser (FL_WHEN_CHANGED + own list)
* neu: load patch names into GL_Ansicht
* neu: new ncl2 tag query for ncl2_names to show a list of patch names - ICCtag::getText(std::string text)
* neu: handle patch names in ICCexamin::farbenLese
* new: single ncl2 patch in ICCexamin::farbenLese as negative n arg
* und: added ncl2 to selectTextsLine callback
* neu: mark ncl2 and cgats tags text lines containing colour patches
* neu: synchronise html report view with cgats tag text line selection
* neu: add name to each patch to make it selectable in html report
* neu: ICCmeasurement::getPatchLines helper to mark cgats tag text lines
* und: rename ICCmeasurement::getLine() ICCmeasurement::getPatchLine()

2007-06-28
* neu: show selected measurement patch in 3D view
* fix: HH: icc_cgats_filter.cpp key word matches complete string
* neu: remember original patch line number from CGATS file
* neu: new ICCmeasurement::getLine() informations from CGATS to patch
* neu: new GL_Ansicht::emphasizePoint() to show a point of interesst
* fix: fix continued key applyance after key release - 3D cut plane
* neu: new bool ICCprofile::tagBelongsToMeasurement() tells about tag
* und: ICCnetzPunkt with new member name used by emphasizePoint()
* fix: makefile: trigger source generation with fluid at missing header

2007-06-27
* neu: build osX universal iccgamut

2007-06-26
* neu: configure: support --disable-dependency-tracking configure option
* und: configure: remove cc/CC and c++/CXX modifiers for x86_64 builds
* fix: configure: fix for user specified fltk-config file
* und: configure.sh: more sensible test for libraries
* und: configure.sh: remove optimisations from external library flags
* neu: configure.sh: detect utf-8 support in FLTK by config script name
* und: configure.sh: more sensible debug options for Darwin
* fix: icc_betrachter.fl: Solaris compile errors
* neu: gui: new dynamic table channel selection buttons
* fix: move pthread_attr_getstacksize to DEBUG for Solaris

2007-06-23
* neu: icc_helfer.cxx/h: getColorSpaceGeneric, getChannelNamesShort
* neu: i18n: set charset according to utf-8 support in FLTK

2007-06-23
* fix: icc_profile_tags.cpp: detect wrong desc count and print warning
* neu: ICCtag::colorSpace (MftChain typ)

2007-06-13
* fix: additional Fl_GL_Window::set_visible() after show() for osX FLTK:STR 1707

2007-04-18
* und: remove non translations from icc_cgats_filter.cpp
* und: translate many comments to english

2007-03-06
* neu: add MCH5++ colour space signatures
* neu: render input profile gamut by directly going from max RGB->Lab

2007-02-28
* und: build system: check for Xpm, Xext, dl, iconv and intl
* neu: check for building in CinePaint source tree.
* und: allow compiling without i18n libs
* und: keep progressbar spinning
* fix: hide all lower widgets to not confuse FLTK event paths
* fix: false variable assignment for background colour
* fix: move global variables to GL_Ansicht object
* fix: valgrind warning in icc_gl.cpp
* und: simplify the motion events

2007-02-20
* fix: empty pointer chrash in locale path search (icc_helfer.cpp)

2007-02-09
* neu: allow Lab without XYZ measurements
* und: new conversion functions for Lab/XYZ and Lab/CIELab

2007-02-06
* und: allocate more memory to thread (in a wrong attempt to fix osX)

2007-02-01
* neu: gray out non selectable profiles (empty proofing profile)

2007-01-00
* fix: plastic workaround for X11, call Fl_Window::show() in
	MyFl_Double_Window::show()
* neu: configure: add -lrt for nanosleep in Solaris
* und: adapt icc_betrachter.fl to FLTK-1.1.(8) and -scheme plastic
* und: reactivate My_Fl_Box to allow DnD on Linux
	submit www.fltk.org/str.php?L1577
* neu: ICCexamin::oyranos_einstellunigen ICCexamin::auffrischen
* neu: ncl2: track old colour places
* neu: show net lines of internal gamut hull (map Lab cubus to profile)
	Oyranos::netzVonProfil_
* neu: add bpc option to colour space hull generation
* neu: icc_helfer.cpp getColorSpaceChannels()
* und: optimise GL drawing
* neu: GL herausNormalPunkte
* und: remove GL_Ansicht::stupps_
* fix: bugfix in CIELab2Lab
* und: move plug-in stuff to icc_cinepaint_internal.cpp for i18n
* fix: CP plug-in: bugfix for 1 byte proof profile size
* neu: add event type info to my_file_chooser callback_
* und: configure: rename ARCH to OS_EXT to avoid name collision

2006-12-29
* neu: ICCereignisManager
* und: move darf_bewegen_ from GL_Ansicht to Agviewer

2006-12-24
* und: check _NET_CURRENT_DESKTOP for X11 window iconification

2006-12-22
* und: beautify configure output
* und: allow silent configure operation

2006-12-05
* neu: add libtiff check
* neu: integrate CinePaint plug-in
* und: move some functions out of icc_main to icc_helfer

2006-12-04
* und: avoid Lab,XYZ... nameclash in icc_formeln.*, rename to *_s

2006-11-20
* und: reduce to a more apropriate authors list

2006-11-20
* fix: switched from XYZ gamut rendering to intermediate over Lab,
	still does not match argylls gamut
* fix: fix some memory leaks in icc_gl.cpp

2006-11-19
* neu: add the Oyranos proof profile
* neu: create and use oyProfileGetMD5 in Oyranos and icc_oyranos
* neu: the Lab to monitor now better checks for changes and recalculates
* neu: preferences saving for marking out of gamut colours

2006-11-17
* neu: "Quit" in 3D Gamut window
* fix: fix mft_choice empty entry
* neu: new registering of threads to be posix conform
* neu: preferences saving for 3D gamut and mft CLUT views
* und: merge into fortschritt and omit fortschrittThreaded
* neu: gamut warning option

2006-11-16
* und: move darf_bewegen_ from GL_Ansicht to Agviewer
* und: add hotspot() call to help dialogs showing for twm
* und: move dateiwahl initialisation from ICCbetrachter to ICCexamin
* und: renaming fenster_id to typ in GL_Ansicht
* neu: new class EinModell to object all GL_Asicht in alle_gl_fenster
	fixes GL context switches bug for menu handling
* und: handle Fl_Window::xclass setting in MyFl_Double_Window
* fix: show icon for all windows
* neu: handle MyFl_Double_Window::is_toolbox; not _NET_WM_WINDOW_TYPE,
	for icc_waehler_
* fix: update fl_translate_file_chooser() to MyFl_File_Chooser
* fix: fix empty measurements crash
* fix: empty observer list crash in Modell::beobachterFort

2006-11-15
* neu: "Show Main Window" in 3D Gamut window
* fix: fix empty icc_waehler_ calling crashes

2006-11-14
* und: reduce to a more apropriate authors list
* neu: save configuration to config.log
* neu: include CFLAGS, CXXFLAGS and LDFLAGS
* fix: fix configure for Solaris shell syntax
* und: update configure.sh from Oyranos
* und: clean up zeigMftTabellen() syntax
* und: more clean ups for the positioning code in icc_examin.h/cpp
* und: move title setting into setzeFensterTitel()
* und: new setzMesswerte() for updating the report table
* und: move the oeffnen() code from ICCbetrachter to ICCexamin
* und: change GL timing from Fl::add_timeout() to
	smoother Fl::repeat_timeout()
* und: override show() and hide() in GL_Ansicht to better control
* fix: fix warnings for non -DDEBUG compiling
* neu: flstring.c and vsnprintf.c from fltk project

2006-11-13
* und: GL timing makes more sense
* und: change DYLD_LIBRARY_PATH to DYLD_FALLBACK_LIBRARY_PATH for Tiger
* neu: add label() to MyFl_Double_Window for handling the title name

2006-11-11
* neu: MyFl_Double_Window to handle iconization
* neu: MyFl_File_Chooser to better bring in sync with MyFl_Double_Window
* and: dangerous: flstring.h is included

2006-11-08
* fix: dont influence the locale settings; (in cgats, vrml ...)
* neu: give config.h the usual header macros
* fix: mft choice stretching behaviour
* neu: make agviewer a observable pointer in GL_Ansicht
* neu: register GL_Ansicht::agv_ to agviever's icc_examin_ns::Modell
* neu: add GL_Ansicht::getAgv()
* und: delete Modell if no observers are attached

2006-11-07
* neu: icValueUI16() and icUI16Value() in icc_helfer.h/cpp
* und: rename header_raw() to headerRaw() in ICCheader
* und: smart tag adding in ICCprofile::addTag()

2006-11-05
* fix: Cmyk measurement remove on vrml net loading

2006-11-04
* und: int    getTagByName("xxx") -> int getTagIDByName
* neu: ICCtag getTag("xxx")
* und: void removeTagByName (std::string) -> int removeTag (std::string)

2006-09-14
* und: monitor profile is passive on load time

2006-09-11
* neu: class ThreadDaten - secure data access in threaded environments

2006-09-10
* und: change CgatsFilter class inside to better handle special fields
* neu: parse CGATS internaly (class ICCmeasurement)

2006-09-08
* neu: move tests files to tests
* neu: add threads test to fltk check

2006-08-11
* und: v0.41

2006-07-21
* fix: measurement scaling for CLUT Profiles

2006-06-28
* neu: Uebertragungsarten frei waehlbar

2006-06-20
* fix: Xinerama / screen bugs

2006-06-19
* neu: ICCvrmlDATA ICCcorruptedprofileDATA in ICCprofile::ICCDataType
* neu: Anzeige des Datentyps in Benutzeroberflaeche
* und: Hinweis: eventuell falsche Farbtransformation (Qualitaetsbericht)
* fix: mehrere Datenabfragen, werden besser ueberprueft vor dem Zugriff
* neu: setzten von 0 Daten bei fehlenden Profileintraegen
* und: Darstellung des Profilkopfeintrages nur f�r ICC Typen
* und: neue Zeilenvorauswahl in icc_betrachter fuer nicht ICC Profile
* fix: Xinerama / Screen bugs

2006-06-18
* fix: 0 cgats -> lcms wird weggelassen (Fehler von HalVEngel berichtet)
* fix: Warnung fuer Eintraege in Tag Tabelle ausserhalb der Datei

2006-06-12
* und: veraendertes zaehlen der Screen's (Xinerama screens)

2006-04-28
* fix: Versionsnummer in configure

2006-04-12
* new: Xinerama Monitornummer Erkennung
* und: v0.40

2006-04-03
* und: v0.39 Ver�ffentlichung

2006-04-01
* neu: Darstellungsoptionen werden respektiert (aktiv/passiv)
* neu: Die ersten nachfolgende Messdaten gehen in das erste Profil ein.
* fix: Ansicht synchronisiert mit ver�nderter Profilkonstellation

2006-03-06
* neu: Monitor Gamma wird kontinuierlich beobachtet

2005-12-20
* neu: Beleuchtung f�r Farbk�rper ein

2005-12-19
* neu: Oberfl�chennormale berechnen

2005-12-14
* und: osX dock unterst�tzt, Standardprofilbetrachter

2005-11-14
* fix: vrml lesen: IndexedFaceSets alleinig
* und: (v0.39)

2005-11-09
* und: configure nimmt --prefix=xxx an beliebiger Position
* und: Mandrake Test �berstanden

2005-10-21
* und: (v0.38-2) angekuendigt
* und: TagTexts bleiben auf Ihrer H�he stehen auch bei Neuladen des Inhaltes

2005-10-18
* und: (v0.38) veroeffentlichen

2005-10-14
* und: freeglut beseitigt

2005-10-13
* fix: ncl2 erste Farbe nicht �bertragen in ICCexamin::farbenLese
* fix: Laden/Benachrichtigen verbessert:
* und: set f�r Warteschleife der zu erneuernden Dateien in ICCexamin::erneuern_
* und: erneuern Funktionen von .h -> .cpp
* und: benachrichtigen aus setzAktiv und passiv entfernt (ICCkette)
* und: Schleife aus ICCexamin::oeffnenThread_(int pos) -> schlafen
* und: Test auf Namen f�r Profilladen in ICCkette::einfuegen -> Pause

2005-10-07
* und: configure acepts --prefix=xxxxx
* und: letzen Oyranos Aenderungen
* und: v0.38
* und: unix name icc_examin -> iccexamin

2005-10-05
* und: -psn_##### geloescht von Kommandozeile

2005-09-19
* fix: Profile Laden in WAECHTER Strang -> LADEN Strang
* fix: chrm tag
* neu: Farbanteile aus chrm im Fenster

2005-09-14
* fix: kein direkter Fl Zeichenaufruf in nicht Nebenstr�ngen
       ICCexamin::fortschrittThreaded() wait(0) -> damage(FL_DAMAGE_ALL)
* neu: ncl2 und Messfarben auch farbig darstellbar

2005-09-13
* fix: i18n codeset for osX with newer gettext
* fix: Proofprofil wird immer relativ colorimetrisch dargestellt
* neu: Auswahlfarben in Statuszeile

2005-09-07
* fix: sprintf durch memcpy ersetzt f�r URLs; sprintf kann keine %20 Zeichen

2005-09-06
* und: Anpassen an Oyranos-0.1.2
* fix: statisches und dynamisches Verkn�pfen mit Elektra
* und: Export einiger icc_oyranos Funktionen nach Oyranos
* fix: filtern von %20 in URLs

2005-09-01
* fix: more cout << (char*) 0; in icc_oyranos.cpp
* fix: Darstellen einer Messtabelle mit null L�nge ; noch einen

2005-08-31
* und: www Seite
* und: v0.37
* fix: cout << (char*) 0; in icc_oyranos.cpp
* fix: Darstellen einer Messtabelle mit null L�nge

2005-08-30
* neu: makefile Ziel pkg/unpkg (osX)
* und: Release 0.36

2005-08-28
* fix: Gamut Warnung w�hlbar auch f�r Messungen TODO: update
* neu: X icon
* neu: punktgroesse f�r zu viel ncl2 Farben auf 2 voreingestellt

2005-08-27
* fix: nicht verstecktes mft_gl ??bug ein zweites mal verstecken
* fix: neuzeichnen zu zeitiges waehleTag
* und: Men�organisation
* und: FL_COMMAND+o ist in icc_examin.cpp tastatur() definiert

2005-08-26
* und: neue mutex_trylock nur f�r fremde Zweige
* neu: Zeit Funktionen
* fix: OpenGLgl�tten besser steuern
* und: Darstellungsoptionen auffrischen und neue Fenstergr��e
* fix: nur aktive Netze werden benamt OpenGL
* fix: L�schen alter H�llen
* fix: 0 Farben ncl2 Profilabsturz

2005-08-25
* und: OpenGL wieder im Hauptzweig
* neu: Fl::wait(pause/0) sorgen f�r eine fl�ssige Oberfl�che
* und: Zeitkoordination

2005-08-24
* und: Zeitmessung ordnen

2005-08-23
* neu: mutexe f�r das Entlaussystem (icc_utils)
* neu: OpenGL threads

2005-08-20
* neu: Start esperanto in po/eo.po
* neu: eigenst�ndiges "bundle" Ziel in makefile

2005-08-18
* und: Resoucen in Bundle orten

2005-08-14
* neu: Bildchen

2005-08-13
* fix: farbraummodus (ncl2) wieder aktiviert f�r CinePaint
* neu: mime f�r Dateimanager - icc.xml

2005-08-12
* neu: Fehlersuche zeigt threads farbig an, bessere Lesbarkeit
* und: nach 'export ICCEXAMIN_DEBUG=19' zeigt icc_examin Funktionesaufrufe
* fix: Verklemmungen
* fix: unerschiedliche Farben in ICCgl osX/X11
* fix: L�schen alter H�llumrisse und Namen in ICCgl
* fix: Maus Ereignisse weiterreichen an ICCgl

2005-08-11
* und: threads
* und: Datei Behandlung einfacher, Dateizugriff aus ICCprofile entfernt
* und: Datei/Speicher Funktionen in icc_helfer.h/cpp
* und: -fnoexeptions + keine Optimierung w�hrend des �bersetzens : schneller

2005-08-10
* fix: DnD werden vom Fenster akzeptiert mittels eines transparenten Blockes

2005-08-07
* neu: KDE + Gnome + FLTK DnD
* neu: Startverzeichniss sinnvoller

2005-08-06
* neu: osX Fl_Sys_Menu wieder entfernt, da nicht kompatibel zu fltk Men�s:
	   Textkodierungsprobleme, kein hide(), kein toogle in Men�leiste,
	   kein update

2005-07-29
* und: aufraeumen in icc_kette.cpp ::einfuegen()
* und: statisches Binden hilft mit osX-Dock
* und: Version 0.36
* und: Speicher kopiert und �bernimmt lediglich mit new reservierte Bereiche
	   direkt, da Freigabefunktion sonst unbekannt, neue Zuweisungsfunktionen

2005-07-24
* neu: osX DND

2005-07-22
* und: osX icc_examin.r eingefuehrt - Menues schwierig

2005-07-21
* neu: osX Display Profil erkennen und auslesen + vcgt aus Ger�t auslesen

2005-07-18
* neu: Oeffnen vereinfacht -> Speicherbloecke

2005-07-10
* neu: VRML schreiben - Version 0.35

2005-06-04
* und: Farbh�llwarnung f�r Messfarben immer, f�r Lab Auswahl im oberen Men�

2005-06-03
* und: farbiger Balken von gew�hlten 3D Punkt Lab -> 0ab

2005-06-02
* und: schnellere OpenGL Tabellendarstellung

2005-05-30
* neu: Brerechnen der Bilder pro Sekunde und an-/abschalten von aufwendigen
       OpenGL Zeichenfunktionen
* neu: Text folgt Kamera
* und: Version 0.33

2005-05-26
* neu: Umrisse f�r Rgb und Cmy/Cmyk Profile

2005-05-19
* neu: im Fenstertitel wird der Dateiname gesetzt
* neu: Gitterlinien im Farbraumbetrachter
* neu: Schatten f�r das Spektralband

2005-05-09
* und: Umstellung nach Oyranos API 0.0.2
* und: Version 0.32
* fix: f�r farbraumModus
* und: more Linguas in icc_helfer_i18n.cpp

2005-05-03
* fix: tag selection crash 
* und: Darstellungoptionen werden versteckt (__APPLE__)
       und unter unix versteckt
* fix: Endianes f�r Solaris korrigiert
* fix: setenv unter Solaris nicht verf�gbar

2005-05-02
* fix: GL zeichnen wenn ansichtig
* fix: Infos in Statusleiste

2005-05-01
* und: OpenGL Test etwas feiner
* und: Darstellungoptionen werden versteckt nicht mehr verkleinert
* und: neuzeichnen f�r farbraumModus

2005-04-30
* und: Schalter hintergrundfarbe in Farbwert umgestaltet
* und: set charset to iso-8859-1 for 'de' in unix
* und: i18n stuff resides in icc_helfer_i18n.cpp

2005-04-29
* und: set LANG variable in osX for i18n

2005-04-28
* und: Layout im Tagw�hler nach links ger�ckt
* fix: Profile bleiben gew�hlt wie im Profilew�hler eingestellt
* fix: Namen werden im GL Fenster so dargestellt wie im Profilew�hler
* neu: Schatten f�r Einzelfarben hinzugef�gt

2005-04-27
* und: de �bersetzung vollst�ndig
* fix: *.po is now the translation file and *.utf-8 temporarily used
* fix: ncl2 korrekt angezeigt
* fix: Voreinstellungen der Farbigkeit im Farbraummodus verbessert

2005-04-26
* und: Men�s Texte werden ausgetauscht; Fluid Zeigernamen bleiben erhalten
* und: i18n: msgmerge obtains utf-8; de: recode -> latin-1 +
* und:       set charset=latin-1 + msgfmt obtains latin-1 => fltk=correct

2005-04-25
* fix: Objekt scope Fehler in icc_betrachter/icc_helfer_fltk

2005-04-22
* neu: gettext �bersetzung in fluid und makefile

2005-04-18
* neu: Fonts mit FTGL: nicht innerhalb anderer glLists benutzen
* neu:                 Raster Position mit glRasterPos* setzen
* fix: Farbeinstellungen im 3D Ansichtw�hler bleiben erhalten
* fix: setzMesswerte nicht falls keine vorhanden sind
* fix: alte Messwerte mit Laden eines neuen Profiles l�schen
* fix: zugriff auf nicht existentes 3D Netz 

2005-04-01
* neu: aktualisieren der Ansichten nach ICCkette::benachrichtigen ->
	   setzMesswerte

2005-03-31
* neu: agviewer ist nun glut frei
* neu: profile werden in einem eigenen Fenster angezeigt

2005-03-12
* neu: automatische Bibliothekskonfiguration in configure.sh

2005-02-07
* fix: osX nutzt andere Verbundobjekte als Linux in time.h
* fix: einige int -> size_t in icc_profile

2005-03-0
* und: pthread -> Fl::add_timeout in icc_kette

2005-02-28
* neu: ICCexamin::frei() zur Anzeige der Benutzung eingef�hrt (pthread)

2005-02-26
* neu: w�chter mit pthread eingef�hrt in ICCkette, �berwacht Dateiver�nderng
* neu: Modell Beobachter als R�ckmeldesystem eingef�hrt icc_modell_bobachter
* neu: holeDateiModifikationsZeit in icc_helfer
* neu: ICCprofile::changing() zur Anzeige der Benutzung eingef�hrt

2005-02-24
* neu: viel an der 3D Darstellung gearbeitet
* neu: Umgebungsvariable LANG wird zum Auslesen von mluc Tags verwendet
* fix: Profilenamen werden alle angezeigt; kein glLoadIdentity() f�r Text
* neu: icc_profile.h viele Funktionen in cpp Dateien verschoben
* neu: ncl2 l�uft als Text

2005-02-11
* fix: Beschnitt der Achsenbeschriftung aufgehoben f�r CIExy
* fix: agvSwitchMoveMode[1,2] nach Fenstern aufgeteilt

2005-02-10
* und: TagDrawings fertig verkappselt, Daten liegen innen

2005-02-08
* neu: Zeichenfunktionen in TagDrawings Klasse verbannt

2005-02-05
* neu: vrml Parser

2005-02-02
* neu: benutzt Monitorprofil zur Anzeige; exportiert html-Farben als sRGB
* neu: Monitorprofil in icc_draw und icc_gl

2005-01-28
* neu: oyranos Objekt zum Verwalten der Profile

2005-01-27
* neu: CGATS Lesen ist robuster

2005-01-21
* und: XYZ Messdaten ohne Ger�tefarben werden beim Laden als Stern gezeigt
* neu: suchenErsetzen in icc_helfer.h/cpp
* neu: CGATS Daten �ber icc_examin exportiert
* neu: nachricht() in icc_fenster.h/cpp zum Anzeigen von Hinweisen
* neu: CGATS Fenster mit [ctrl-g] aufrufbar

2005-01-20
* neu: Spektralbanddarstellung fertig
* neu: Laden von Messdateien -> 3D Farbraum
* neu: [Escape] wird in icc_examin als Tastenk�rzel abgefangen

2005-01-19
* neu: Men�s neu sortiert
* neu: Pfeile/Texte an und aus
* neu: Farbvariablen f�r Text und Pfeile
* neu: Hintergrund umschaltbar
* und: Spektralbanddarstellung vorbereitet - neue Liste SPEKTRAL
* neu: Messdatendifferenzen mit Verlauf versehen
* neu: init teilweise auf Anwahl �ber Men� umgestellt

2005-01-18
* fix: GL Fenster 2 bewegt sich wenn Fenster 1 angeschoben wurde
* fix: Men� f�r GL 1 und GL 2 differenziert angezeigt
* fix: html Bericht Speichern

2005-01-17
* neu: Messdaten / Differenz im Farbraumfenster
* neu: Men� f�r Messdaten in GL : Kugel, Stern , nur Differenzen
* neu: Spektralbanddarstellung vorbereitet

2005-01-15
* fix: aktualisiere GL nach Profilladen
* fix: Daten �bertragung (Farben)
* fix: Daten Skalierung

version 0.17
2005-01-14
* neu: Messdaten im Farbraumfenster darstellen 

2005-01-13
* neu: drawCurve/CIE greift �ber id von TagDrawing auf Daten in icc_examin
* neu: DDC Auslese via X - bisher nur Monitor Model und Hersteller
* neu: Gamma Auslese aus X - 

2005-01-12
* neu: neue X Dateien icc_helfer_x.h/cpp
* neu: vcgt Fenster

2005-01-11
* und: icc_kette von icc_betrachter gereinigt
* neu: Datei icc_fenster.h/cpp mit Dateiauswahlfenster
* fix: leerer string l�st Ausnahme bei Zugriff aus in icc_gl.h - DBG Macros!

2005-01-09/10
* fix: doppelter Aufruf von auswahlCb - in MftChoice::profilTag
* und: Textausgabe in GL_Ansicht �berarbeitet
* und: glut Men�s neu sortiert

2005-01-08
* neu: Initialisierungsreihenfolge der GL_Ansicht in icc_examin
* neu: statische glut Funktionen je GL_Ansicht in icc_gl.h/cpp
* neu: zweiphasigiges Initialisierungs Muster erkannt

2005-01-07
* neu: erstes Signal(nix)

2005-01-06
* und: weiterer Umbau zu Model/Observer Muster
* neu: weitere Integration von OpenGL in Klasse GL_Ansicht und Agviewer
* neu: GL_Ansicht um Laden von Netzen erweitert
* neu: Kette von Profilen in ICCkette implementiert (vorher vector)

2004-12-31
* und: Gedanken �ber Farbraumh�lle
*    : maximaler Farbauftrag f�r maximale Farbs�ttigung
*    : Raster (polar oder orthogonal) mit Pr�fung des relativen Intents
	auf Ver�nderung; Begin der Ver�nderung zeigt das Innere des 
	Farbraumes an
     : Grenze Abtasten bei Verdacht

2004-12-30
* und: OpenGL Tests;  Transparenz - scheint standpunktabh�ngig

2004-12-28
* und: Tag- und Mft-auswahl von icc_betrachter -> icc_examin
* fix: vcgt Kurven umkehren
* und: Klasse GL_Ansicht wird von icc_examin als vector verwaltet

2004-12-27
* neu: ICCfltkBetrachter als fltk Klasse geformt (Fenster und Garnitur)
* und: �ffnen nach icc_examin verschoben
* und: Statuszeile ist string in icc_examin
* neu: ICCfltkBetrachter: aus start() wurden init() und eine run()
* und: agv: Tastatur Signale ausgeschalten
* und: icc_betrachter: globale Variablen -> ICCfltkBetrachter
* und: icc_draw() Variablen -> icc_examin

2004-12-14
* fix: mft GL Untermen�: Kanalnamen mit jedem Tagwechsel aktualisiert
* neu: Fl::run aus main entfernt, Oberfl�che nun in icc_betrachter
* neu: Klasse icc_examin als zentrales Model begonnen
* neu: neue Farben der Oberfl�che, Einheitlich zu osX

2004-12-6
* neu: CLUT Koordinaten besser positioniert 
* fix: CLUT A2B nicht mehr dargestellt
* neu: CLUT B2A alle Kan�le w�hlbar
* und: bessere Beschriftung namenloser Farbkan�le
* und: RGB und CMYK Messdaten gemeinsam berechnet
* neu: Graudarstellung begonnen, noch fehlerhaft
* neu: Funktionen f�r Farbkanalanzahl und Test ob CLUT vorhanden
* und: Optimierung f�r die Entwicklung ausgeschaltet
* und: nicht ben�tigte Bibliotheken aus make entfernt 

2004-12-5
* bug: Weisspunkt f�r RGB Messdaten ber�cksichtigt
* bug: Dunkelablgleich f�r RGB Messdaten
* bug: GL Fenster wird nicht mehr bei Tagwechsel auf 1x1 verkleinert

2004-12-1
* und: DEBUG ge�ndert
* bug: Hellabgleich XYZ Messdaten
* bug: A2B versus B2A Tabellen besser verstanden

2004/11/25
* neu: Start oyranos Integration (Konfiguration)
* neu: Men�s neu ordnen

2004/11/24
* neu: bin�r/ascii Darstellung unbekannter Inhalte
* neu: tag mluc - mehrsprachige Beschreibungen
* neu: tag chad - Farbanpassungsmatrix
* fix: Speicherfreigabe bei CIExy

