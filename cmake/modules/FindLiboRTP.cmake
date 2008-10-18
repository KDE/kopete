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
  
  EXECUTE_PROCESS(COMMAND gcc -o ortpversion -lortp -L${LIBORTP_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/cmake/modules/ortpversion.c RESULT_VARIABLE result)
  if (NOT result)
    EXECUTE_PROCESS(COMMAND ./ortpversion 0 13 RESULT_VARIABLE result)
  endif(NOT result)
  if (NOT result)
#   Version OK
    SET( LIBORTP_FOUND TRUE)
    MESSAGE( STATUS "Found libortp: ${LIBORTP_LIBRARY}")
  elseif(result)
#   Version non OK
    MESSAGE( STATUS "You need ortp version 0.13 minimum !")
  endif(NOT result)
ENDIF( LIBORTP_INCLUDE_DIR AND LIBORTP_LIBRARY )
