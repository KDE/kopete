# - Try to find Decibel packages (QtTapioca, QtTelepathy)
# Once done this will define
#
#  DECIBEL_FOUND - Test has found Decibel dependencies
#  DECIBEL_INCLUDES - Include needed for Decibel
#  DECIBEL_LIBRARIES - Libraries needed for Decibel
#  DECIBEL_DEFINITIONS - Compiler swithces required for using Decibel
#
#  QT_TELEPATHY_INCLUDE_DIR - Include needed for QtTelepathy
#  QT_TELEPATHY_LIBRARY - Library for QtTelepathy
#  QT_TAPIOCA_INCLUDE_DIR - Included need for QtTapioca
#  QT_TAPIOCA_LIBRARY - Library for QtTapioca
#
# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
#
# Copyright (c) 2006, Michaël Larouche, <larouche@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (NOT WIN32)
find_package(PkgConfig)
pkg_check_modules(QTTAPIOCA QtTapioca)

ENDIF(NOT WIN32)

set(QT_TAPIOCA_INCLUDE_DIR ${QTTAPIOCA_INCLUDE_DIRS})

set(DECIBEL_DEFINITIONS ${QTTAPIOCA_CFLAGS})
set(DECIBEL_INCLUDES ${QT_TAPIOCA_INCLUDE_DIR} )

FIND_LIBRARY(QT_TAPIOCA_LIBRARY NAMES QtTapioca
  PATHS
  ${QTTAPIOCA_LIBRARY_DIRS} 
  NO_DEFAULT_PATH 
)

set(DECIBEL_LIBRARIES ${QT_TAPIOCA_LIBRARY})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Decibel DEFAULT_MSG DECIBEL_INCLUDES DECIBEL_LIBRARIES )

MARK_AS_ADVANCED(QT_TAPIOCA_INCLUDE_DIR QT_TAPIOCA_LIBRARY)
