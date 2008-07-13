/*
    kopetemetacontactlvi.cpp - Kopete Meta Contact K3ListViewItem

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2002-2004 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2002      by Duncan Mac-Vicar P     <duncan@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "kopetemetacontactlvi.h"
#include "kopetemetacontact.h"

KopeteMetaContactViewItem::KopeteMetaContactViewItem( Kopete::MetaContact* contact )
: QObject(0), QStandardItem()
{
	m_metaContact = contact;
}

KopeteMetaContactViewItem::~KopeteMetaContactViewItem()
{
}

Kopete::MetaContact* KopeteMetaContactViewItem::metaContact() const
{
	return m_metaContact;
}

#include "kopetemetacontactlvi.moc"

// vim: set noet ts=4 sts=4 sw=4:
