# cmake macro to test LiboRTP

# Copyright (c) 2008, Detlev Casanova <detlev.casanova @ gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

INCLUDE( FindPackageHandleStandardArgs )

FIND_PATH( LIBORTP_INCLUDE_DIR ortp/ortp.h ${KDE4_INCLUDE_DIR} )

FIND_LIBRARY( LIBORTP_LIBRARY NAMES ortp )

SET( LIBORTP_FOUND FALSE )

IF( LIBORTP_INCLUDE_DIR AND LIBORTP_LIBRARY )
  EXECUTE_PROCESS( COMMAND gcc -o ortpversion -lortp -L${LIBORTP_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/cmake/modules/ortpversion.c OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE compileErr )
  if ( NOT compileErr )
    EXECUTE_PROCESS(COMMAND ./ortpversion 0 13 RESULT_VARIABLE resultErr )
    if ( NOT resultErr )
#     Version OK
      SET( LIBORTP_FOUND TRUE )
      MESSAGE( STATUS "Found libortp: ${LIBORTP_LIBRARY}" )
    endif( NOT resultErr )
  endif( NOT compileErr )
ENDIF( LIBORTP_INCLUDE_DIR AND LIBORTP_LIBRARY )

IF( resultErr OR compileErr )
  if( LiboRTP_FIND_REQUIRED )
    message( FATAL_ERROR "You need ortp version 0.13 minimum.\nPlease build and install it first." )
  else ( LiboRTP_FIND_REQUIRED )
    message( "LiboRTP not found on your system !" )
  endif ( LiboRTP_FIND_REQUIRED )
endif( resultErr OR compileErr )
