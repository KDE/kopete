/*
    kircentity.cpp - IRC Client

    Copyright (c) 2004      by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "kircentity.h"

const QRegExp KIRCEntity::sm_userRegExp(QString::fromLatin1("(.*)(?:!(.*))(?:@(.*))"));
const QRegExp KIRCEntity::sm_channelRegExp( QString::fromLatin1("^[#!+&][^\\s,:]+$") );

#include "kircentity.moc"
