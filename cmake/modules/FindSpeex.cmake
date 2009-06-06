# cmake macro to test LiboRTP

# Copyright (c) 2008, Detlev Casanova <detlev.casanova @ gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

INCLUDE( FindPackageHandleStandardArgs )

FIND_PATH( SPEEX_INCLUDE_DIR  speex/speex.h ${KDE4_INCLUDE_DIR} )

FIND_LIBRARY( SPEEX_LIBRARY NAMES speex )

SET( SPEEX_FOUND FALSE )

IF( NOT SPEEX_INCLUDE_DIR OR NOT SPEEX_LIBRARY )
  if( LiboRTP_FIND_REQUIRED )
    message( FATAL_ERROR "Unable to find Speex library on your system.\nPlease build and install speex first." )
  else ( LiboRTP_FIND_REQUIRED )
    message( "Speex not found on your system !" )
  endif ( LiboRTP_FIND_REQUIRED )
ELSE( NOT SPEEX_INCLUDE_DIR OR NOT SPEEX_LIBRARY )
  message( STATUS "Found speex library : ${SPEEX_LIBRARY}")
  set( SPEEX_FOUND TRUE )
ENDIF( NOT SPEEX_INCLUDE_DIR OR NOT SPEEX_LIBRARY )

