/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Aleix Pol              <aleixpol@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_UI_CONTACTLISTMODEL_H
#define KOPETE_UI_CONTACTLISTMODEL_H

#include <QAbstractItemModel>

#include <kopete_export.h>

namespace Kopete {

class Group;
class Contactlist;
class MetaContact;
class ContactListElement;
	
namespace UI {

class MetaContactModelItem;
class GroupModelItem;
class ContactListModelItem;
/**
@author Aleix Pol <aleixpol@gmail.com>
*/
class KOPETE_CONTACT_LIST_EXPORT ContactListModel : public QAbstractItemModel
{
Q_OBJECT
	public:
		ContactListModel(QObject* parent = 0);
		~ContactListModel();
		
		virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const { return 1; }
		virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
		virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
		
		virtual QModelIndex parent ( const QModelIndex & index ) const;
		virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
		virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
		
		virtual Qt::ItemFlags flags(const QModelIndex &index) const;

		/* drag-n-drop stuff */
		virtual Qt::DropActions supportedDropActions() const;
		virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
		virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
		                          int row, int column, const QModelIndex &parent);
		
		QModelIndexList indexListFor ( Kopete::ContactListElement* ) const;
	public Q_SLOTS:
		void addMetaContact( Kopete::MetaContact* );
		void removeMetaContact( Kopete::MetaContact* );
		
		void addGroup( Kopete::Group* );
		void removeGroup( Kopete::Group* );

		void addMetaContactToGroup( Kopete::MetaContact*, Kopete::Group* );
		void removeMetaContactFromGroup( Kopete::MetaContact*, Kopete::Group* );
		void moveMetaContactToGroup( Kopete::MetaContact*, Kopete::Group*, Kopete::Group*);
	
	private Q_SLOTS:
		void resetModel();
		void handleContactDataChange(Kopete::MetaContact*);
		void loadContactList();

	private:
		int indexOfMetaContact( const GroupModelItem* inGroup, const Kopete::MetaContact* mc ) const;
		int indexOfGroup( Kopete::Group* group ) const;

		int childCount( const QModelIndex& parent ) const;
		int countConnected( GroupModelItem* gmi ) const;
		QVariant metaContactImage( Kopete::MetaContact* mc ) const;
		QString metaContactTooltip( Kopete::MetaContact* metaContact ) const;

		QList<GroupModelItem*> m_groups;
		QMap<const GroupModelItem*, QList<MetaContactModelItem*> > m_contacts;
};

class ContactListModelItem {
public:
	ContactListModelItem() {}
	virtual ~ContactListModelItem() {}

private:
	// For dynamic cast
	virtual void dummy() {}
};

class GroupModelItem : public ContactListModelItem {
public:
	GroupModelItem( Kopete::Group* group )
		: ContactListModelItem(), mGroup( group )
	{}
	
	inline Kopete::Group* group() const { return mGroup; }
private:
	Kopete::Group* mGroup;
};

class MetaContactModelItem : public ContactListModelItem {
public:
	MetaContactModelItem( GroupModelItem* groupModelItem, Kopete::MetaContact* metaContact )
		: ContactListModelItem(), mMetaContact( metaContact ), mGroupModelItem( groupModelItem )
	{}
	
	inline Kopete::MetaContact* metaContact() const { return mMetaContact; }
	inline GroupModelItem* groupModelItem() const { return mGroupModelItem; }
private:
	Kopete::MetaContact* mMetaContact;
	GroupModelItem* mGroupModelItem;
};

}

}

#endif
//kate: tab-width 4
