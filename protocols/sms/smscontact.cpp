/*
    Copyright (c) 2002 Duncan Mac-Vicar Prett <duncan@kde.org>
              (c) 2002 Ryan Cumming           <bodnar42@phalynx.dhs.org>
              (c) 2002 Martijn Klingens       <klingens@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qlineedit.h>
#include <qcheckbox.h>
#include <kdialogbase.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopete.h"
#include "kopetecontactlistview.h"
#include "kopetestdaction.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "smscontact.h"
#include "smsprotocol.h"
#include "kopetewindow.h"
#include "kopeteemailwindow.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "smsservice.h"

SMSContact::SMSContact( SMSProtocol *protocol, const QString &smsId,
	const QString &displayName, KopeteMetaContact *parent )
: KopeteContact( protocol, parent )
{
	historyDialog = 0L;

	m_smsId = smsId;

	setDisplayName( displayName );

	m_protocol = protocol;

}

SMSContact::~SMSContact()
{
}


QString SMSContact::id() const
{
	return m_smsId;
}

void SMSContact::execute()
{
	msgManager()->readMessages();
}

KopeteMessageManager* SMSContact::msgManager()
{
	if ( mMsgManager )
	{
		return mMsgManager;
	}
	else
	{
		QPtrList<KopeteContact> contacts;
		contacts.append(this);
		mMsgManager = kopeteapp->sessionFactory()->create(m_protocol->myself(), contacts, m_protocol, "sms_logs/" + m_smsId + ".log", KopeteMessageManager::Email);
		connect(mMsgManager, SIGNAL(messageSent(const KopeteMessage&, KopeteMessageManager*)),
		this, SLOT(slotSendMessage(const KopeteMessage&)));
		return mMsgManager;
	}
}

void SMSContact::slotSendMessage(const KopeteMessage &msg)
{
	QString text = msg.plainBody();
	QString nr = id();
	SMSService* s = new SMSService;
	s->send(nr, text);

	msgManager()->appendMessage(msg);
}

void SMSContact::slotViewHistory()
{
	kdDebug() << "SMS Plugin: slotViewHistory()" << endl;

	if (historyDialog != 0L)
	{
		historyDialog->raise();
	}
	else
	{
		historyDialog = new KopeteHistoryDialog(QString("sms_logs/%1.log").arg(m_smsId), displayName(), true, 50, 0, "SMSHistoryDialog");
		connect ( historyDialog, SIGNAL(closing()), this, SLOT(slotCloseHistoryDialog()) );
	}
}

void SMSContact::slotCloseHistoryDialog()
{
	kdDebug() << "SMS Plugin: slotCloseHistoryDialog()" << endl;
	delete historyDialog;
	historyDialog = 0L;
}

void SMSContact::slotUserInfo()
{
}

void SMSContact::slotSendFile()
{
	KMessageBox::sorry(kopeteapp->mainWindow(), i18n("Sorry, you can't send a file with SMS"), i18n("Cant' send file"));
}

void SMSContact::slotDeleteContact()
{
	kdDebug() << "SMSContact::slotDeleteContact" << endl;

	delete this;
	return;
}

KopeteContact::ContactStatus SMSContact::status() const
{
	return Online;
}

QString SMSContact::statusText() const
{
	return i18n("Online");
}

QString SMSContact::statusIcon() const
{
	return "";
}

int SMSContact::importance() const
{
	return 20;
}

QString SMSContact::smsId() const
{
	return m_smsId;
}

void SMSContact::setSmsId( const QString &id )
{
	m_smsId = id;
}

#include "smscontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

