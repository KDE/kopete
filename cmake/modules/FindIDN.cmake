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
INCLUDE(UsePkgConfig)

PKGCONFIG(libidn _IDNIncDir _IDNLinkDir _IDNLinkFlags _IDNCflags)
ENDIF (NOT WIN32)

FIND_PATH(IDN_INCLUDE_DIR idna.h
  PATHS
  ${_IDNIncDir}
  NO_DEFAULT_PATH
)

set(IDN_DEFINITIONS ${_IDNCflags})
set(IDN_INCLUDES ${IDN_INCLUDE_DIR} )

FIND_LIBRARY(IDN_LIBRARY NAMES idn
  PATHS
  ${_IDNLinkDir} 
  NO_DEFAULT_PATH 
)
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(IDN DEFAULT_MSG IDN_INCLUDES AND IDN_LIBRARY )

MARK_AS_ADVANCED(IDN_INCLUDE_DIR IDN_LIBRARY)

