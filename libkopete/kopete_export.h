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
#include <kdeversion.h>

#if KDE_IS_VERSION(3,3,2)
#define KOPETE_EXPORT KDE_EXPORT
#else
#define KOPETE_EXPORT
#endif

#endif
