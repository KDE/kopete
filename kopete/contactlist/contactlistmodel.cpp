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

	// Wait till whole contact list is loaded so we can apply manual sort.
	if ( !kcl->loaded() )
		connect( kcl, SIGNAL(contactListLoaded()), this, SLOT(loadContactList()) );
	else
		loadContactList();
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
	GroupModelItem* gmi = new GroupModelItem( group );
	m_groups.append( gmi );
	m_contacts[gmi] = QList<MetaContactModelItem*>();
	endInsertRows();
}

void ContactListModel::removeGroup( Kopete::Group* group )
{
	int pos = indexOfGroup( group );
	beginRemoveRows( QModelIndex(), pos, pos );
	GroupModelItem* gmi = m_groups.takeAt( pos );
	qDeleteAll( m_contacts.value( gmi ) );
	m_contacts.remove( gmi );
	endRemoveRows();
}

void ContactListModel::addMetaContactToGroup( Kopete::MetaContact *mc, Kopete::Group *group )
{
	int pos = indexOfGroup( group );
	if (pos == -1)
	{
		addGroup( group );
		pos = indexOfGroup( group );
	}

	GroupModelItem* groupModelItem = m_groups.at( pos );

	// if the metacontact already belongs to the group, returns
	if ( indexOfMetaContact( groupModelItem, mc ) != -1 )
		return;

	int mcDesireIndex = m_contacts[groupModelItem].count();
	MetaContactModelItem* mcModelItem = new MetaContactModelItem( groupModelItem, mc );

	QModelIndex idx = index( pos, 0 );
	beginInsertRows( idx, mcDesireIndex, mcDesireIndex );
	m_contacts[groupModelItem].append( mcModelItem );
	endInsertRows();

	// emit the dataChanged signal for the group index so that the filtering proxy
	// can evaluate the row
	emit dataChanged(idx, idx);
}

void ContactListModel::removeMetaContactFromGroup( Kopete::MetaContact *mc, Kopete::Group *group )
{
	int pos = indexOfGroup( group );
	Q_ASSERT( pos != -1 );

	GroupModelItem* groupModelItem = m_groups.at( pos );
	// if the mc is not on the list anymore, just returns
	int offset = indexOfMetaContact( groupModelItem, mc );
	if (offset == -1)
		return;

	QModelIndex idx = index(pos, 0);
	beginRemoveRows(idx, offset, offset);
	delete m_contacts[groupModelItem].takeAt(offset);
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

int ContactListModel::indexOfMetaContact( const GroupModelItem* inGroup, const Kopete::MetaContact* mc ) const
{
	QList<MetaContactModelItem*> mcModelItemList = m_contacts.value( inGroup );
	for ( int i = 0; i < mcModelItemList.size(); ++i )
	{
		if ( mcModelItemList.at( i )->metaContact() == mc )
			return i;
	}
	return -1;
}

int ContactListModel::indexOfGroup( Kopete::Group* group ) const
{
	for ( int i = 0; i < m_groups.size(); ++i )
	{
		if ( m_groups.at( i )->group() == group )
			return i;
	}
	return -1;
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
		ContactListModelItem *clmi = static_cast<ContactListModelItem*>( parent.internalPointer() );
		GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );
		if ( gmi )
			cnt = m_contacts[gmi].count();
	}
	
	return cnt;
}

int ContactListModel::rowCount( const QModelIndex& parent ) const
{
	ContactListModelItem *clmi = static_cast<ContactListModelItem*>( parent.internalPointer() );
	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );

	int cnt = 0;
	if ( !parent.isValid() )
		cnt = m_groups.count();
	else
	{
		if ( gmi )
			cnt+= m_contacts[gmi].count();
	}

	return cnt;
}

bool ContactListModel::hasChildren( const QModelIndex& parent ) const
{
	ContactListModelItem *clmi = static_cast<ContactListModelItem*>( parent.internalPointer() );
	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );
	
	bool res = false;
	if ( !parent.isValid() )
		res=!m_groups.isEmpty();
	else
	{
		if ( gmi )
		{
			int row = parent.row();
			GroupModelItem *gmi = m_groups[row];
			res = !m_contacts[gmi].isEmpty();
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

	ContactListModelItem *clmi = static_cast<ContactListModelItem*>( parent.internalPointer() );
	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>(clmi);
	
	QModelIndex idx;
	if( !parent.isValid() )
		idx = createIndex( row, column, m_groups[row] );
	else if ( gmi )
		idx = createIndex( row, column, m_contacts[gmi][row] );

	return idx;
}

int ContactListModel::countConnected( GroupModelItem* gmi ) const
{
	int onlineCount = 0;
	
	QList<MetaContactModelItem*> metaContactList = m_contacts.value(gmi);
	QList<MetaContactModelItem*>::const_iterator it, itEnd;
	itEnd = metaContactList.constEnd();
	for (it = metaContactList.constBegin(); it != itEnd; ++it)
	{
	  if ( (*it)->metaContact()->isOnline() )
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
	ContactListModelItem *clmi = static_cast<ContactListModelItem*>( index.internalPointer() );
	GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );
	MetaContactModelItem *mcmi = dynamic_cast<MetaContactModelItem*>( clmi );

	QString display;
	QImage img;
	if ( gmi )
	{
		Kopete::Group* g = gmi->group();
		switch ( role )
		{
		case Qt::DisplayRole:
			display = i18n( "%1 (%2/%3)", g->displayName(), countConnected( gmi ),
			                m_contacts[gmi].count() );
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
			return countConnected( gmi );
			break;
		case Kopete::Items::OnlineStatusRole:
			return OnlineStatus::Unknown;
			break;
		case Kopete::Items::IdRole:
			return QString::number(g->groupId());
			break;
		}
	}

	if ( mcmi )
	{
		Kopete::MetaContact* mc = mcmi->metaContact();
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
		ContactListModelItem *clmi = static_cast<ContactListModelItem*>(index.internalPointer());
		MetaContactModelItem *mcmi = dynamic_cast<MetaContactModelItem*>(clmi);
		if (mcmi)
		{
			bool online = true;
			foreach(Kopete::Contact *c, mcmi->metaContact()->contacts())
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
	{
		f |= Qt::ItemIsDropEnabled;
// 		if ( Kopete::AppearanceSettings::self()->contactListGroupSorting() == Kopete::AppearanceSettings::EnumContactListGroupSorting::Manual )
// 			f |= Qt::ItemIsDragEnabled;
	}
	
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

	ContactListModelItem *clmi = static_cast<ContactListModelItem*>(parent.internalPointer());
	// check if the parent is a group or a metacontact
	if (parent.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact)
	{
		MetaContactModelItem *mcmi = dynamic_cast<MetaContactModelItem*>(clmi);
		if ( !mcmi )
			return false;

		// Merge the metacontacts from mimedata into this one
		Kopete::ContactList::self()->mergeMetaContacts(metaContacts, mcmi->metaContact());
		return true;
	}
	else if (parent.data( Kopete::Items::TypeRole ) == Kopete::Items::Group)
	{
		GroupModelItem *gmi = dynamic_cast<GroupModelItem*>(clmi);
		if ( !gmi )
			return false;

		// if we have metacontacts on the mime data, move them to this group
		for (int i=0; i < metaContacts.count(); ++i)
			metaContacts[i]->moveToGroup( groups[i], gmi->group() );

		return true;
	}

	return false;
}

QModelIndex ContactListModel::parent(const QModelIndex & index) const
{
	QModelIndex parent;
	
	if(index.isValid())
	{
		ContactListModelItem *clmi = static_cast<ContactListModelItem*>( index.internalPointer() );
		GroupModelItem *gmi = dynamic_cast<GroupModelItem*>( clmi );
		if ( !gmi )
		{
			MetaContactModelItem *mcmi = dynamic_cast<MetaContactModelItem*>( clmi );
			gmi = mcmi->groupModelItem();
			parent = createIndex( m_groups.indexOf( gmi ), 0, gmi );
		}
	}
	return parent;
}

QModelIndexList ContactListModel::indexListFor( Kopete::ContactListElement* cle ) const
{
	QModelIndexList indexList;
	Kopete::MetaContact *mc = dynamic_cast<Kopete::MetaContact*>(cle);

	if (mc)
	{
		// metacontact handling
		// search for all the groups in which this contact is
		foreach( Kopete::Group *g, mc->groups() )
		{
			int groupPos = indexOfGroup( g );
			int mcPos = indexOfMetaContact( m_groups.at( groupPos ), mc );
			Q_ASSERT( mcPos != -1 );

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
		Kopete::Group *g = dynamic_cast<Kopete::Group*>(cle);
		if (g)
		{
			int pos = indexOfGroup( g );
			Q_ASSERT( pos != -1 );
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

void ContactListModel::loadContactList()
{
	Kopete::ContactList* kcl = Kopete::ContactList::self();
	disconnect( kcl, SIGNAL(contactListLoaded()), this, SLOT(loadContactList()) );

	foreach ( Kopete::Group* g, kcl->groups() )
		addGroup( g );

	foreach ( Kopete::MetaContact* mc, kcl->metaContacts() )
		addMetaContact( mc );

	//TODO: load group/mc positions of manual sort.

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

QVariant ContactListModel::metaContactImage( Kopete::MetaContact* mc ) const
{
	using namespace Kopete;

	int iconMode = AppearanceSettings::self()->contactListIconMode();
	if ( iconMode == AppearanceSettings::EnumContactListIconMode::IconPhoto )
	{
		QImage img = mc->picture().image();
		if ( !img.isNull() && img.width() > 0 && img.height() > 0 )
			return img;
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
