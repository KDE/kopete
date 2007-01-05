# cmake macro to test IDN library

# Copyright (c) 2006, Will Stephenson <wstephenson@kde.org>
#
#  IDN_FOUND - Test has found Decibel dependencies
#  IDN_INCLUDES - Include needed for Decibel
#  IDN_LIBRARIES - Libraries needed for Decibel
#  IDN_DEFINITIONS - Compiler swithces required for using Decibel
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

INCLUDE(UsePkgConfig)

PKGCONFIG(libidn _IDNIncDir _IDNLinkDir _IDNLinkFlags _IDNCflags)

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

if (IDN_INCLUDES AND IDN_LIBRARY)
   set(IDN_FOUND TRUE)
endif (IDN_INCLUDES AND IDN_LIBRARY)

if (IDN_FOUND)
  if (NOT IDN_FIND_QUIETLY)
    message(STATUS "Found IDN: ${IDN_LIBRARY}")
  endif (NOT IDN_FIND_QUIETLY)
else (IDN_FOUND)
  if (IDN_FIND_REQUIRED)
    message(SEND_ERROR "Could NOT find IDN")
  endif (IDN_FIND_REQUIRED)
endif (IDN_FOUND)

MARK_AS_ADVANCED(IDN_INCLUDE_DIR IDN_LIBRARY)

