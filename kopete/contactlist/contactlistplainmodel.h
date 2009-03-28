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

#ifndef KOPETE_UI_CONTACTLISTPLAINMODEL_H
#define KOPETE_UI_CONTACTLISTPLAINMODEL_H

#include "contactlistmodel.h"

#include <kopete_export.h>

namespace Kopete {

class Group;
class Contactlist;
class MetaContact;
class ContactListElement;
	
namespace UI {

class MetaContactModelItem;
class GroupModelItem;
class ContactListPlainModelItem;
/**
@author Aleix Pol <aleixpol@gmail.com>
*/
class KOPETE_CONTACT_LIST_EXPORT ContactListPlainModel : public ContactListModel
{
Q_OBJECT
public:
	ContactListPlainModel(QObject* parent = 0);
	~ContactListPlainModel();

	virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
	virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

	virtual QModelIndex parent ( const QModelIndex & index ) const;
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
	                          int row, int column, const QModelIndex &parent);

public Q_SLOTS:
	virtual void addMetaContact( Kopete::MetaContact* );
	virtual void removeMetaContact( Kopete::MetaContact* );

protected Q_SLOTS:
	virtual void handleContactDataChange(Kopete::MetaContact*);
	virtual void appearanceConfigChanged();
	virtual void loadContactList();

protected:
	virtual void loadModelSettingsImpl( QDomElement& rootElement );
	virtual void saveModelSettingsImpl( QDomDocument& doc, QDomElement& rootElement );

	virtual bool dropMetaContacts( int row, const QModelIndex &parent, Qt::DropAction action, const QList<GroupMetaContactPair> &items );

private:
	void addMetaContactImpl( Kopete::MetaContact *mc );
	QModelIndexList indexListFor( Kopete::ContactListElement* ) const;

	void savePositions();
	void loadPositions();

	QHash<const Kopete::MetaContact*, int > m_addContactPosition;
	QList<Kopete::MetaContact*> m_contacts;
};

}

}

#endif
//kate: tab-width 4
