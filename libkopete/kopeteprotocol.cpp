/*
    kopeteprotocol.cpp - Kopete Protocol

	Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
	Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteprotocol.h"
#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopete.h"

KopeteProtocol::KopeteProtocol(QObject *parent, const char *name)
    : Plugin( parent, name )
{
	m_canStream = false;
}

KopeteProtocol::~KopeteProtocol()
{
}

QString KopeteProtocol::icon() const
{
	return m_icon;
}

void KopeteProtocol::setIcon( const QString &icon )
{
	m_icon = icon;
}

// vim: set noet ts=4 sts=4 sw=4:

