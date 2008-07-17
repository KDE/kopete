# cmake macro to test LiboRTP

# Copyright (c) 2008, Detlev Casanova <detlev.casanova @ gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

#INCLUDE(MacroEnsureVersion)
MESSAGE( "Looking for LiboRTP" )
INCLUDE(FindPackageHandleStandardArgs)

FIND_PATH(LIBORTP_INCLUDE_DIR ortp/ortp.h)

FIND_LIBRARY(LIBORTP_LIBRARY NAMES ortp)

IF( LIBORTP_INCLUDE_DIR AND LIBORTP_LIBRARY )
  SET( LIBORTP_FOUND TRUE)
  MESSAGE( STATUS "Found libortp: ${LIBORTP_LIBRARY}")
ENDIF( LIBORTP_INCLUDE_DIR AND LIBORTP_LIBRARY )
