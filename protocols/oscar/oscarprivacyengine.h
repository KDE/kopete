/*
    oscarprivacyengine.h  -  Oscar Privacy Engine

    Copyright (c) 2005-2006 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2005-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARPRIVACYENGINE_H
#define OSCARPRIVACYENGINE_H

#include <QtCore/QObject>

#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtGui/QStandardItemModel>

#include "kopete_export.h"

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/

class QComboBox;
class QAbstractItemView;

class OscarAccount;
namespace Oscar {
class Client;
}

class OSCAR_EXPORT OscarPrivacyEngine : public QObject
{
	Q_OBJECT
public:
	enum Type { Visible, Invisible, Ignore };
	OscarPrivacyEngine( OscarAccount* account, Type type );
	~OscarPrivacyEngine();
	
	void setContactsView( QAbstractItemView* view );
	void setAllContactsView( QComboBox* combo );
	
public slots:
	void slotAdd();
	void slotRemove();
	void storeChanges();
	
private:
	typedef QMap<QString, QString> ContactMap;
	
	void addContacts( const ContactMap& contacts, const QSet<QString>& idSet );
	void addAllContacts( const ContactMap& contacts );
	
	enum Action{ Remove = 0, Add };
	typedef QMap<QString, Action> ChangeMap;
	
	//map with changes that should be send to server
	ChangeMap m_changesMap;
	
	QSet<QString> m_idSet;
	
	QStandardItemModel m_contactsModel;
	QStandardItemModel m_allContactsModel;
	
	Oscar::Client* m_client;
	Type m_type;
	
	QComboBox* m_comboBox;
	QAbstractItemView* m_listView;
};

#endif
