/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Aleix Pol              <aleixpol@gmail.com>
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

#ifndef KOPETE_UI_CONTACTLISTTREEMODEL_H
#define KOPETE_UI_CONTACTLISTTREEMODEL_H

#include "contactlistmodel.h"

#include <kopete_export.h>

namespace Kopete {

class Group;
class MetaContact;
class ContactListElement;
	
namespace UI {

class MetaContactModelItem;
class GroupModelItem;
class ContactListModelItem;
/**
@author Aleix Pol <aleixpol@gmail.com>
*/
class KOPETE_CONTACT_LIST_EXPORT ContactListTreeModel : public ContactListModel
{
Q_OBJECT
public:
	ContactListTreeModel(QObject* parent = 0);
	~ContactListTreeModel();

	virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

	virtual QModelIndex parent ( const QModelIndex & index ) const;
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action,
	                           int row, int column, const QModelIndex &parent );

public Q_SLOTS:
	virtual void addMetaContact( Kopete::MetaContact* );
	virtual void removeMetaContact( Kopete::MetaContact* );

	virtual void addGroup( Kopete::Group* );
	virtual void removeGroup( Kopete::Group* );

	virtual void addMetaContactToGroup( Kopete::MetaContact*, Kopete::Group* );
	virtual void removeMetaContactFromGroup( Kopete::MetaContact*, Kopete::Group* );

protected Q_SLOTS:
	virtual void handleContactDataChange(Kopete::MetaContact*);
	virtual void appearanceConfigChanged();
	virtual void loadContactList();

protected:
	virtual void loadModelSettingsImpl( QDomElement& rootElement );
	virtual void saveModelSettingsImpl( QDomDocument& doc, QDomElement& rootElement );

	virtual bool dropMetaContacts( int row, const QModelIndex &parent, Qt::DropAction action, const QList<GroupMetaContactPair> &items );

private:
	ContactListModelItem* itemFor( const QModelIndex& index ) const;
	QModelIndex indexFor( ContactListModelItem* modelItem ) const;
	QModelIndexList indexListFor ( Kopete::ContactListElement* ) const;

	int countConnected( GroupModelItem* gmi ) const;
	QVariant metaContactImage( Kopete::MetaContact* mc ) const;
	QString metaContactTooltip( Kopete::MetaContact* metaContact ) const;

	QHash<GroupMetaContactPair, int> m_addContactPosition;

	GroupModelItem* m_topLevelGroup;
	QHash<Kopete::Group*, GroupModelItem*> m_groups;
	QMap<GroupMetaContactPair, MetaContactModelItem*> m_metaContacts;
};

}

}

#endif
//kate: tab-width 4
