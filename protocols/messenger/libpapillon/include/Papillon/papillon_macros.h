/*
   papillon_macros.h - List of macros used across libpapillon

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#ifndef PAPILLON_MACROS_H
#define PAPILLON_MACROS_H

#include <QtGlobal>

/**
 * @file
 * @brief Macros used by libpapillon
 */

/**
 * @def PAPILLON_EXPORT
 * @brief PAPILLON_EXPORT macro
 *
 * PAPILLON_EXPORT is to set the visibility of the library.
 * Use the EXPORT macro from Qt because I didn't want to test for GCC's visibility support myself or
 * test for WINDOWS too.
 */

#ifndef PAPILLON_EXPORT
# if defined(Q_OS_WIN)
#  if defined(MAKE_PAPILLON_KOPETE_LIB)
#   define PAPILLON_EXPORT Q_DECL_EXPORT
#  else
#   define PAPILLON_EXPORT Q_DECL_IMPORT
#  endif
# else
#  define PAPILLON_EXPORT Q_DECL_EXPORT
# endif
#endif

#define PAPILLON_VERSION_STRING "0.0.2"
#define PAPILLON_VERSION_MAJOR 0
#define PAPILLON_VERSION_MINOR 0
#define PAPILLON_VERSION_RELEASE 2
#define PAPILLON_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

/**
 * @brief PAPILLON_VERSION macro
 *
 * Return a special integer which represent the current version of libpapillon.
 */
#define PAPILLON_VERSION \
  PAPILLON_MAKE_VERSION(PAPILLON_VERSION_MAJOR,PAPILLON_VERSION_MINOR,PAPILLON_VERSION_RELEASE)

/**
 * @def PAPILLON_FUNCINFO
 * @brief PAPILLON_FUNCINFO macro
 *
 * An indicator of where you are in a source file, to be used in
 * warnings (perhaps debug messages too).
 *
 * Extra pretty with GNU C.
 */
#ifdef __GNUC__
#define PAPILLON_FUNCINFO "[" << __PRETTY_FUNCTION__ << "]"
#else
#define PAPILLON_FUNCINFO "[" << __FILE__ << ":" << __LINE__ << "]"
#endif

/**
 * @brief PAPILLON_IS_VERSION(major,minor,revision) macro
 *
 * Simple test for current version of libpapillon.
 */
#define PAPILLON_IS_VERSION(a,b,c) ( PAPILLON_VERSION >= PAPILLON_MAKE_VERSION(a,b,c) )

#endif
