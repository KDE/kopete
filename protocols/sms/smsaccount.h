/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SMSACCOUNT_H
#define SMSACCOUNT_H

#include "kopeteaccount.h"

class SMSProtocol;
class SMSContact;

class SMSAccount : public KopeteAccount
{
	Q_OBJECT

public:
	SMSAccount( SMSProtocol *parent, const QString &accountID, const char *name = 0L );
	~SMSAccount();

	virtual KActionMenu* actionMenu();			// Per-protocol actions for the systray and the status bar

	virtual void setAway( bool away, const QString & );
	virtual KopeteContact* myself() const;

public slots:
	virtual void connect();
	virtual void disconnect();

protected:
	bool addContactToMetaContact( const QString &contactId, const QString &displayName,
		KopeteMetaContact *parentContact );

private:
	SMSContact* m_myself;
};

#endif
