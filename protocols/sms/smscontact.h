/*
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SMSCONTACT_H
#define SMSCONTACT_H

#include <qptrlist.h>

#include "kopetecontact.h"
#include "kopetehistorydialog.h"
#include "kopetegroup.h"
#include "smsprotocol.h"

class KAction;
class KActionCollection;

class KopeteHistoryDialog;
class KopeteMessageManager;


class SMSContact : public KopeteContact
{
	Q_OBJECT

public:
	SMSContact( SMSProtocol* protocol, const QString &smsId,
		const QString &displayName, KopeteMetaContact *parent );
	~SMSContact();

	virtual QString id() const;

	ContactStatus status() const;
	QString statusText() const;
	QString statusIcon() const;
	int importance() const;

	QString smsId() const;
	void setSmsId( const QString &id );

	virtual bool isReachable() { return true; };

public slots:

	void slotUnloading ( void );
	virtual void slotUserInfo();
	virtual void slotDeleteContact();
	virtual void execute();
	virtual void slotViewHistory();
	void slotSendFile();
	void slotSendMessage(const KopeteMessage &msg);

	void slotCloseHistoryDialog();

private:
	KopeteMessageManager* msgManager();

	QString m_smsId;
	SMSProtocol* m_protocol;

	KopeteHistoryDialog *historyDialog;

	KopeteMessageManager* mMsgManager;
};

#endif

