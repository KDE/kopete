/*
    kircconst.h - The KIRC constants & enums.

    Copyright (c) 2005-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2005-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCCONST_H
#define KIRCCONST_H

#include <QRegExp>

class QTextCodec;

/**
 * @author Michel Hermier <michel.hermier@gmail.com>
 */
namespace KIrc
{

// Static regular expressions
extern const QRegExp sm_RemoveLinefeeds;

// Static URL query attributes.
extern const QString URL_NICKNAME;
extern const QString URL_REALNAME;

// Static codecs
extern QTextCodec *UTF8;

}

#endif

