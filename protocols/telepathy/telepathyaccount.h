/*
 * telepathyaccount.h - Telepathy Kopete Account.
 *
 * Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>
 * 
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
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
#ifndef TELEPATHYACCOUNT_H
#define TELEPATHYACCOUNT_H

#include <kopeteaccount.h>


class KActionMenu;

namespace Kopete 
{ 
	class MetaContact;
	class StatusMessage;
}

class TelepathyProtocol;

/**
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class TelepathyAccount : public Kopete::Account
{
	Q_OBJECT
public:
	TelepathyAccount(TelepathyProtocol *parent, const QString &accountId);
	~TelepathyAccount();

	virtual KActionMenu *actionMenu();

public slots:
	virtual void connect(const Kopete::OnlineStatus& initialStatus = Kopete::OnlineStatus());
	virtual void disconnect();
	
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason = Kopete::StatusMessage());
	virtual void setStatusMessage(const Kopete::StatusMessage &statusMessage);
	
protected:
	virtual bool createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact);
	
private:
	
};
#endif
