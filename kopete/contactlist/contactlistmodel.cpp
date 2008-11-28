/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Aleix Pol              <aleixpol@gmail.com>
    Copyright (c) 2008      by Matt Rogers            <mattr@kde.org>

    Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "contactlistmodel.h"

#include <QStandardItem>
#include <QList>
#include <QUuid>
#include <QImage>

#include <qimageblitz.h>

#include <KIcon>
#include <KDebug>
#include <KLocale>
#include <KIconLoader>

#include "kopetegroup.h"
#include "kopetepicture.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopeteitembase.h"
#include "kopeteappearancesettings.h"

namespace Kopete {

namespace UI {

ContactListModel::ContactListModel( QObject* parent )
 : QAbstractItemModel( parent )
{
	Kopete::ContactList* kcl = Kopete::ContactList::self();
	connect( kcl, SIGNAL( metaContactAdded( Kopete::MetaContact* ) ),
	         this, SLOT( addMetaContact( Kopete::MetaContact* ) ) );
	connect( kcl, SIGNAL( groupAdded( Kopete::Group* ) ),
	         this, SLOT( addGroup( Kopete::Group* ) ) );
	
}


ContactListModel::~ContactListModel()
{
}

void ContactListModel::addMetaContact( Kopete::MetaContact* contact )
{
	foreach( Kopete::Group* g, contact->groups() )
		m_contacts[g].append(contact);
	
	connect( contact,
	         SIGNAL(onlineStatusChanged(Kopete::MetaContact*, Kopete::OnlineStatus::StatusType)),
	         this, SLOT(resetModel()));
}

void ContactListModel::removeMetaContact( Kopete::MetaContact* contact )
{
	disconnect( contact,
	            SIGNAL(onlineStatusChanged(Kopete::MetaContact*, Kopete::OnlineStatus::StatusType)),
				this, SLOT(resetModel()) );
}

void ContactListModel::addGroup( Kopete::Group* group )
{
	kDebug(14001) << "addGroup" << group->displayName();
	beginInsertRows( QModelIndex(), rowCount(), rowCount() + 1 );
	m_groups.append( group );
	m_contacts[group] = QList<Kopete::MetaContact*>();
	endInsertRows();
}

void ContactListModel::removeGroup( Kopete::Group* group )
{
	int pos = m_groups.indexOf( group );
	beginRemoveRows( QModelIndex(), pos, pos );
	m_groups.removeAt( m_groups.indexOf( group ) );
	m_contacts.remove( group );
	endRemoveRows();
}

int ContactListModel::childCount( const QModelIndex& parent ) const
{
	int cnt = 0;
	if ( !parent.isValid() )
	{ //Number of groups
		cnt = m_groups.count();
	}
	else
	{
		Kopete::ContactListElement *cle = static_cast<Kopete::ContactListElement*>( parent.internalPointer() );
		Kopete::Group *g = dynamic_cast<Kopete::Group*>( cle );
		if (g)
			cnt = m_contacts[g].count();
	}
	
	return cnt;
}

int ContactListModel::rowCount( const QModelIndex& parent ) const
{
	Kopete::ContactListElement *cle = static_cast<Kopete::ContactListElement*>( parent.internalPointer() );
	Kopete::Group *g = dynamic_cast<Kopete::Group*>( cle );
	
	int cnt = 0;
	if ( !parent.isValid() )
	{ //Number of groups and contacts
		cnt = m_groups.count();
		foreach( QList<Kopete::MetaContact*> l, m_contacts.values() )
			cnt += l.count();
	}
	else
	{
		if ( g )
			cnt+= m_contacts[g].count();
	}
	
	return cnt;
}

bool ContactListModel::hasChildren( const QModelIndex& parent ) const
{
	Kopete::ContactListElement *cle = static_cast<Kopete::ContactListElement*>( parent.internalPointer() );
	Kopete::Group *g = dynamic_cast<Kopete::Group*>( cle );
	
	bool res = false;
	if ( !parent.isValid() )
		res=!m_groups.isEmpty();
	else
	{
		if ( g )
		{
			int row = parent.row();
			Kopete::Group *g = m_groups[row];
			res = !m_contacts[g].isEmpty();
		}
	}
	return res;
}

QModelIndex ContactListModel::index( int row, int column, const QModelIndex & parent ) const
{
	if ( row < 0 || row >= childCount( parent ) )
	{
		return QModelIndex();
	}

	Kopete::ContactListElement *cle = static_cast<Kopete::ContactListElement*>( parent.internalPointer() );
	Kopete::Group *g = dynamic_cast<Kopete::Group*>(cle);
	
	QModelIndex idx;
	Kopete::ContactListElement *itemPtr=0;
	if( !parent.isValid() )
		itemPtr = m_groups[row];
	else
	{
		if ( g )
			itemPtr = m_contacts[g][row];
	}
	idx = createIndex( row, column, itemPtr );
	return idx;
}

int ContactListModel::countConnected(Kopete::Group* g) const
{
	int onlineCount = 0;
	QList<Kopete::MetaContact*> metaContactList = m_contacts.value(g);
	QList<Kopete::MetaContact*>::const_iterator it, itEnd;
	itEnd = metaContactList.constEnd();
	for (it = metaContactList.constBegin(); it != itEnd; ++it)
	{
	  if ( (*it)->isOnline() )
		onlineCount++;
	}
	
	return onlineCount;
}

QVariant ContactListModel::data ( const QModelIndex & index, int role ) const
{
	if ( !index.isValid() )
		return QVariant();
	
	using namespace Kopete;
	
	/* do all the casting up front. I need to profile to see how expensive this is though */
	ContactListElement *cle = static_cast<ContactListElement*>( index.internalPointer() );
	Group *g = qobject_cast<Group*>( cle );
	MetaContact *mc = qobject_cast<MetaContact*>( cle );

	QString display;
	QImage img;
	if ( g )
	{
		switch ( role )
		{
		case Qt::DisplayRole:
			display = i18n( "%1 (%2/%3)", g->displayName(), countConnected( g ),
			                m_contacts[g].count() );
			return display;
			break;
		case Qt::DecorationRole:
			if ( g->isExpanded() )
			{
				if ( g->useCustomIcon() )
					return g->icon();
				else
					return KIcon( KOPETE_GROUP_DEFAULT_OPEN_ICON );
			}
			else
			{
				if ( g->useCustomIcon() )
					return g->icon();
				else
					return KIcon( KOPETE_GROUP_DEFAULT_CLOSED_ICON );
			}
			break;
		case Kopete::Items::TypeRole:
			return Kopete::Items::Group;
			break;
		case Kopete::Items::UuidRole:
			return QUuid().toString();
			break;
		case Kopete::Items::TotalCountRole:
			return g->members().count();
			break;
		case Kopete::Items::ConnectedCountRole:
			return countConnected( g );
			break;
		case Kopete::Items::OnlineStatusRole:
			return OnlineStatus::Unknown;
			break;
		}
	}

	if ( mc )
	{
		switch ( role )
		{
		case Qt::DisplayRole:
			display = mc->displayName();
			return display;
			break;
		case Qt::DecorationRole:
			return metaContactImage( mc );
			break;
		case Kopete::Items::TypeRole:
			return Kopete::Items::MetaContact;
			break;
		case Kopete::Items::UuidRole:
			return mc->metaContactId().toString();
			break;
		case Kopete::Items::OnlineStatusRole:
			return mc->status();
			break;
		}
	}

	return QVariant();
}

Qt::ItemFlags ContactListModel::flags( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return 0;
	
	return Qt::ItemIsEnabled;
}

QModelIndex ContactListModel::parent(const QModelIndex & index) const
{
	QModelIndex parent;
	
	if(index.isValid())
	{
		Kopete::ContactListElement *cle = static_cast<Kopete::ContactListElement*>( index.internalPointer() );
		Kopete::Group *g = dynamic_cast<Kopete::Group*>( cle );
		if ( !g )
		{
			Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>( cle );
			parent = createIndex( 0, 0, mc->groups().last() );
		}
	}
	return parent;
}

void ContactListModel::resetModel()
{
	reset();
}

QVariant ContactListModel::metaContactImage( Kopete::MetaContact* mc ) const
{
	using namespace Kopete;
	QImage img;
	img = mc->picture().image();
	int displayMode = AppearanceSettings::self()->contactListDisplayMode();
	int iconMode = AppearanceSettings::self()->contactListIconMode();
	int imageSize = IconSize( KIconLoader::Small );
	bool usePhoto = ( iconMode == AppearanceSettings::EnumContactListIconMode::IconPhoto );
	if ( displayMode == AppearanceSettings::EnumContactListDisplayMode::Detailed )
	{
		imageSize = ( iconMode == AppearanceSettings::EnumContactListIconMode::IconPic ?
		              KIconLoader::SizeMedium : KIconLoader::SizeLarge );
	}
	else
	{
		imageSize = ( iconMode == AppearanceSettings::EnumContactListIconMode::IconPic ?
		              IconSize( KIconLoader::Small ) : KIconLoader::SizeMedium );
	}
	
	if ( usePhoto && !img.isNull() )
	{
		img = img.scaled( imageSize, imageSize, Qt::KeepAspectRatio,
		                  Qt::SmoothTransformation );
		if ( mc->status() == Kopete::OnlineStatus::Offline )
			Blitz::grayscale(img);
		return img;
	}
	else
	{
		switch( mc->status() )
		{
		case OnlineStatus::Online:
			if( mc->useCustomIcon() )
				return SmallIcon( mc->icon( ContactListElement::Online ), imageSize );
			else
				return SmallIcon( QString::fromUtf8( "user-online" ), imageSize );
			break;
		case OnlineStatus::Away:
			if( mc->useCustomIcon() )
				return SmallIcon( mc->icon( ContactListElement::Away ), imageSize );
			else
				return SmallIcon( QString::fromUtf8( "user-away" ), imageSize );
			break;
		case OnlineStatus::Unknown:
			if( mc->useCustomIcon() )
				return SmallIcon( mc->icon( ContactListElement::Unknown ), imageSize );
			if ( mc->contacts().isEmpty() )
				return SmallIcon( QString::fromUtf8( "metacontact_unknown" ), imageSize );
			else
				return SmallIcon( QString::fromUtf8( "user-offline" ), imageSize );
			break;
		case OnlineStatus::Offline:
		default:
			if( mc->useCustomIcon() )
				return SmallIcon( mc->icon( ContactListElement::Offline ), imageSize );
			else
				return SmallIcon( QString::fromUtf8( "user-offline" ), imageSize );
			break;
		}
	}
	return img;
}


}

}

#include "contactlistmodel.moc"
//kate: tab-width 4
