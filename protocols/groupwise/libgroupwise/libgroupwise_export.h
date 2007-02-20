#ifndef LIBGROUPWISE_EXPORT_H
#define LIBGROUPWISE_EXPORT_H


#include <kdemacros.h>

#if defined Q_OS_WIN

#ifndef LIBGROUPWISE_EXPORT
# ifdef MAKE_QGROUPWISE_LIB
#  define LIBGROUPWISE_EXPORT  KDE_EXPORT
# else
#  define LIBGROUPWISE_EXPORT KDE_IMPORT
# endif
#endif

#else
#define LIBGROUPWISE_EXPORT KDE_EXPORT
#endif

#endif

