/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Aleix Pol              <aleixpol@gmail.com>
    Copyright (c) 2008      by Matt Rogers            <mattr@kde.org>
    Copyright (c) 2009      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright     2009      by Roman Jarosz           <kedgedev@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers  <kopete-devel@kde.org>

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
#include <QMimeData>

#include <qimageblitz.h>

#include <KIcon>
#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KEmoticonsTheme>
#include <QTextDocument>

#include "kopeteaccount.h"
#include "kopetegroup.h"
#include "kopetepicture.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopeteitembase.h"
#include "kopeteappearancesettings.h"
#include "kopeteemoticons.h"

namespace Kopete {

namespace UI {

ContactListModel::ContactListModel( QObject* parent )
 : QAbstractItemModel( parent )
{
	Kopete::ContactList* kcl = Kopete::ContactList::self();

	// MetaContact related
	connect( kcl, SIGNAL( metaContactAdded( Kopete::MetaContact* ) ),
	         this, SLOT( addMetaContact( Kopete::MetaContact* ) ) );
	connect( kcl, SIGNAL( metaContactRemoved( Kopete::MetaContact* ) ),
	         this, SLOT( removeMetaContact( Kopete::MetaContact* ) ) );

	// Group related
	connect( kcl, SIGNAL( groupAdded( Kopete::Group* ) ),
	         this, SLOT( addGroup( Kopete::Group* ) ) );
	connect( kcl, SIGNAL( groupRemoved( Kopete::Group* ) ),
	         this, SLOT( removeGroup( Kopete::Group* ) ) );

	// MetaContact and Group related
	connect( kcl, SIGNAL(metaContactAddedToGroup(Kopete::MetaContact*, Kopete::Group*)),
	         this, SLOT(addMetaContactToGroup(Kopete::MetaContact*, Kopete::Group*)) );
	connect( kcl, SIGNAL(metaContactRemovedFromGroup(Kopete::MetaContact*, Kopete::Group*)),
	         this, SLOT(removeMetaContactFromGroup(Kopete::MetaContact*, Kopete::Group*)) );
	connect( kcl, SIGNAL(metaContactMovedToGroup(Kopete::MetaContact*, Kopete::Group*, Kopete::Group*)),
	         this, SLOT(moveMetaContactToGroup(Kopete::MetaContact*, Kopete::Group*, Kopete::Group*)));
}


ContactListModel::~ContactListModel()
{
}

void ContactListModel::addMetaContact( Kopete::MetaContact* contact )
{
	foreach( Kopete::Group* g, contact->groups() )
		addMetaContactToGroup( contact, g );
	
	connect( contact, SIGNAL(onlineStatusChanged(Kopete::MetaContact*, Kopete::OnlineStatus::StatusType)),
	         this, SLOT(handleContactDataChange(Kopete::MetaContact*)) );
	connect( contact, SIGNAL(statusMessageChanged(Kopete::MetaContact*)),
	         this, SLOT(handleContactDataChange(Kopete::MetaContact*)) );
}

void ContactListModel::removeMetaContact( Kopete::MetaContact* contact )
{
	foreach(Kopete::Group *g, contact->groups())
		removeMetaContactFromGroup(contact, g);

	disconnect( contact, SIGNAL(onlineStatusChanged(Kopete::MetaContact*, Kopete::OnlineStatus::StatusType)),
	            this, SLOT(handleContactDataChange(Kopete::MetaContact*)));
	disconnect( contact, SIGNAL(statusMessageChanged(Kopete::MetaContact*)),
	            this, SLOT(handleContactDataChange(Kopete::MetaContact*)) );
}

void ContactListModel::addGroup( Kopete::Group* group )
{
	int pos = m_groups.count();
	kDebug(14001) << "addGroup" << group->displayName();
	beginInsertRows( QModelIndex(), pos, pos );
	m_groups.append( group );
	m_contacts[group] = QList<Kopete::MetaContact*>();
	endInsertRows();
}

void ContactListModel::removeGroup( Kopete::Group* group )
{
	int pos = m_groups.indexOf( group );
	beginRemoveRows( QModelIndex(), pos, pos );
	m_groups.removeAt( pos );
	m_contacts.remove( group );
	endRemoveRows();
}

void ContactListModel::addMetaContactToGroup( Kopete::MetaContact *mc, Kopete::Group *group )
{
	int pos = m_groups.indexOf( group );
	int offset = m_contacts[group].count();
	
	if (pos == -1)
	{
		addGroup( group );
		pos = m_groups.indexOf( group );
	}

	// if the metacontact already belongs to the group, returns
	if (m_contacts[group].contains(mc))
		return;

	QModelIndex idx = index(pos, 0);
	beginInsertRows(idx, offset, offset);
	m_contacts[group].append(mc);
	endInsertRows();

	// emit the dataChanged signal for the group index so that the filtering proxy
	// can evaluate the row
	emit dataChanged(idx, idx);
}

void ContactListModel::removeMetaContactFromGroup( Kopete::MetaContact *mc, Kopete::Group *group )
{
	int pos = m_groups.indexOf( group );
	int offset = m_contacts[group].indexOf(mc);

	// if the mc is not on the list anymore, just returns
	if (offset == -1)
		return;

	QModelIndex idx = index(pos, 0);
	beginRemoveRows(idx, offset, offset);
	m_contacts[group].removeAt(offset);
	endRemoveRows();

	// emit the dataChanged signal for the group index so that the filtering proxy
	// can evaluate if the group row should still be visible
	emit dataChanged(idx, idx);
}

void ContactListModel::moveMetaContactToGroup( Kopete::MetaContact *mc, Kopete::Group *from, Kopete::Group *to)
{
	removeMetaContactFromGroup(mc, from);
	addMetaContactToGroup(mc, to);
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
		cnt = m_groups.count();
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
		case Kopete::Items::ObjectRole:
			return qVariantFromValue( (QObject*)g );
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
		case Kopete::Items::IdRole:
			return QString::number(g->groupId());
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
		case Kopete::Items::MetaContactImageRole:
			return metaContactImage( mc );
			break;
		case Qt::ToolTipRole:
			return metaContactTooltip( mc );
			break;
		case Kopete::Items::TypeRole:
			return Kopete::Items::MetaContact;
			break;
		case Kopete::Items::ObjectRole:
			return qVariantFromValue( (QObject*)mc );
			break;
		case Kopete::Items::UuidRole:
			return mc->metaContactId().toString();
			break;
		case Kopete::Items::OnlineStatusRole:
			return mc->status();
			break;
		case Kopete::Items::StatusMessageRole:
			return mc->statusMessage().message();
			break;
		case Kopete::Items::StatusTitleRole:
			return mc->statusMessage().title();
			break;
		case Kopete::Items::AccountIconsRole:
			QList<QVariant> accountIconList;
			foreach ( Kopete::Contact *contact, mc->contacts() )
				accountIconList << qVariantFromValue( contact->onlineStatus().iconFor( contact ) );

			return accountIconList;
		}
	}

	return QVariant();
}

Qt::ItemFlags ContactListModel::flags( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return 0;

	Qt::ItemFlags f(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	
	// if it is a contact item, add the selectable flag
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		// TODO: for now we are only allowing drag-n-drop of a
		// metacontact if all the accounts its contacts belong are online
		Kopete::ContactListElement *cle = static_cast<Kopete::ContactListElement*>(index.internalPointer());
		Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>(cle);
		if (mc)
		{
			bool online = true;
			foreach(Kopete::Contact *c, mc->contacts())
			{
				if (!c->account()->isConnected())
				{
					online = false;
					break;
				}
			}
			if (online)
				f = f | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
		}
	}
	else if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
		f |= Qt::ItemIsDropEnabled;
	
	return f;
}

Qt::DropActions ContactListModel::supportedDropActions() const
{
	return QAbstractItemModel::supportedDropActions();
	return Qt::MoveAction;
}

QMimeData* ContactListModel::mimeData(const QModelIndexList &indexes) const
{
	// the mimeData encoded by QAbstractItemView is required for the 
	// drop indicators to work on QTreeView
	QMimeData *mdata = QAbstractItemModel::mimeData(indexes);
	QByteArray encodedData;

	QDataStream stream(&encodedData, QIODevice::WriteOnly);

	foreach (QModelIndex index, indexes) 
	{
		if (index.isValid() && data(index, Kopete::Items::TypeRole) == Kopete::Items::MetaContact)
		{
			// each metacontact entry will be encoded as group/uuid to
			// make sure that when moving a metacontact from one group
			// to another it will handle the right group
			
			// so get the group id
			QString text = data(index.parent(), Kopete::Items::IdRole).toString();
			
			// and the metacontactid
			text += "/" + data(index, Kopete::Items::UuidRole).toString();
			stream << text;
		}
	}

	mdata->setData("application/kopete.metacontacts.list", encodedData);
	return mdata;
}

bool ContactListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, 
                                    int row, int column, const QModelIndex &parent)
{
	if (action == Qt::IgnoreAction)
		return true;

	// for now only accepting drop of metacontacts
	// TODO: support dropping of files in metacontacts to allow file transfers
	if (!data->hasFormat("application/kopete.metacontacts.list"))
		return false;

	// contactlist has only one column
	if (column > 0)
		return false;
	
	// we don't support dropping things in an empty space
	if (!parent.isValid())
		return false;

	// decode the mime data
	QByteArray encodedData = data->data("application/kopete.metacontacts.list");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	QList<Kopete::MetaContact*> metaContacts;
	QList<Kopete::Group*> groups; // each metacontact is linked to one group in this context
	
	while (!stream.atEnd()) 
	{
		QString line;
		stream >> line;

		QStringList entry = line.split("/");
		
		QString grp = entry[0];
		QString id = entry[1];
		
		metaContacts.append(Kopete::ContactList::self()->metaContact( QUuid(id) ));
		groups.append(Kopete::ContactList::self()->group( grp.toUInt() ));
	}

	Kopete::ContactListElement *cle = static_cast<Kopete::ContactListElement*>(parent.internalPointer());
	// check if the parent is a group or a metacontact
	if (parent.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact)
	{
		Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>(cle);
		if (!mc)
			return false;

		// Merge the metacontacts from mimedata into this one
		Kopete::ContactList::self()->mergeMetaContacts(metaContacts, mc);
		return true;
	}
	else if (parent.data( Kopete::Items::TypeRole ) == Kopete::Items::Group)
	{
		Kopete::Group *g = dynamic_cast<Kopete::Group*>(cle);
		if (!g)
			return false;

		// if we have metacontacts on the mime data, move them to this group
		for (int i=0; i < metaContacts.count(); ++i)
			metaContacts[i]->moveToGroup(groups[i], g);

		return true;
	}

	return false;
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
			// WARNING: FIX THIS!!! We can't have one metaContact in more groups
			// because we won't find correct group and item won't be updated in proxy
			g = mc->groups().last();
			parent = createIndex( m_groups.indexOf( g ), 0, g );
		}
	}
	return parent;
}

QModelIndexList ContactListModel::indexListFor(Kopete::ContactListElement *ce) const
{
	QModelIndexList indexList;
	Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>(ce);

	if (mc)
	{
		// metacontact handling
		// search for all the groups in which this contact is
		foreach( Kopete::Group *g, mc->groups() )
		{
			int groupPos = m_groups.indexOf( g );
			int mcPos = m_contacts[g].indexOf( mc );

			// get  the group index to be the parent for the contact search
			QModelIndex grpIndex = index(groupPos, 0);

			QModelIndex mcIndex = index(mcPos, 0, grpIndex);
			if (mcIndex.isValid())
				indexList.append(mcIndex);
		}
	}
	else
	{
		// group handling
		Kopete::Group *g = dynamic_cast<Kopete::Group*>(ce);
		if (g)
		{
			int pos = m_groups.indexOf( g );
			indexList.append(index(pos,0));
		}
	}

	return indexList;
}

void ContactListModel::resetModel()
{
	reset();
}

void ContactListModel::handleContactDataChange(Kopete::MetaContact* mc)
{
	// get all the indexes for this metacontact
	QModelIndexList indexList = indexListFor(mc);

	// and now notify all the changes
	foreach(QModelIndex index, indexList)
	{
		// we need to emit the dataChanged signal to the groups this metacontact belongs to
		// so that proxy filtering is aware of the changes, first we have to update the parent
		// otherwise the group won't be expandable.
		emit dataChanged(index.parent(), index.parent());
		emit dataChanged(index, index);
	}
}

QVariant ContactListModel::metaContactImage( Kopete::MetaContact* mc ) const
{
	using namespace Kopete;

	int iconMode = AppearanceSettings::self()->contactListIconMode();
	if ( iconMode == AppearanceSettings::EnumContactListIconMode::IconPhoto )
	{
		QImage img = mc->picture().image();
		if ( !img.isNull() )
		{
			if ( mc->status() == Kopete::OnlineStatus::Offline )
				Blitz::grayscale(img);

			return img;
		}
	}

	switch( mc->status() )
	{
	case OnlineStatus::Online:
		if( mc->useCustomIcon() )
			return mc->icon( ContactListElement::Online );
		else
			return QString::fromUtf8( "user-online" );
		break;
	case OnlineStatus::Away:
		if( mc->useCustomIcon() )
			return mc->icon( ContactListElement::Away );
		else
			return QString::fromUtf8( "user-away" );
		break;
	case OnlineStatus::Unknown:
		if( mc->useCustomIcon() )
			return mc->icon( ContactListElement::Unknown );
		if ( mc->contacts().isEmpty() )
			return QString::fromUtf8( "metacontact_unknown" );
		else
			return QString::fromUtf8( "user-offline" );
		break;
	case OnlineStatus::Offline:
	default:
		if( mc->useCustomIcon() )
			return mc->icon( ContactListElement::Offline );
		else
			return QString::fromUtf8( "user-offline" );
		break;
	}

	return QVariant();
}

QString ContactListModel::metaContactTooltip( Kopete::MetaContact* metaContact ) const
{
	// We begin with the meta contact display name at the top of the tooltip
	QString toolTip = QLatin1String("<qt><table cellpadding=\"0\" cellspacing=\"1\">");
	toolTip += QLatin1String("<tr><td>");
	
	if ( !metaContact->picture().isNull() )
	{
#ifdef __GNUC__
#warning Currently using metaContact->picture().path() but should use replacement of KopeteMimeSourceFactory
#endif
#if 0
			QString photoName = QLatin1String("kopete-metacontact-photo:%1").arg( KUrl::encode_string( metaContact->metaContactId() ));
			//QMimeSourceFactory::defaultFactory()->setImage( "contactimg", metaContact->photo() );
			toolTip += QString::fromLatin1("<img src=\"%1\">").arg( photoName );
#endif
		toolTip += QString::fromLatin1("<img src=\"%1\">").arg( metaContact->picture().path() );
	}

	toolTip += QLatin1String("</td><td>");

	QString displayName;
	QList<KEmoticonsTheme::Token> t = Kopete::Emoticons::tokenize( metaContact->displayName());
	QList<KEmoticonsTheme::Token>::iterator it;
	for( it = t.begin(); it != t.end(); ++it )
	{
		if( (*it).type == KEmoticonsTheme::Image )
			displayName += (*it).picHTMLCode;
		else if( (*it).type == KEmoticonsTheme::Text )
			displayName += Qt::escape( (*it).text );
	}

	toolTip += QString::fromLatin1("<b><font size=\"+1\">%1</font></b><br><br>").arg( displayName );

	QList<Contact*> contacts = metaContact->contacts();
	if ( contacts.count() == 1 )
		return toolTip + contacts.first()->toolTip() + QLatin1String("</td></tr></table></qt>");

	toolTip += QLatin1String("<table>");

	// We are over a metacontact with > 1 child contacts, and not over a specific contact
	// Iterate through children and display a summary tooltip
	foreach ( Contact* c, contacts )
	{
		QString iconName = QString::fromLatin1("kopete-contact-icon:%1:%2:%3")
			.arg( QString(QUrl::toPercentEncoding( c->protocol()->pluginId() )),
			      QString(QUrl::toPercentEncoding( c->account()->accountId() )),
			      QString(QUrl::toPercentEncoding( c->contactId() ) )
			    );

		toolTip += i18nc("<tr><td>STATUS ICON <b>PROTOCOL NAME</b> (ACCOUNT NAME)</td><td>STATUS DESCRIPTION</td></tr>",
		                 "<tr><td><img src=\"%1\">&nbsp;<nobr><b>%2</b></nobr>&nbsp;<nobr>(%3)</nobr></td><td align=\"right\"><nobr>%4</nobr></td></tr>",
		                 iconName, Kopete::Emoticons::parseEmoticons(c->property(Kopete::Global::Properties::self()->nickName()).value().toString()) , c->contactId(), c->onlineStatus().description() );
	}

	return toolTip + QLatin1String("</table></td></tr></table></qt>");
}

}

}

#include "contactlistmodel.moc"
//kate: tab-width 4
