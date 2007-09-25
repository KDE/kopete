/*
    Kopete Contactlist Model

    Copyright (c) 2007      by Matt Rogers            <mattr@kde.org>

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

#include <QStandardItemModel>

namespace Kopete {

class Group;
class Contactlist;
class MetaContact;

namespace UI {

/**
@author Matt Rogers <mattr@kde.org>
*/
class ContactListModel : public QStandardItemModel
{
Q_OBJECT
public:
	ContactListModel(QObject* parent = 0);
	~ContactListModel();

	virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
	
public Q_SLOTS:
	void addMetaContact( Kopete::MetaContact* );
	void removeMetaContact( Kopete::MetaContact* );

	void addGroup( Kopete::Group* );
	void removeGroup( Kopete::Group* );
};

}

}

#endif
