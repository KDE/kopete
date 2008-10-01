/*
    Kopete Export macors

    Copyright (c) 2007      by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2007      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCGLOBAL_H
#define KIRCGLOBAL_H

#include <kdemacros.h>

#include <QtCore/QByteArray>
#include <QtCore/QList>

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

namespace KIrc
{

typedef struct
{
	QByteArray value;
} OptArg;

static inline
KIrc::OptArg optArg(const QByteArray &arg)
{ KIrc::OptArg r; r.value = arg; return r; }

// The isNull test is intented.
static inline
QList<QByteArray> &operator << (QList<QByteArray> &list, const KIrc::OptArg &optArg)
{ if (!optArg.value.isNull()) list.append(optArg.value); return list; }

};

#endif
