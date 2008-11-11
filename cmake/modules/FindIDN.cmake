# cmake macro to test IDN library

# Copyright (c) 2006, Will Stephenson <wstephenson@kde.org>
#
#  IDN_FOUND - Test has found IDN dependencies
#  IDN_INCLUDES - Include needed for IDN
#  IDN_LIBRARY - Libraries needed for IDN
#  IDN_DEFINITIONS - Compiler swithces required for using IDN
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
IF (NOT WIN32)
  find_package(PkgConfig)
  pkg_check_modules(LIBIDN libidn)

  FIND_PATH(IDN_INCLUDE_DIR idna.h
    PATHS
    ${LIBIDN_INCLUDE_DIRS}
    NO_DEFAULT_PATH
  )

  set(IDN_DEFINITIONS ${LIBIDN_CFLAGS})

  FIND_LIBRARY(IDN_LIBRARY NAMES idn
    PATHS
    ${LIBIDN_LIBRARY_DIRS} 
    NO_DEFAULT_PATH 
  )
ELSE (NOT WIN32)
  FIND_PATH(IDN_INCLUDE_DIR idna.h)
  FIND_LIBRARY(IDN_LIBRARY NAMES idn idn-11 libidn-11)
ENDIF (NOT WIN32)
set(IDN_INCLUDES ${IDN_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(IDN DEFAULT_MSG IDN_INCLUDES IDN_LIBRARY )

MARK_AS_ADVANCED(IDN_INCLUDE_DIR IDN_LIBRARY)

