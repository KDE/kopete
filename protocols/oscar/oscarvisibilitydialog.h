/*
    oscarvisibilitydialog.h  -  Visibility Dialog

    Copyright (c) 2005 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef OSCARVISIBILITYDIALOG_H
#define OSCARVISIBILITYDIALOG_H

#include <kdialogbase.h>
#include "kopete_export.h"

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/
class OscarVisibilityBase;
class QStringList;
class Client;

class KOPETE_EXPORT OscarVisibilityDialog : public KDialogBase
{
	Q_OBJECT
public:
	typedef QMap<QString, QString> ContactMap;

	OscarVisibilityDialog( Client* client, QWidget* parent );
	~OscarVisibilityDialog() {}

	void addContacts( const ContactMap& contacts );
	void addVisibleContacts( const QStringList& contactList );
	void addInvisibleContacts( const QStringList& contactList );

signals:
	void closing();

protected:
	virtual void slotOk();
	virtual void slotCancel();

protected slots:
	void slotAddToVisible();
	void slotRemoveFromVisible();
	void slotAddToInvisible();
	void slotRemoveFromInvisible();

private:
	enum Action{ Remove = 0, Add };
	typedef QMap<QString, Action> ChangeMap;
	
	//maps with changes that should be send to server
	ChangeMap m_visibleListChangesMap;
	ChangeMap m_invisibleListChangesMap;
	
	ContactMap m_contactMap;
	
	OscarVisibilityBase* m_visibilityUI;
	Client* m_client;
};

#endif
