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
#include "kopetemetacontactitem.h"
#include "kopeteitembase.h"
#include <K3Icon>
#include <KDebug>
#include <KIconLoader>

#include "kopetemetacontact.h"
#include "kopetepicture.h"
#include "kopeteonlinestatus.h"

KopeteMetaContactItem::KopeteMetaContactItem( Kopete::MetaContact* contact )
: QObject(0), QStandardItem()
{
    setData( Kopete::Items::MetaContact, Kopete::Items::TypeRole );
    
	m_metaContact = contact;
    setData( m_metaContact, Kopete::Items::ElementRole );
	setText( m_metaContact->displayName() );
    setData( m_metaContact->picture().image(), Qt::DecorationRole );
	setData( m_metaContact->idleTime(), Kopete::Items::IdleTimeRole );
    setData( m_metaContact->metaContactId().toString(), Kopete::Items::UuidRole );

	connect( m_metaContact,
	         SIGNAL( displayNameChanged( const QString&, const QString& ) ),
	         this, SLOT( changeDisplayName( const QString&, const QString ) ) );
	connect( m_metaContact, SIGNAL( photoChanged() ),
	         this, SLOT( changePhoto() ) );

}

KopeteMetaContactItem::~KopeteMetaContactItem()
{
	m_metaContact = 0;
}

Kopete::MetaContact* KopeteMetaContactItem::metaContact() const
{
	return m_metaContact;
}

void KopeteMetaContactItem::changeDisplayName( const QString&,
                                               const QString& newName )
{
	if ( !newName.isEmpty() )
		setText( newName );
}

void KopeteMetaContactItem::changePhoto()
{
    QImage img = m_metaContact->picture().image();
    if ( img.isNull() )
    {
        /* load the default metacontact icon instead */
        setData( SmallIcon( m_metaContact->statusIcon(), Qt::DecorationRole ) );
    }
    else
    {
        setData( m_metaContact->picture().image(), Qt::DecorationRole );
    }
}

void KopeteMetaContactItem::updateOnlineStatus( Kopete::MetaContact* metaContact,
                                                Kopete::OnlineStatus::StatusType status )
{
	using namespace Kopete::Items;
	setData( status, OnlineStatusRole );
	setData( metaContact->idleTime(), IdleTimeRole );
}

#include "kopetemetacontactlvi.moc"

// vim: set noet ts=4 sts=4 sw=4:
