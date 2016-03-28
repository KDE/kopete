## cmake macro to test LibOTR

# Copyright (c) 2008-2013, Michael Zanetti <mzanetti @ kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

INCLUDE(FindPackageHandleStandardArgs)

IF (LIBOTR_INCLUDE_DIR AND LIBOTR_LIBRARY)
    # Already in cache, be silent
    SET(LIBOTR_FIND_QUIETLY TRUE)
ENDIF (LIBOTR_INCLUDE_DIR AND LIBOTR_LIBRARY)

FIND_PATH(LIBOTR_INCLUDE_DIR libotr/version.h)

FIND_LIBRARY(LIBOTR_LIBRARY NAMES otr libotr)

# Determine version information from libotr/version.h
IF( LIBOTR_INCLUDE_DIR AND LIBOTR_LIBRARY )
  EXECUTE_PROCESS(COMMAND grep "OTRL_VERSION" "${LIBOTR_INCLUDE_DIR}/libotr/version.h" OUTPUT_VARIABLE output)
  STRING(REGEX MATCH "OTRL_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+" LIBOTR_VERSION "${output}")
  STRING(REGEX REPLACE "^OTRL_VERSION \"" "" LIBOTR_VERSION "${LIBOTR_VERSION}")
  # Check if version is at least 4.0.0
  IF( LIBOTR_VERSION VERSION_GREATER "4.0.0" )
    SET( LIBOTR_FOUND TRUE )
  ENDIF ( LIBOTR_VERSION VERSION_GREATER "4.0.0" )

  IF( LIBOTR_FOUND )
    IF( NOT LIBOTR_FIND_QUIETLY )
      MESSAGE( STATUS "Found libotr: ${LIBOTR_LIBRARY} (version ${LIBOTR_VERSION})")
    ENDIF( NOT LIBOTR_FIND_QUIETLY )
  ELSE( LIBOTR_FOUND )
    MESSAGE(STATUS "libotr version 4.0.0 or greater required but found ${LIBOTR_VERSION}.")
  ENDIF( LIBOTR_FOUND )

ENDIF( LIBOTR_INCLUDE_DIR AND LIBOTR_LIBRARY )
