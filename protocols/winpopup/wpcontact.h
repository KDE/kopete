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

#ifndef __WPCONTACT_H
#define __WPCONTACT_H

// KDE Includes
#include <kaction.h>

// Qt Includes
#include <qvaluestack.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcursor.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qstringlist.h>

// Kopete Includes
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemessagemanager.h"
#include "kopetehistorydialog.h"
#include "kopetemessage.h"

// Local Includes
#include "wpprotocol.h"

class QTimer;
class QPixmap;
class QListView;
class QListViewItem;
class KPopupMenu;
class KAction;
class KListAction;
class KopeteMetaContact;

class WPProtocol;

class WPContact: public KopeteContact
{
	Q_OBJECT

private:
	bool myIsOnline;		// true if online, false if not
	QString myHost;			// stores the hostname of this contact
	WPProtocol *myProtocol;	// stores the protocol instance to which this contact belongs
	QTimer checkStatus;		// checks the status of this contact every second or so
	KActionCollection *myActionCollection;
							// holds all the protocol specific actions (not many!)
	KopeteMessageManager *myEmailManager, *myChatManager;
							// holds the two message managers - one for email and one for chat

public slots:
	void slotCheckStatus();	// the call back for the checkStatus timer
	void slotNewMessage(const QString &Body, const QDateTime &Arrival);
							// the call back for the winpopup protocol
	void slotSendMessage(const KopeteMessage& message);
							// carries out sending of a message (supporting a hacked-up subject field)

public:
	WPContact(WPProtocol *protocol, const QString &userID, KopeteMetaContact *parent);
							// the constructor
	const QString host() { return myHost; }
							// the host name return method

//***********************************************************************
// BEGIN MANDATORY OVERLOADING
//***********************************************************************

public:
	// very basic actions
	bool isOnline() const { return myIsOnline; }
	bool isReachable() { return myIsOnline; }
//	QStringList groups() { return QStringList(); }
	KopeteContact::ContactStatus status() const { return myIsOnline ? Online : Offline; }
	QString statusText() const { return myIsOnline ? "Online" : "Offline"; }
	QString statusIcon() const { return myIsOnline ? "wp_available" : "wp_offline"; }
	
	int importance() const { return myIsOnline ? 20 : 0; }
	KActionCollection *customContextMenuActions() { return myActionCollection; }

	QString identityId() const { return myHost; }
	QString contactId() const { return "smb://" + myHost; }

	// null actions
/*	void addToGroup(const QString &) {}
	void removeFromGroup(const QString &) {}
	void moveToGroup(const QString &, const QString &) {}*/

public slots:
	// not quite so basic actions
	void execute();

	void slotDeleteContact() { deleteLater(); }
	void slotUserInfo() { /* show user info? */ }

signals:
	void statusChanged(KopeteContact *contact, KopeteContact::ContactStatus status);
private slots: // Private slots
	void slotMovedToMetaContact();
};

#endif
