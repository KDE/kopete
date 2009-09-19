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

#ifndef WPPROTOCOL_H
#define WPPROTOCOL_H

// QT Includes
#include <qpixmap.h>
#include <qdatetime.h>

// Kopete Includes
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopeteonlinestatus.h"

// Local Includes
#include "libwinpopup.h"
#include "wpaddcontact.h"

namespace Kopete { class Account; }

/**
 * The actual Protocol class used by Kopete.
 */
class WPProtocol : public Kopete::Protocol
{
	Q_OBJECT

// Kopete::Protocol overloading
public:
	WPProtocol( QObject *parent, const QVariantList & );
	~WPProtocol();

	virtual AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *theAccount);
	virtual KopeteEditAccountWidget *createEditAccountWidget(Kopete::Account *account, QWidget *parent);
	virtual Kopete::Account *createNewAccount(const QString &accountId);

	const QStringList getGroups() {return popupClient->getGroups(); }
	const QStringList getHosts(const QString &Group) { return popupClient->getHosts(Group); }
	bool checkHost(const QString &Name) { return popupClient->checkHost(Name); }

// Kopete::Plugin overloading
public:
	virtual Kopete::Contact *deserializeContact(Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData);

// Stuff used internally & by colleague classes
public:
	static WPProtocol *protocol() { return sProtocol; }

	const Kopete::OnlineStatus WPOnline;
	const Kopete::OnlineStatus WPAway;
	const Kopete::OnlineStatus WPOffline;
	void sendMessage(const QString &Body, const QString &Destination);
	void settingsChanged(void);			// Callback when settings changed

public slots:
	void installSamba();				// Modify smb.conf to use winpopup-send script
	void slotReceivedMessage(const QString &Body, const QDateTime &Time, const QString &From);

private:
	QString smbClientBin;
	int groupCheckFreq;
	void readConfig();
	WinPopupLib *popupClient;
	static WPProtocol *sProtocol;			// Singleton
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
