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

#include "smsservice.h"
#include "serviceloader.h"
#include "smsprotocol.h"
#include "smscontact.h"
#include "smsuserpreferences.h"
#include "smsglobal.h"

#include "kopetehistorydialog.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopete.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <kdialogbase.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

SMSContact::SMSContact( SMSProtocol *protocol, const QString &smsId,
	const QString &displayName, KopeteMetaContact *parent )
: KopeteContact( protocol, parent )
{
	historyDialog = 0L;

	m_smsId = smsId;

	setDisplayName( displayName );

	m_protocol = protocol;

	initActions();

	mMsgManager = 0L;
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
	SMSService* s;

	QString sName = SMSGlobal::readConfig("SMS", "ServiceName", m_smsId);

	s = ServiceLoader::loadService(sName, m_smsId);

	if ( s == 0L)
		return;

	if (s->send(nr, text))
		msgManager()->appendMessage(msg);

	delete s;
}

void SMSContact::slotViewHistory()
{
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
	delete historyDialog;
	historyDialog = 0L;
}

void SMSContact::slotUserInfo()
{
}

void SMSContact::slotDeleteContact()
{
	delete this;
	return;
}

KopeteContact::ContactStatus SMSContact::status() const
{
	return Unknown;
}

int SMSContact::importance() const
{
	return 20;
}

QString SMSContact::smsId() const
{
	return m_smsId;
}

void SMSContact::setSmsId( const QString id )
{
	m_smsId = id;
}

void SMSContact::initActions()
{
	actionCollection = 0L;
	actionPrefs = 0L;
}

KActionCollection* SMSContact::customContextMenuActions()
{
	if( actionCollection != 0L )
		delete actionCollection;
	if( actionPrefs != 0L )
		delete actionPrefs;
	
	actionCollection = new KActionCollection(this, "userColl");
	actionPrefs = new KAction(i18n("&User preferences"), 0, this, SLOT(userPrefs()), actionCollection, "userPrefs");
	actionCollection->insert(actionPrefs);
	return actionCollection;
}

void SMSContact::userPrefs()
{
	kdDebug() << "SMSContact::userPrefs()" << endl;
	SMSUserPreferences* p = new SMSUserPreferences( m_smsId );
	p->show();
	connect (p, SIGNAL(updateUserId(const QString)), this, SLOT(setSmsId(const QString)));
}

#include "smscontact.moc"



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

