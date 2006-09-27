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
# Copyright (c) 2006, Michaël Larouche, <michael.larouche@kdemail.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.



INCLUDE(UsePkgConfig)

PKGCONFIG(telepathy-qt _TelepathyIncDir _TelepathyLinkDir _TelepathyLinkFlags _TelepathyCflags)
PKGCONFIG(tapioca-qt _TapiocaIncDir _TapiocaLinkDir _TapiocaLinkFlags _TapiocaCflags)

set(QT_TELEPATHY_INCLUDE_DIR ${_TelepathyIncDir})
set(QT_TAPIOCA_INCLUDE_DIR ${_TapiocaIncDir})

set(DECIBEL_DEFINITIONS ${_TelepathyCflags} ${_TapiocaCflags})
set(DECIBEL_INCLUDES ${QT_TELEPATHY_INCLUDE_DIR} ${QT_TAPIOCA_INCLUDE_DIR})

FIND_LIBRARY(QT_TELEPATHY_LIBRARY NAMES QtTelepathy
  PATHS
  ${_TelepathyLinkDir} 
  NO_DEFAULT_PATH 
)

FIND_LIBRARY(QT_TAPIOCA_LIBRARY NAMES QtTapioca
  PATHS
  ${_TapiocaLinkDir} 
  NO_DEFAULT_PATH 
)

set(DECIBEL_LIBRARIES ${QT_TELEPATHY_LIBRARY} ${QT_TAPIOCA_LIBRARY})

if (DECIBEL_INCLUDES AND DECIBEL_LIBRARIES)
   set(DECIBEL_FOUND TRUE)
endif (DECIBEL_INCLUDES AND DECIBEL_LIBRARIES)

if (DECIBEL_FOUND)
  if (NOT DECIBEL_FIND_QUIETLY)
    message(STATUS "Found Decibel: ${DECIBEL_LIBRARIES}")
  endif (NOT DECIBEL_FIND_QUIETLY)
else (DECIBEL_FOUND)
  if (DECIBEL_FIND_REQUIRED)
    message(SEND_ERROR "Could NOT find Decibel")
  endif (DECIBEL_FIND_REQUIRED)
endif (DECIBEL_FOUND)

MARK_AS_ADVANCED(QT_TAPIOCA_INCLUDE_DIR QT_TAPIOCA_LIBRARY QT_TELEPATHY_INCLUDE_DIR QT_TELEPATHY_LIBRARY)
