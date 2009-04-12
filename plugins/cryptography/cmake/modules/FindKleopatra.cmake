# - try to find Kleopatra
# Once done this will define
#
#  KLEOPATRA_FOUND - system has Kleopatra
#  KLEOPATRA_INCLUDE_DIRS - Include dirs for Kleopatra
#  KLEOPATRA_LIBRARIES - Link these to use Kleopatra
#
# Copyright (c) 2009, Christophe Giboudeaux, <cgiboudeaux@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
 
if (KLEOPATRA_INCLUDE_DIR AND KLEOPATRA_LIBRARIES)

  # Already in cache
  set (KLEOPATRA_FOUND TRUE)

else (KLEOPATRA_INCLUDE_DIR AND KLEOPATRA_LIBRARIES)

  FIND_PATH(KLEOPATRA_INCLUDE_DIR
  NAMES kleo/kleo_export.h
  )
  
  FIND_LIBRARY(KLEOPATRA_LIBRARIES
  NAMES kleo
  )
    
  if(KLEOPATRA_INCLUDE_DIR AND KLEOPATRA_LIBRARIES)
    set(KLEOPATRA_FOUND TRUE)
  endif(KLEOPATRA_INCLUDE_DIR AND KLEOPATRA_LIBRARIES)

  if(KLEOPATRA_FOUND)
    message(STATUS "Found Kleopatra: ${KLEOPATRA_LIBRARIES}")
  else(KLEOPATRA_FOUND)
    if(Kleopatra_FIND_REQUIRED)
      if(NOT KLEOPATRA_INCLUDE_DIR)
        message(FATAL_ERROR "Could not find Kleopatra includes.")
      endif(NOT KLEOPATRA_INCLUDE_DIR)
      if(NOT KLEOPATRA_LIBRARIES)
        message(FATAL_ERROR "Could not find Kleopatra library.")
      endif(NOT KLEOPATRA_LIBRARIES)
    else(Kleopatra_FIND_REQUIRED)
      if(NOT KLEOPATRA_INCLUDE_DIR)
        message(STATUS "Could not find Kleopatra includes.")
      endif(NOT KLEOPATRA_INCLUDE_DIR)
      if(NOT KLEOPATRA_LIBRARIES)
        message(STATUS "Could not find Kleopatra library.")
      endif(NOT KLEOPATRA_LIBRARIES)
    endif(Kleopatra_FIND_REQUIRED)
  endif(KLEOPATRA_FOUND)

endif (KLEOPATRA_INCLUDE_DIR AND KLEOPATRA_LIBRARIES)
