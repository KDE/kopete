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

namespace Kopete {

class Group;
class Contactlist;
class MetaContact;
class ContactListElement;
	
namespace UI {

/**
@author Aleix Pol <aleixpol@gmail.com>
*/
class ContactListModel : public QAbstractItemModel
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
  
	private:
		int childCount(const QModelIndex& parent) const;
		int countConnected(Kopete::Group* g) const;
		QVariant metaContactImage( Kopete::MetaContact* mc ) const;
		
		QList<Kopete::Group*> m_groups;
		QMap<Kopete::Group*, QList<Kopete::MetaContact*> > m_contacts;
};

}

}

#endif
//kate: tab-width 4
