CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

PROJECT (ICC_Examin)

SET (ICCEXAMIN_VERSION_MAJOR 0)
SET (ICCEXAMIN_VERSION_MINOR 5)
SET (ICCEXAMIN_VERSION_MICRO 6)

SET (ICCEXAMIN_DEVEL_MONTH 11)
SET (ICCEXAMIN_DEVEL_YEAR 2014)

SET (PACKAGE_NAME "icc-examin")
set (PACKAGE_DESCRIPTION "ICC Examin is a small utility (unix name: iccexamin) for the purpose of watching the internals of ICC profiles, measurement data (CGATS), colour samples (named colour profiles), gamut visualisations (vrml), video card gamma tables (Xorg/XFree86/osX).")

#additional cmake modules for loading libraries
SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)


# some basic and important variables
SET( DOMAIN oyranos.org ) #needed by doxygen
SET( REVERSE_DOMAIN org.oyranos )

SET( ICCEXAMIN_VERSION "${ICCEXAMIN_VERSION_MAJOR}.${ICCEXAMIN_VERSION_MINOR}.${ICCEXAMIN_VERSION_MICRO}" )
SET( ICCEXAMIN_VERSION_MAJORMINOR "${ICCEXAMIN_VERSION_MAJOR}.${ICCEXAMIN_VERSION_MINOR}" )
SET( ICC_EXAMIN_V "\"${ICCEXAMIN_VERSION_MAJOR}.${ICCEXAMIN_VERSION_MINOR}${ICCEXAMIN_VERSION_MICRO}\"" )


IF( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(PLATFORM "OSX")
  SET( gamma_view icc_helfer_osx.cpp )
ELSEIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(PLATFORM "Linux")
ELSE( ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(PLATFORM "${CMAKE_SYSTEM_NAME}")
ENDIF( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
MESSAGE( "-- Found PLATFORM: ${PLATFORM}" )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/FL
                     ${CMAKE_SOURCE_DIR}/Flmm
                     ${CMAKE_BINARY_DIR}
                   )

#additional cmake modules for loading libraries
SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

FIND_PACKAGE( Oyranos REQUIRED )
INCLUDE_DIRECTORIES( ${OYRANOS_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${OYRANOS_LIBRARY_DIRS} )

FIND_PACKAGE( Xcm REQUIRED )
INCLUDE_DIRECTORIES( ${XCM_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${XCM_LIBRARY_DIRS} )

FIND_PACKAGE( X11 )
IF(X11_FOUND)
  INCLUDE_DIRECTORIES( ${X11_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${X11_LIBRARY_DIRS} )
  SET( HAVE_X TRUE )
  SET( HAVE_X11 TRUE )
  SET( gamma_view icc_helfer_x.cpp )
  IF(X11_Xrandr_FOUND)
    SET( HAVE_XRANDR TRUE )
  ENDIF()
  IF(X11_xf86vmode_FOUND)
    SET( HAVE_XF86VMODE TRUE )
  ENDIF()
  IF(X11_Xinerama_FOUND)
    SET( HAVE_XIN TRUE )
  ENDIF()
  IF(X11_Xpm_FOUND)
    SET( HAVE_Xpm TRUE )
  ENDIF()
ENDIF()

FIND_PACKAGE( FLTK REQUIRED )
IF(FLTK_FOUND)
  SET( HAVE_FLTK TRUE )
  SET( HAVE_FLTK_UTF8 TRUE )
ENDIF()
INCLUDE_DIRECTORIES( ${FLTK_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${FLTK_LIBRARY_DIRS} )

FIND_PACKAGE( OpenGL )
IF(OPENGL_FOUND)
  SET( HAVE_OPENGL TRUE )
ENDIF()
INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR} )
LINK_DIRECTORIES( ${OPENGL_LIBRARY_DIRS} )

FIND_PACKAGE( FTGL )
IF(FTGL_FOUND)
  SET( HAVE_FTGL TRUE )
ENDIF()
INCLUDE_DIRECTORIES( ${FTGL_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${FTGL_LIBRARY_DIRS} )

FIND_PACKAGE( Freetype )
IF(FREETYPE_FOUND)
  SET( HAVE_FONTCONFIG )
  SET( EXTRA_LIBS ${EXTRA_LIBS} fontconfig )
ENDIF()
INCLUDE_DIRECTORIES( ${FREETYPE_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${FREETYPE_LIBRARY_DIRS} )

FIND_PACKAGE( Threads )
IF(Threads_FOUND)
  SET( HAVE_PTHREAD TRUE )
  SET( HAVE_PTHREAD_H TRUE )
ENDIF()

FIND_PACKAGE( GetText )
MESSAGE( "GetText: ${GETTEXT_FOUND}" )
IF( GETTEXT_FOUND )
  SET( USE_GETTEXT TRUE )
ENDIF()

CONFIGURE_FILE (
		"${CMAKE_SOURCE_DIR}/config.h.in"
		"${CMAKE_BINARY_DIR}/config.h"
		@ONLY
	)
CONFIGURE_FILE (
		"${CMAKE_SOURCE_DIR}/config.log.h.in"
		"${CMAKE_BINARY_DIR}/config.log.h"
		@ONLY
	)
CONFIGURE_FILE (
		"${CMAKE_SOURCE_DIR}/icc_examin_version.h.in"
		"${CMAKE_BINARY_DIR}/icc_examin_version.h"
		@ONLY
	)

SET( COMMON_CPPFILES
	agviewer.cpp
        bsp/bspAlloc.c
        bsp/bspCollide.c
        bsp/bspMemory.c
        bsp/bspPartition.c
        bsp/bspTree.c
        bsp/bspUtility.c
        fl_i18n/fl_i18n.cxx
        Flmm/Flmm_Tabs.cxx
        ${gamma_view}
	icc_cgats_filter.cpp
	icc_draw.cpp
	icc_examin.cpp
	icc_examin_farbraum.cpp
	icc_examin_io.cpp
	icc_examin_tagwahl.cpp
	icc_fenster.cpp
	icc_formeln.cpp
	icc_gamut.cpp
	icc_gl.cpp
	icc_helfer.cpp
        icc_helfer_fltk.cpp
	icc_info.cpp
	icc_kette.cpp
	icc_main.cpp
	icc_measurement.cpp
	icc_model_observer.cpp
	icc_oyranos.cpp
	icc_profile.cpp
	icc_profile_header.cpp
	icc_profile_tags.cpp
	icc_schnell.cpp
	icc_speicher.cpp
	icc_threads.cpp
	icc_thread_daten.cpp
	icc_ueber.cpp
	icc_utils.cpp
	icc_vrml.cpp
	icc_vrml_parser.cpp
	icc_betrachter.cxx
	my_file_chooser.cxx
	my_file_chooser2.cxx
	my_fl_double_window.cxx
	threads.cpp
	icc_oyranos_extern.c
	flstring.c
	vsnprintf.c
   )
SET( icc_examin_SRCS
     ${COMMON_CPPFILES}
   )

ADD_EXECUTABLE( iccexamin ${icc_examin_SRCS} )
TARGET_LINK_LIBRARIES ( iccexamin oyranos ${EXTRA_LIBS}
                 ${FLTK_LIBRARIES}
                 ${OYRANOS_LIBRARIES}
                 ${XCM_LIBRARIES}
                 ${FTGL_LIBRARIES}
                 ${FREETYPE_LIBRARIES}
                 ${OPENGL_LIBRARIES}
                 ${X11_Xinerama_LIB}
                 ${X11_Xpm_LIB}
                 ${X11_Xrandr_LIB}
                 ${X11_Xxf86vm_LIB}
                 ${CMAKE_THREAD_LIBS_INIT} )
INSTALL( TARGETS iccexamin DESTINATION bin )
INSTALL( FILES icc_examin.desktop DESTINATION share/applications )
INSTALL( FILES icc_examin.svg DESTINATION share/pixmaps )

# i18n
IF( GETTEXT_FOUND )
  SET(_potFile ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot)
  #SET( _xgettext_option_list --language=C --keyword=_ --keyword=N_ --keyword=C_:1c,2 --keyword=NC_:1c,2 -s --package-name=${PROJECT_NAME} --package-version=${OYRANOS_VERSION_MAJOR}.${OYRANOS_VERSION_MINOR} )
  SET( _xgettext_option_list --add-comments --keyword=gettext --flag=gettext:1:pass-c-format --keyword=_ --flag=_:1:pass-c-format --keyword=N_ --flag=N_:1:pass-c-format  --copyright-holder='Kai-Uwe Behrmann'  --msgid-bugs-address='ku.b@gmx.de' --from-code=utf-8 --package-name=${PROJECT_NAME} --package-version=${OYRANOS_VERSION_MAJOR}.${OYRANOS_VERSION_MINOR} )
  SET( XGETTEXT_CMD xgettext )
  FILE( GLOB SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        *.c
        *.cpp
        *.h
      )

  ADD_CUSTOM_COMMAND(OUTPUT ${_potFile}
    COMMAND ${XGETTEXT_CMD} ${_xgettext_option_list} -o ${_potFile} ${SRC_FILES}
    DEPENDS ${SRC_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Extract translatable messages to ${_potFile}"
  )

  SET( LINGUAS po/cs.po po/de.po po/eo.po po/eu.po  po/fr.po )
  # compile translations always to have them ready for installation
  GETTEXT_CREATE_TRANSLATIONS( ${_potFile} ALL ${LINGUAS} )
ENDIF()

