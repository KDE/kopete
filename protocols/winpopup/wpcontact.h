/***************************************************************************
                          wpcontact.h  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@indigoarchive.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WPCONTACT_H
#define WPCONTACT_H

// KDE Includes
#include <kaction.h>

// Qt Includes
#include <qdatetime.h>
#include <QList>
#include <qtimer.h>
#include <qstringlist.h>

// Kopete Includes
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopetechatsession.h"
#include "kopetemessage.h"

// Local Includes
#include "wpprotocol.h"
#include "wpuserinfo.h"

class QTimer;
class Q3ListView;
class KAction;
namespace Kopete { class MetaContact; }

class WPContact: public Kopete::Contact
{
	Q_OBJECT

public:
	WPContact(Kopete::Account *account, const QString &userId, const QString &fullName, Kopete::MetaContact *metaContact);

//	virtual bool isOnline() const;
	virtual bool isReachable();
	virtual QList<KAction*> *customContextMenuActions();
	using Kopete::Contact::customContextMenuActions;
	virtual Kopete::ChatSession *manager(Kopete::Contact::CanCreateFlags = Kopete::Contact::CannotCreate);
	virtual void serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData);

public slots:
	virtual void slotUserInfo();
	void slotCheckStatus();	// the call back for the checkStatus timer
	void slotNewMessage(const QString &Body, const QDateTime &Arrival);

private slots:
	void slotChatSessionDestroyed();
	void slotSendMessage(Kopete::Message &message);
	void slotCloseUserInfoDialog(); // Called when the userinfo dialog is getting closed

private:
	bool myWasConnected;	// true if protocol connected at last check

	QTimer checkStatus;		// checks the status of this contact every second or so
//	KActionCollection *myActionCollection;
							// holds all the protocol specific actions (not many!)
	Kopete::ChatSession *m_manager;
							// holds the two message managers - one for email and one for chat
	WPUserInfo *m_infoDialog;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
