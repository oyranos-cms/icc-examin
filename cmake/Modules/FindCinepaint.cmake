# - Find CinePaint
#  CINEPAINT_EXECUTABLE: the full path to the cinepainttool tool.
#  CINEPAINT_FOUND: True if CinePaint has been found.
#  CINEPAINT_VERSION_STRING: the version of CinePaint found (since CMake 2.8.8)
#  CINEPAINT_INCLUDE_DIRS: path
#  CINEPAINT_LIBRARIES: link libs
#  CINEPAINT_INSTALL_PATH: the plugin install path

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
# Copyright 2007      Alexander Neundorf <neundorf@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

FIND_PROGRAM(CINEPAINTTOOL_EXECUTABLE cinepainttool)

IF(CINEPAINTTOOL_EXECUTABLE)
   FIND_PACKAGE(PkgConfig)
   IF (PKG_CONFIG_FOUND)
     pkg_check_modules(GTK gtk+-2.0)
   ENDIF (PKG_CONFIG_FOUND)
   EXECUTE_PROCESS(COMMAND ${CINEPAINTTOOL_EXECUTABLE} --version
                  OUTPUT_VARIABLE CINEPAINT_VERSION_STRING
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
   EXECUTE_PROCESS(COMMAND ${CINEPAINTTOOL_EXECUTABLE} --install-dir
                  OUTPUT_VARIABLE CINEPAINT_INSTALL_PATH
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
   EXECUTE_PROCESS(COMMAND ${CINEPAINTTOOL_EXECUTABLE} --cflags
                  OUTPUT_VARIABLE CINEPAINT_CFLAGS
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
   EXECUTE_PROCESS(COMMAND ${CINEPAINTTOOL_EXECUTABLE} --prefix
                  OUTPUT_VARIABLE CINEPAINT_PREFIX
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
   SET( CINEPAINT_INCLUDE_DIRS "${CINEPAINT_PREFIX}/include/cinepaint;${GTK_INCLUDE_DIRS}" )

   EXECUTE_PROCESS(COMMAND ${CINEPAINTTOOL_EXECUTABLE} --libs
                  OUTPUT_VARIABLE CINEPAINT_LIBRARIES
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF(CINEPAINTTOOL_EXECUTABLE)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Cinepaint
                                  REQUIRED_VARS CINEPAINTTOOL_EXECUTABLE CINEPAINT_INSTALL_PATH CINEPAINT_INCLUDE_DIRS CINEPAINT_LIBRARIES
                                  VERSION_VAR CINEPAINT_VERSION_STRING)


IF (CINEPAINTTOOL_EXECUTABLE)
   SET(CINEPAINT_FOUND TRUE)
ELSE (CINEPAINTTOOL_EXECUTABLE)
   SET(CINEPAINT_FOUND FALSE)
   IF (Cinepaint_REQUIRED)
      MESSAGE(FATAL_ERROR "CinePaint not found")
   ENDIF (Cinepaint_REQUIRED)
ENDIF (CINEPAINTTOOL_EXECUTABLE)
