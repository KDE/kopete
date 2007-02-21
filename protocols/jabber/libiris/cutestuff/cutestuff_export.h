#ifndef CUTESTUFF_EXPORT_H
#define CUTESTUFF_EXPORT_H

#include <QtGlobal>

#ifndef CUTESTUFF_EXPORT
# ifdef Q_OS_WIN
#  ifdef MAKE_IRIS_KOPETE_LIB
#   define CUTESTUFF_EXPORT Q_DECL_EXPORT
#  else
#   define CUTESTUFF_EXPORT Q_DECL_IMPORT
#  endif
# else
#  define CUTESTUFF_EXPORT Q_DECL_EXPORT
# endif
#endif

#endif
