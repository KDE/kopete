/*
    Kopete Export macors

    Copyright (c) 2004 by Dirk Mueller <mueller@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_EXPORT_H
#define KOPETE_EXPORT_H

#include <kdemacros.h>

#ifndef KOPETE_EXPORT
# ifdef MAKE_KOPETE_LIB
#  define KOPETE_EXPORT  KDE_EXPORT
# else
#  define KOPETE_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KOPETEPRIVACY_EXPORT
# ifdef MAKE_KOPETEPRIVACY_LIB
#  define KOPETEPRIVACY_EXPORT KDE_EXPORT
# else
#  define KOPETEPRIVACY_EXPORT KDE_IMPORT
# endif
#endif


#ifndef KOPETE_CONTACT_LIST_EXPORT_SHARED_EXPORT
# if defined(MAKE_KOPETECONTACTLIST_LIB)
#  define KOPETE_CONTACT_LIST_EXPORT KDE_EXPORT
# else
#  define KOPETE_CONTACT_LIST_EXPORT KDE_IMPORT
# endif
#endif

#ifndef YAHOO_EXPORT
# if defined(MAKE_KOPETE_YAHOO_LIB)
#  define YAHOO_EXPORT KDE_EXPORT
# else
#  define YAHOO_EXPORT KDE_IMPORT
# endif
#endif

#ifndef QQ_EXPORT
# if defined(MAKE_KOPETE_QQ_LIB)
#  define QQ_EXPORT KDE_EXPORT
# else
#  define QQ_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KOPETE_OTR_SHARED_EXPORT
# if defined(MAKE_KOPETE_OTR_SHARED_LIB)
#  define KOPETE_OTR_SHARED_EXPORT KDE_EXPORT
# else
#  define KOPETE_OTR_SHARED_EXPORT KDE_IMPORT
# endif
#endif

#endif
