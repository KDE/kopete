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

#include "kopetecontact.h"

#include <qstring.h>

class SMSProtocol;
class KopeteHistoryDialog;
class KopeteMessageManager;
class KopeteMetaContact;

class KActionCollection;
class KAction;


class SMSContact : public KopeteContact
{
	Q_OBJECT

public:
	SMSContact( SMSProtocol* protocol, const QString &smsId,
		const QString &displayName, KopeteMetaContact *parent );
	~SMSContact();

	virtual QString id() const;

	KActionCollection* customContextMenuActions();

	ContactStatus status() const;
	int importance() const;

	QString smsId() const;

	virtual bool isReachable() { return true; };

public slots:

	virtual void slotUserInfo();
	virtual void slotDeleteContact();
	virtual void execute();
	virtual void slotViewHistory();
	void slotSendMessage(const KopeteMessage &msg);
	void setSmsId( const QString id );

	void slotCloseHistoryDialog();

private slots:
	void userPrefs();

private:
	KopeteMessageManager* msgManager();

	void initActions();
	KActionCollection* actionCollection;
	KAction* actionPrefs;

	QString m_smsId;
	SMSProtocol* m_protocol;

	KopeteHistoryDialog *historyDialog;

	KopeteMessageManager* mMsgManager;
};

#endif

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

