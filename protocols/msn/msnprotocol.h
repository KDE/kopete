/*
    msnprotocol.h - Kopete MSN Protocol Plugin

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __msnprotocol_h__
#define __msnprotocol_h__

#include <qmap.h>
#include <qstringlist.h>

#include "kopeteprotocol.h"
#include "kopeteonlinestatus.h"
#include "kopetecontactproperty.h"

#include "msnsocket.h"

class KAction;
class KActionMenu;

class MSNContact;
class MSNAccount;
class MSNNotifySocket;
class MSNSwitchBoardSocket;
class MSNMessageManager;
class MSNInvitation;
class KopeteMessageManager;
class KopeteMetaContact;
class KopeteContact;
class KopeteMessage;
class KopeteGroup;

/**
 * @author duncan
 * @author Martijn Klingens <klingens@kde.org>
 * @author Olivier Goffart  <ogoffart@tiscalinet.be>
 */
class MSNProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	MSNProtocol( QObject *parent, const char *name, const QStringList &args );

	/**
	 * SyncMode indicates whether settings differing between client and
	 * server should be propagated to keep them in sync.
	 * SyncToServer   - Ignore the server setting when sent. Instead, push
	 *                  the local setting to the server. Used when changing
	 *                  settings offline.
	 * SyncFromServer - Update locally stored settings with the value sent
	 *                  by the server. Used when connecting to the server if
	 *                  no offline changes are pending to force a sync.
	 * SyncBoth       - Changes are updated both ways. This is truly a
	 *                  'first come, first serve' scenario, which breaks if
	 *                  the 'old' value is sent by one peer before the other
	 *                  end is able to push the new value. An example of this
	 *                  is changing the MSN nickname offline - the server can
	 *                  only be updated after it has sent the old value to
	 *                  the client during connect, destroying the new setting.
	 *                  Once connected this is often the most useful setting.
	 * DontSync       - Do not sync values at all. This is used if settings
	 *                  are overridden locally, but should not be sent to the
	 *                  server, nor should the client update server-pushed
	 *                  values. This can be useful for e.g. contact lists.
	 */
	enum SyncMode
	{
		DontSync       = 0x00,
		SyncToServer   = 0x01,
		SyncFromServer = 0x02,
		SyncBoth       = 0x03
	};

	/**
	 * The possible MSN online statuses
	 */
	const KopeteOnlineStatus NLN;  //online
	const KopeteOnlineStatus BSY;  //busy
	const KopeteOnlineStatus BRB;  //be right back
	const KopeteOnlineStatus AWY;  //away
	const KopeteOnlineStatus PHN;  //on the phone
	const KopeteOnlineStatus LUN;  //out to lunch
	const KopeteOnlineStatus FLN;  //offline
	const KopeteOnlineStatus HDN;  //invisible
	const KopeteOnlineStatus IDL;  //idle
	const KopeteOnlineStatus UNK;  //inknown (internal)
	const KopeteOnlineStatus CNT;  //connecting (internal)

	const Kopete::ContactPropertyTmpl propEmail;
	const Kopete::ContactPropertyTmpl propPhoneHome;
	const Kopete::ContactPropertyTmpl propPhoneMobile;

	enum List
	{
		FL,    // forward
		AL,    // allow
		BL,    // blocked
		RL     // reverse
	};

	virtual void deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	virtual AddContactPage *createAddContactWidget( QWidget *parent , KopeteAccount *i);
	virtual KopeteEditAccountWidget *createEditAccountWidget(KopeteAccount *account, QWidget *parent);
	virtual KopeteAccount *createNewAccount(const QString &accountId);

	static MSNProtocol* protocol();
	static bool validContactId(const QString&);

private slots:
	void slotSyncContactList();

private:

	static MSNProtocol *s_protocol;

signals:
	/**
	 * A new msn invitation has been arrived. plugins can connect this signal to handle invitations.
	 * if the invitationID match to their internal id. they can create a new MSNInvitation and pass it via invitation
	 *
	 * @param invitation should be set by the plugin to the new invitaiton. plugin should check it is equal to 0L before
	 * @param bodyMSG is the whole invitation message
	 * @param cookie is the invitation cookie
	 * @param msnMM is the message manager
	 * @param c is the contact
	 */
	void invitation(MSNInvitation*& invitation,  const QString &bodyMSG , long unsigned int cookie , MSNMessageManager* msnMM , MSNContact* c );
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

