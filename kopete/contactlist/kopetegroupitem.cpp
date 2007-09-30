/*
    kopeteonlinestatus.cpp - Kopete Online Status

    Copyright (c) 2002-2004 by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetegroupitem.h"
#include "kopeteitembase.h"
#include <QtCore/QString>

#include <KIcon>
#include <KLocale>

#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopeteappearancesettings.h"

KopeteGroupItem::KopeteGroupItem( Kopete::Group *group )
 : QObject(0), QStandardItem()
{
    setData( Kopete::Items::Group, Kopete::Items::TypeRole );
	setGroup( group );
}

KopeteGroupItem::~KopeteGroupItem()
{
	m_group = 0;
}

void KopeteGroupItem::setGroup( Kopete::Group* group )
{
	m_group = group;
	refreshDisplayName();
	setIcon( KIcon(KOPETE_GROUP_DEFAULT_CLOSED_ICON) );
}

Kopete::Group* KopeteGroupItem::group() const
{
	return m_group;
}

void KopeteGroupItem::groupNameChanged( Kopete::Group* group, const QString& )
{
	if ( group == m_group )
		refreshDisplayName();
}

void KopeteGroupItem::refreshDisplayName()
{
	m_totalMemberCount = 0;
	m_onlineMemberCount = 0;

	foreach ( Kopete::MetaContact *m, m_group->members())
	{
		m_totalMemberCount++;
		if ( m->isOnline() )
			m_onlineMemberCount++;
	}

	QString nameAndCount = i18nc( "GROUP NAME (NUMBER OF ONLINE CONTACTS/NUMBER OF CONTACTS IN GROUP)", "%1 (%2/%3)",
				m_group->displayName(), m_onlineMemberCount, m_totalMemberCount );
	setText(nameAndCount);
}

bool KopeteGroupItem::shouldBeVisible() const
{
	int visibleUsers = m_onlineMemberCount;
	if ( Kopete::AppearanceSettings::self()->showOfflineUsers() )
		visibleUsers = m_totalMemberCount;

	return Kopete::AppearanceSettings::self()->showEmptyGroups() || ( visibleUsers > 0 );
}

#include "kopetegroupitem.moc"
// vim: set noet ts=4 sts=4 sw=4:
