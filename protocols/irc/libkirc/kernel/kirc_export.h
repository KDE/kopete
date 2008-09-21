/*
    libkirc export macors

    Copyright (c) 2007-2008 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2007-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRC_EXPORT_H
#define KIRC_EXPORT_H

#include <kdemacros.h>

#if defined Q_OS_WIN

#ifndef KIRC_EXPORT
# ifdef MAKE_KIRC_LIB
#  define KIRC_EXPORT KDE_EXPORT
# else
#  define KIRC_EXPORT KDE_IMPORT
# endif
#endif

#ifndef KIRCCLIENT_EXPORT
# ifdef MAKE_KIRC_CLIENT_LIB
#  define KIRCCLIENT_EXPORT KDE_EXPORT
# else
#  define KIRCCLIENT_EXPORT KDE_IMPORT
# endif
#endif

#else

#define KIRC_EXPORT KDE_EXPORT
#define KIRCCLIENT_EXPORT KDE_EXPORT

#endif

#endif // KIRC_EXPORT_H
