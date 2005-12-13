/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SMSACCOUNT_H
#define SMSACCOUNT_H

#include "kopeteaccount.h"

class KActionMenu;
class SMSProtocol;
class SMSContact;
class SMSService;
class KProcess;

enum SMSMsgAction { ACT_ASK = 0, ACT_CANCEL, ACT_SPLIT };

class SMSAccount : public Kopete::Account
{
	Q_OBJECT

public:
	SMSAccount( SMSProtocol *parent, const QString &accountID, const char *name = 0L );
	~SMSAccount();

	virtual KActionMenu* actionMenu();			// Per-protocol actions for the systray and the status bar

	virtual void setAway( bool away, const QString & );

	void translateNumber(QString &theNumber);

	/**
	 * Checks to see if the message should be split or not, in case it is too long.
	 *
	 * Only ever call in case of message being too long - may result in user interaction.
	 */
	const bool splitNowMsgTooLong(int msgLength);

	SMSService* service();

public slots:
	void loadConfig();
	void setOnlineStatus( const Kopete::OnlineStatus& status , const QString &reason = QString::null);

public slots:
	virtual void connect(const Kopete::OnlineStatus& initial= Kopete::OnlineStatus());
	virtual void disconnect();
	virtual void slotSendMessage(Kopete::Message &msg);

protected slots:
	virtual void slotSendingSuccess(const Kopete::Message &msg);
	virtual void slotSendingFailure(const Kopete::Message &msg, const QString &error);
	virtual void slotConnected();
	virtual void slotDisconnected();
	

protected:
	bool createContact(const QString &contactId,  Kopete::MetaContact *parentContact);

private:
	bool theSubEnable;
	QString theSubCode;
	SMSMsgAction theLongMsgAction;
	SMSService* theService;
};

#endif
