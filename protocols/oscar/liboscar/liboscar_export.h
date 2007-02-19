/* LGPL licensed. Copyright (c) 2006 by Matt Rogers */

#ifndef LIBOSCAR_EXPORT_H
#define LIBOSCAR_EXPORT_H

#include <kdemacros.h>

#if defined Q_OS_WIN

#ifndef LIBOSCAR_EXPORT
# ifdef MAKE_OSCAR_LIB
#  define LIBOSCAR_EXPORT KDE_EXPORT
# else
#  define LIBOSCAR_EXPORT KDE_IMPORT
# endif
#endif

#else
#define LIBOSCAR_EXPORT KDE_EXPORT
#endif

#endif

