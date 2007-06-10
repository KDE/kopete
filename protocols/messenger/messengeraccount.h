/*
 * messengeraccount.h - Windows Live Messenger Kopete Account.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#ifndef MESSENGERACCOUNT_H
#define MESSENGERACCOUNT_H

#include <kopetepasswordedaccount.h>

#define MESSENGER_DEFAULT_SERVER	"muser.messenger.hotmail.com"
#define MESSENGER_DEFAULT_PORT		1863
class KActionMenu;

namespace Kopete 
{ 
	class MetaContact;
	class StatusMessage;
}

class MessengerProtocol;

class MessengerAccountPrivate;

class MessengerAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT
public:
	MessengerAccount(MessengerProtocol *parent, const QString &accountId);
	~MessengerAccount();

	virtual KActionMenu *actionMenu();

public slots:
	virtual void connectWithPassword(const QString &password);
	virtual void disconnect();
	
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason = Kopete::StatusMessage());
	virtual void setStatusMessage(const Kopete::StatusMessage &statusMessage);
	
protected:
	virtual bool createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact);
	
private:
	MessengerAccountPrivate *d;
};
#endif
