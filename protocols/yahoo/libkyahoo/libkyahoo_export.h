#ifndef LIBKYAHOO_EXPORT_H
#define LIBKYAHOO_EXPORT_H


#include <kdemacros.h>

#if defined Q_OS_WIN

#ifndef LIBKYAHOO_EXPORT
# ifdef MAKE_KYAHOO_LIB
#  define LIBKYAHOO_EXPORT KDE_EXPORT
# else
#  define LIBKYAHOO_EXPORT KDE_IMPORT
# endif
#endif

#else
#define LIBKYAHOO_EXPORT KDE_EXPORT
#endif

#endif

