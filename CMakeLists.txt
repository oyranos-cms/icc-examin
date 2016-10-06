CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

PROJECT (icc-examin)

SET (ICCEXAMIN_VERSION_MAJOR 0)
SET (ICCEXAMIN_VERSION_MINOR 5)
SET (ICCEXAMIN_VERSION_MICRO 6)

SET (ICCEXAMIN_DEVEL_MONTH January)
SET (ICCEXAMIN_DEVEL_YEAR 2016)

SET (PACKAGE_NAME "icc-examin")
set (PACKAGE_DESCRIPTION "ICC Examin is a small utility (unix name: iccexamin) for the purpose of watching the internals of ICC profiles, measurement data (CGATS), colour samples (named colour profiles), gamut visualisations (vrml), video card gamma tables (Xorg/XFree86/osX).")

#additional cmake modules for loading libraries
SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Uninstall.cmake"
    IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET( uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Uninstall.cmake")


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
  SET( gamma_view
       icc_helfer_osx.cpp
       icc_helfer_osx_cg.c
     )
ELSEIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(PLATFORM "Linux")
  SET( gamma_view icc_helfer_x.cpp )
ELSE( ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(PLATFORM "${CMAKE_SYSTEM_NAME}")
  SET( gamma_view icc_helfer_x.cpp )
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

FIND_PACKAGE( Cinepaint )
INCLUDE_DIRECTORIES( ${CINEPAINT_INCLUDE_DIRS} )
#LINK_DIRECTORIES( ${CINEPAINT_LIBRARIES} )

FIND_PACKAGE( Xcm REQUIRED )
INCLUDE_DIRECTORIES( ${XCM_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${XCM_LIBRARY_DIRS} )

FIND_PACKAGE( X11 )
IF(X11_FOUND)
  INCLUDE_DIRECTORIES( ${X11_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${X11_LIBRARY_DIRS} )
  SET( HAVE_X TRUE )
  SET( HAVE_X11 TRUE )
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
  FIND_PACKAGE( JPEG )
  IF( NOT JPEG_FOUND )
    # use fltk's own jpeg
    SET( FLTK_ADD_LIBS "fltk_jpeg" )
  ENDIF( NOT JPEG_FOUND )
ENDIF(FLTK_FOUND)
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
  INCLUDE_DIRECTORIES( ${FTGL_INCLUDE_DIRS} )
  LINK_DIRECTORIES( ${FTGL_LIBRARY_DIRS} )
  SET( EXTRA_LIBS ${EXTRA_LIBS} ${FTGL_LIBRARIES} )
ENDIF()

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

IF(APPLE)
  SET( ICCE_APP ICCExamin )

  SET( OSX_SCRIPT_FILES "${CMAKE_CURRENT_BINARY_DIR}/extras/osx/ICC Examin.sh" )
  SET( OSX_PKG_FILES ${PROJECT_SOURCE_DIR}/extras/osx/PkgInfo )

  # icon files to copy in the bundle
  SET( OSX_ICON_FILES ${PROJECT_SOURCE_DIR}/extras/osx/icc-examin.icns  )
  CONFIGURE_FILE (
                "${CMAKE_CURRENT_SOURCE_DIR}/extras/osx/ICC Examin.sh.in"
                "${OSX_SCRIPT_FILES}"
                @ONLY
                 )
  ADD_EXECUTABLE( ${ICCE_APP} MACOSX_BUNDLE ${icc_examin_SRCS} icc_main.cpp ${OSX_ICON_FILES} )

  INSTALL( TARGETS ${ICCE_APP}
           EXPORT ICCExaminLibraryDepends
           RUNTIME DESTINATION bin
           BUNDLE DESTINATION  bin
           RENAME ${ICCE_APP}.bin
         )

  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "ICC Examin" )
  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_INFO_STRING "ICC Examin front end to Oyranos Color Management System, www.oyranos.org" )
  # MACOSX_BUNDLE_EXECUTABLE_NAME var does not work with custom Info.plist.in; use TARGETS name
  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_EXECUTABLE_NAME ${ICCE_APP} )
  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_ICON_FILE icc-examin.icns )
  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER org.oyranos.icc-examin )
  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING "ICC Examin ${ICC_EXAMIN_V}" )
  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING ${ICC_EXAMIN_V} )
  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION 1.0 )
  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_COPYRIGHT "2004-2016 Kai-Uwe Behrmann" )

  # configure CMake to use a custom Info.plist
  SET_TARGET_PROPERTIES( ${ICCE_APP} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/extras/osx/Info.plist.in )

  # set where in the bundle to put the icns files
  SET_SOURCE_FILES_PROPERTIES( ${OSX_ICON_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
  #SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${ICCE_APP}.app")  # paths to executables
  SET(DIRS "\${CMAKE_INSTALL_PREFIX}/lib;\${CMAKE_CURRENT_BINARY_DIR};\${CMAKE_INSTALL_PREFIX}/Library/Frameworks")   # directories to search for prerequisites
  INSTALL(CODE "
   include(BundleUtilities)
   fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\")
   " COMPONENT Runtime)

  # this is the script to setup the environment and call the real binary
  INSTALL( PROGRAMS "${OSX_SCRIPT_FILES}" DESTINATION "bin/${ICCE_APP}.app/Contents/MacOS"
           RENAME ${ICCE_APP} )
  INSTALL( FILES "${OSX_PKG_FILES}" DESTINATION "bin/${ICCE_APP}.app/Contents" COMPONENT bin )

  # place the binary
  INSTALL( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${ICCE_APP}.app/Contents/MacOS/${ICCE_APP}
           DESTINATION "bin/${ICCE_APP}.app/Contents/MacOS"
           RENAME ${ICCE_APP}.bin )

  # install Oyranos modules
  FILE( GLOB OYRANOS_ALL_CMMS ${OYRANOS_CMM_DIR}/liboyranos_*_cmm* )
  INSTALL( FILES ${OYRANOS_ALL_CMMS}
           DESTINATION "bin/${ICCE_APP}.app/Contents/Resources/${OYRANOS_CMM_SUBDIR}"
           COMPONENT lib )
  FILE( GLOB OYRANOS_ALL_META ${OYRANOS_META_DIR}/liboyranos_*_cmm* )
  INSTALL( FILES ${OYRANOS_ALL_META}
           DESTINATION "bin/${ICCE_APP}.app/Contents/Resources/${OYRANOS_META_SUBDIR}"
           COMPONENT lib )
  FOREACH( oy_dep_lib ${OYRANOS_MODULE_LIBRARIES} )
    FILE( GLOB OYRANOS_MODULE_DEP_LIBRARIES ${OYRANOS_META_DIR}/../lib${oy_dep_lib}*.dylib )
    INSTALL( FILES ${OYRANOS_MODULE_DEP_LIBRARIES}
             DESTINATION "bin/${ICCE_APP}.app/Contents/MacOS"
             COMPONENT lib )
  ENDFOREACH( oy_dep_lib )

  #INSTALL( PROGRAMS ${OYRANOS_META_DIR}/../../bin/oyranos-profile-graph DESTINATION "bin/${ICCE_APP}.app/Contents/MacOS" COMPONENT bin )
  INSTALL( DIRECTORY ${OYRANOS_META_DIR}/../../Library/Frameworks/
           DESTINATION "bin/${ICCE_APP}.app/Contents/Frameworks"
           COMPONENT lib
           USE_SOURCE_PERMISSIONS )

  FIND_PACKAGE(PkgConfig)
  PKG_CHECK_MODULES(ELEKTRA elektra)
  INSTALL( DIRECTORY ${ELEKTRA_LIBDIR}/elektra DESTINATION "bin/${ICCE_APP}.app/Contents/Resources" COMPONENT lib )


ELSEIF(UNIX)
  SET( ICCE_APP iccexamin )
  ADD_EXECUTABLE( ${ICCE_APP} ${icc_examin_SRCS} icc_main.cpp )
  TARGET_LINK_LIBRARIES ( ${ICCE_APP}
                 ${FREETYPE_LIBRARIES}
                 ${X11_Xinerama_LIB}
                 ${X11_Xpm_LIB}
                 ${X11_Xrandr_LIB}
                 ${X11_Xxf86vm_LIB}
                 )

ELSEIF (WIN32)
  SET( ICCE_APP iccexamin )
  ADD_EXECUTABLE( ${ICCE_APP} WIN32 ${icc_examin_SRCS} icc_main.cpp )
ENDIF ()

TARGET_LINK_LIBRARIES ( ${ICCE_APP}
                 ${EXTRA_LIBS}
                 ${FLTK_LIBRARIES} ${FLTK_IMAGES_LIBS} ${FLTK_ADD_LIBS}
                 ${OYRANOS_LIBRARIES}
                 ${XCM_LIBRARIES}
                 ${OPENGL_LIBRARIES}
                 ${CMAKE_THREAD_LIBS_INIT} )

IF( NOT APPLE )
  INSTALL( TARGETS ${ICCE_APP} 
           RUNTIME DESTINATION bin )
ENDIF()
INSTALL( FILES iccexamin.desktop DESTINATION share/applications )
INSTALL( FILES iccexamin.svg DESTINATION share/pixmaps )
INSTALL( FILES iccexamin.1 DESTINATION share/man/man1 )

IF( CINEPAINT_FOUND )
  ADD_EXECUTABLE( icc_examin_cp ${icc_examin_SRCS} icc_cinepaint.cpp )
  TARGET_LINK_LIBRARIES ( icc_examin_cp ${EXTRA_LIBS}
                 ${CINEPAINT_LIBRARIES}
                 ${FLTK_LIBRARIES}
                 ${OYRANOS_LIBRARIES}
                 ${XCM_LIBRARIES}
                 ${FREETYPE_LIBRARIES}
                 ${OPENGL_LIBRARIES}
                 ${X11_Xinerama_LIB}
                 ${X11_Xpm_LIB}
                 ${X11_Xrandr_LIB}
                 ${X11_Xxf86vm_LIB}
                 ${CMAKE_THREAD_LIBS_INIT} )
  INSTALL( TARGETS icc_examin_cp DESTINATION ${CINEPAINT_INSTALL_PATH} )
ENDIF()

# i18n
IF( GETTEXT_FOUND )
  SET(_potFile ${CMAKE_SOURCE_DIR}/po/${PACKAGE_NAME}.pot)
  #SET( _xgettext_option_list --language=C --keyword=_ --keyword=N_ --keyword=C_:1c,2 --keyword=NC_:1c,2 -s --package-name=${PROJECT_NAME} --package-version=${OYRANOS_VERSION_MAJOR}.${OYRANOS_VERSION_MINOR} )
  SET( _xgettext_option_list --add-comments --keyword=gettext --flag=gettext:1:pass-c-format --keyword=_ --flag=_:1:pass-c-format --keyword=N_ --flag=N_:1:pass-c-format  --copyright-holder='Kai-Uwe Behrmann'  --msgid-bugs-address='ku.b@gmx.de' --from-code=utf-8 --package-name=${PROJECT_NAME} --package-version=${OYRANOS_VERSION_MAJOR}.${OYRANOS_VERSION_MINOR} )
  SET( XGETTEXT_CMD xgettext )
  FILE( GLOB SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        *.c
        *.cpp
        *.cxx
        *.h
      )

  ADD_CUSTOM_COMMAND(OUTPUT ${_potFile}
    COMMAND ${XGETTEXT_CMD} ${_xgettext_option_list} -o ${_potFile} ${SRC_FILES}
    DEPENDS ${SRC_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Extract translatable messages to ${_potFile}"
  )

  SET( LINGUAS po/cs.po po/de.po po/eo.po po/eu.po po/fr.po )
  FOREACH( my_lang ${LINGUAS} )
    GET_FILENAME_COMPONENT(_absFile ${my_lang} ABSOLUTE)
    GET_FILENAME_COMPONENT(_abs_PATH ${_absFile} PATH)
    GET_FILENAME_COMPONENT(_lang ${_absFile} NAME_WE)
    SET( MY_LINGUAS ${MY_LINGUAS} ${_lang} )
  ENDFOREACH( my_lang )
  # compile translations always to have them ready for installation
  GETTEXT_CREATE_TRANSLATIONS( ${_potFile} ALL ${LINGUAS} )

  # install into app bundle
  IF(APPLE)
    FOREACH( my_lang ${MY_LINGUAS} )
      INSTALL( FILES "${CMAKE_BINARY_DIR}/${my_lang}.gmo"
               DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/${ICCE_APP}.app/Contents/Resources/locale/${my_lang}/LC_MESSAGES"
               RENAME icc-examin.mo )
    ENDFOREACH( my_lang )
  ENDIF(APPLE)
ENDIF()

