/***************************************************************************
     wpprotocol.h  -  Base class for the Kopete WP protocol
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Gav Wood
    email                : gav@kde.org

    Based on code from   : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __WPPROTOCOL_H
#define __WPPROTOCOL_H


// QT Includes
#include <qpixmap.h>
#include <qptrlist.h>

// KDE Includes

// Kopete Includes
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopeteonlinestatus.h"

// Local Includes
#include "libwinpopup.h"
#include "wpaddcontact.h"

namespace Kopete { class Account; }
class KPopupMenu;
class KActionMenu;
class KAction;
class WPContact;
class WPAccount;

/**
 * This is a subclass of the KWinPopup class needed in order to use the virtual
 * methods and communicate nicely with Kopete.
 */
class KopeteWinPopup: public KWinPopup
{
	Q_OBJECT

public slots:
	void slotSendMessage(const QString &Body, const QString &Destination) { sendMessage(Body, Destination); }

signals:
	void newMessage(const QString &Body, const QDateTime &Arrival, const QString &From);

protected:
	virtual void receivedMessage(const QString &Body, const QDateTime &Arrival, const QString &From) { emit newMessage(Body, Arrival, From); }

public:
	KopeteWinPopup(const QString &SMBClientPath, const QString &InitialSearchHost, const QString &HostName, int HostCheckFrequency, int MessageCheckFrequency) :
		KWinPopup(SMBClientPath, InitialSearchHost, HostName, HostCheckFrequency, MessageCheckFrequency) {}
};

/**
 * The actual Protocol class used by Kopete.
 */
class WPProtocol : public Kopete::Protocol
{
	Q_OBJECT

// Kopete::Protocol overloading
public:
	WPProtocol( QObject *parent, const char *name, const QStringList &args );
	~WPProtocol();

	virtual AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *theAccount);
	virtual KopeteEditAccountWidget *createEditAccountWidget(Kopete::Account *account, QWidget *parent);
	virtual Kopete::Account *createNewAccount(const QString &accountId);

// Kopete::Plugin overloading
public:
	virtual Kopete::Contact *deserializeContact(Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData);

// Stuff used internally & by colleague classes
public:
	static WPProtocol *protocol() { return sProtocol; }
	KopeteWinPopup *createInterface(const QString &theHostName);
	void destroyInterface(KopeteWinPopup *theInterface);

	const Kopete::OnlineStatus WPOnline;
	const Kopete::OnlineStatus WPAway;
	const Kopete::OnlineStatus WPOffline;

public slots:
	void slotSettingsChanged(void);			// Callback when settings changed
	void installSamba();					// Modify smb.conf to use winpopup-send.sh script

private:
	QPtrList<KopeteWinPopup> theInterfaces;	// List of all the interfaces created
	static WPProtocol *sProtocol;			// Singleton
};

#endif

