# cmake macro to test LiboRTP

# Copyright (c) 2008, Detlev Casanova <detlev.casanova @ gmail.com>
# Copyright (c) 2008, Pino Toscano <pino@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

INCLUDE( FindPackageHandleStandardArgs )
include( FindPkgConfig )

SET( LIBORTP_FOUND FALSE )

pkg_check_modules(ortp ortp)

FIND_PATH( LIBORTP_INCLUDE_DIR
           ortp/ortp.h
           PATHS ${ortp_INCLUDEDIR} ${KDE4_INCLUDE_DIR}
)

FIND_LIBRARY( LIBORTP_LIBRARY
              NAMES ortp
              PATHS ${ortp_LIBDIR}
)

IF( LIBORTP_INCLUDE_DIR AND LIBORTP_LIBRARY )
  # we need to save and disable the C flags currently set, as they can conflict
  # with ortp's requirements (c99)
  set(save_cmake_c_flags ${CMAKE_C_FLAGS})
  set(CMAKE_C_FLAGS)
  try_run( run_result compile_result
           ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/ortpversion.c
           CMAKE_FLAGS -DINCLUDE_DIRECTORIES:STRING=${LIBORTP_INCLUDE_DIR} -DLINK_LIBRARIES=${LIBORTP_LIBRARY}
           COMPILE_DEFINITIONS "${ortp_CFLAGS}"
           ARGS "0 13"
  )
  set(CMAKE_C_FLAGS ${save_cmake_c_flags})

  if ( compile_result )
    if ( run_result EQUAL 0 )
#     Version OK
      SET( LIBORTP_FOUND TRUE )
      MESSAGE( STATUS "Found libortp: ${LIBORTP_LIBRARY}" )
    endif ( run_result EQUAL 0 )
  endif( compile_result )
ENDIF( LIBORTP_INCLUDE_DIR AND LIBORTP_LIBRARY )

if( NOT LIBORTP_FOUND )
  if( LiboRTP_FIND_REQUIRED )
    message( FATAL_ERROR "You need ortp, minimum version 0.13." )
  endif ( LiboRTP_FIND_REQUIRED )
endif( NOT LIBORTP_FOUND )
