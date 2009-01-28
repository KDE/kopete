# - try to find Kleopatra
# Once done this will define
#
#  KLEO_FOUND - system has Kleopatra
#  KLEO_INCLUDE_DIRS - Include dirs for Kleopatra
#  KLEO_LIBRARIES - Link these to use Kleopatra
#
# Copyright (c) 2009, Christophe Giboudeaux, <cgiboudeaux@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
 
if (KLEO_INCLUDE_DIR AND KLEO_LIBRARIES)

  # Already in cache
  set (KLEO_FOUND TRUE)

else (KLEO_INCLUDE_DIR AND KLEO_LIBRARIES)

  FIND_PATH(KLEO_INCLUDE_DIR
  NAMES kleo/kleo_export.h
  )

  FIND_LIBRARY(KLEO_LIBRARIES
  NAMES kleo
  )
  message ("KLEO_LIBRARIES= ${KLEO_LIBRARIES}") #Testing
  
  if(KLEO_INCLUDE_DIR AND KLEO_LIBRARIES)
    set(KLEO_FOUND TRUE)
  endif(KLEO_INCLUDE_DIR AND KLEO_LIBRARIES)

  if(KLEO_FOUND)
    message(STATUS "Found Kleopatra: ${KLEO_LIBRARIES}")
  else(KLEO_FOUND)
    if(Kleopatra_FIND_REQUIRED)
      if(NOT KLEO_INCLUDE_DIR)
        message(FATAL_ERROR "Could not find Kleopatra includes.")
      endif(NOT KLEO_INCLUDE_DIR)
      if(NOT KLEO_LIBRARIES)
        message(FATAL_ERROR "Could not find Kleopatra library.")
      endif(NOT KLEO_LIBRARIES)
    else(Kleopatra_FIND_REQUIRED)
      if(NOT KLEO_INCLUDE_DIR)
        message(STATUS "Could not find Kleopatra includes.")
      endif(NOT KLEO_INCLUDE_DIR)
      if(NOT KLEO_LIBRARIES)
        message(STATUS "Could not find Kleopatra library.")
      endif(NOT KLEO_LIBRARIES)
    endif(Kleopatra_FIND_REQUIRED)
  endif(KLEO_FOUND)

endif (KLEO_INCLUDE_DIR AND KLEO_LIBRARIES)
