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

#ifndef KOPETEADDACCOUNTWIZARD_EXPORT
# if defined(MAKE_KOPETEADDACCOUNTWIZARD_LIB)
#  define KOPETEADDACCOUNTWIZARD_EXPORT KDE_EXPORT
# else
#  define KOPETEADDACCOUNTWIZARD_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KOPETE_STATUSMENU_EXPORT
# if defined(MAKE_KOPETESTATUSMENU_LIB)
#  define KOPETE_STATUSMENU_EXPORT KDE_EXPORT
# else
#  define KOPETE_STATUSMENU_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KOPETE_IDENTITY_EXPORT
# if defined(MAKE_KOPETEIDENTITY_LIB)
#  define KOPETE_IDENTITY_EXPORT KDE_EXPORT
# else
#  define KOPETE_IDENTITY_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KOPETE_CONTACT_LIST_EXPORT_SHARED_EXPORT
# if defined(MAKE_KOPETECONTACTLIST_LIB)
#  define KOPETE_CONTACT_LIST_EXPORT KDE_EXPORT
# else
#  define KOPETE_CONTACT_LIST_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KOPETECHATWINDOW_SHARED_EXPORT
# if defined(MAKE_KOPETECHATWINDOW_SHARED_LIB)
#  define KOPETECHATWINDOW_SHARED_EXPORT KDE_EXPORT
# else
#  define KOPETECHATWINDOW_SHARED_EXPORT KDE_IMPORT
# endif
#endif

#ifndef CHATTEXTEDITPART_EXPORT
# if defined(MAKE_CHATTEXTEDITPART_LIB) || defined(MAKE_KOPETECHATWINDOW_SHARED_LIB)
#  define CHATTEXTEDITPART_EXPORT KDE_EXPORT
# else
#  define CHATTEXTEDITPART_EXPORT KDE_IMPORT
# endif
#endif

#ifndef OSCAR_EXPORT
# if defined(MAKE_KOPETE_OSCAR_LIB)
#  define OSCAR_EXPORT KDE_EXPORT
# else
#  define OSCAR_EXPORT KDE_IMPORT
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

#ifndef JABBER_EXPORT
# if defined(MAKE_KOPETE_JABBER_LIB)
#  define JABBER_EXPORT KDE_EXPORT
# else
#  define JABBER_EXPORT KDE_IMPORT
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
