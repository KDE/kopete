# Try to find libmeanwhile package
#
# Copyright (c) 2008, Jeremy Kerr <jk@ozlabs.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (NOT WIN32)
  INCLUDE(UsePkgConfig)

  PKGCONFIG(meanwhile
      _LibMeanwhileIncDir
      _LibMeanwhileLinkDir
      _LibMeanwhileLinkFlags
      _LibMeanwhileCflags)

  FIND_PATH(LIBMEANWHILE_INCLUDE_DIR meanwhile/mw_common.h
    PATHS
    ${_LibMeanwhileIncDir}
    NO_DEFAULT_PATH
  )

  set(LIBMEANWHILE_DEFINITIONS ${_LibMeanwhileCflags})

  FIND_LIBRARY(LIBMEANWHILE_LIBRARY NAMES meanwhile libmeanwhile
    PATHS
    ${_LibMeanwhileLinkDir}
    NO_DEFAULT_PATH
  )
ELSE (NOT WIN32)
  FIND_PATH(LIBMEANWHILE_INCLUDE_DIR meanwhile/mw_common.h)
  FIND_LIBRARY(LIBMEANWHILE_LIBRARY NAMES meanwhile libmeanwhile)
ENDIF (NOT WIN32)

set(LIBMEANWHILE_INCLUDES ${LIBMEANWHILE_INCLUDE_DIR} )

FIND_PACKAGE(GLIB2)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libmeanwhile DEFAULT_MSG
    LIBMEANWHILE_INCLUDES LIBMEANWHILE_LIBRARY)

MARK_AS_ADVANCED(LIBMEANWHILE_INCLUDE_DIR LIBMEANWHILE_LIBRARY)

