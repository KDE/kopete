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

#include "msnsocket.h"

class KAction;
class KActionMenu;

class MSNContact;
class MSNIdentity;
class MSNPreferences;
class MSNNotifySocket;
class MSNSwitchBoardSocket;
class MSNMessageManager;
class KopeteMessageManager;
class KopeteMetaContact;
class KopeteContact;
class KopeteMessage;
class KopeteGroup;

/**
 * @author duncan
 * @author Martijn Klingens <klingens@kde.org>
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
	const KopeteOnlineStatus NLN;
	const KopeteOnlineStatus BSY;
	const KopeteOnlineStatus BRB;
	const KopeteOnlineStatus AWY;
	const KopeteOnlineStatus PHN;
	const KopeteOnlineStatus LUN;
	const KopeteOnlineStatus FLN;
	const KopeteOnlineStatus HDN;
	const KopeteOnlineStatus IDL;
	const KopeteOnlineStatus UNK;

	enum List
	{
		FL,    // forward
		AL,    // allow
		BL,    // blocked
		RL     // reverse
	};

	virtual void deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	virtual AddContactPage *createAddContactWidget( QWidget *parent );
	virtual EditIdentityWidget *createEditIdentityWidget(KopeteIdentity *identity, QWidget *parent);
	virtual KopeteIdentity *createNewIdentity(const QString &identityId);

	/**
	 * Returns a set of action items for the chatWindows
	 */
	KActionCollection * customChatActions(KopeteMessageManager * );

	virtual KActionMenu* protocolActions();
	virtual const QString protocolIcon();

	static MSNProtocol* protocol();

private slots:
	void slotSyncContactList();

private:
	KActionMenu *m_menu;

	static MSNProtocol *s_protocol;
	MSNPreferences *mPrefs;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

