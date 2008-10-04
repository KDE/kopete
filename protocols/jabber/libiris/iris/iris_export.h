#ifndef IRIS_EXPORT_H
#define IRIS_EXPORT_H

#include <QtGlobal>

#ifndef IRIS_EXPORT
# ifdef Q_OS_WIN
#  ifdef MAKE_IRIS_KOPETE_LIB
#   define IRIS_EXPORT Q_DECL_EXPORT
#  else
#   define IRIS_EXPORT Q_DECL_IMPORT
#  endif
# else
#  define IRIS_EXPORT Q_DECL_EXPORT
# endif
#endif

#ifdef Q_OS_WIN
# define IRIS_FULL_TEMPLATE_EXPORT_INSTANTIATION
#endif

#endif
