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

#include "smscontact.h"
#include "smsservice.h"
#include "serviceloader.h"
#include "smsprotocol.h"
#include "smsuserpreferences.h"
#include "smsaccount.h"

#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopeteaccount.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <kdialogbase.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

SMSContact::SMSContact( KopeteAccount* _account, const QString &phoneNumber,
	const QString &displayName, KopeteMetaContact *parent )
: KopeteContact( _account, phoneNumber, parent )
{
	setPhoneNumber( phoneNumber );
	setDisplayName( displayName );

	m_actionCollection = 0L;
	m_actionPrefs = new KAction(i18n("&User Preferences"), 0, this, SLOT(userPrefs()), m_actionCollection, "userPrefs");

	m_msgManager = 0L;

	setOnlineStatus( SMSProtocol::protocol()->SMSUnknown );
}

void SMSContact::serialize( QMap<QString, QString> &serializedData,
	QMap<QString, QString> & /* addressBookData */ )
{
	// Contact id and display name are already set for us
	if (m_phoneNumber != contactId())
		serializedData[ "contactId" ] = m_phoneNumber;
}

KopeteMessageManager* SMSContact::manager( bool )
{
	if ( m_msgManager )
	{
		return m_msgManager;
	}
	else
	{
		QPtrList<KopeteContact> contacts;
		contacts.append(this);
		m_msgManager = KopeteMessageManagerFactory::factory()->create(account()->myself(), contacts, protocol());
		connect(m_msgManager, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager*)),
		this, SLOT(slotSendMessage(KopeteMessage&)));
		connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotMessageManagerDestroyed()));
		connect(this, SIGNAL(messageSuccess()), m_msgManager, SIGNAL(messageSuccess()));
		return m_msgManager;
	}
}

void SMSContact::slotMessageManagerDestroyed()
{
	m_msgManager = 0L;
}

void SMSContact::slotSendMessage(KopeteMessage &msg)
{
	QString sName = account()->pluginData(protocol(), "ServiceName");

	SMSService* s = ServiceLoader::loadService( sName, account() );

	if ( s == 0L)
		return;

	connect ( s, SIGNAL(messageSent(const KopeteMessage&)),
		this, SLOT(messageSent(KopeteMessage&)));

	int msgLength = msg.plainBody().length();

	if (s->maxSize() == -1)
		s->send(msg);
	else if (s->maxSize() < msgLength)
	{
		int res = KMessageBox::questionYesNo( 0L, i18n("This message is longer than the maximum length (%1). Should it be divided to %2 messages?").arg(s->maxSize()).arg(msgLength/(s->maxSize())+1), i18n("Message Too Long") );
		switch (res)
		{
		case KMessageBox::Yes:
			for (int i=0; i < (msgLength/(s->maxSize())+1); i++)
			{
				QString text = msg.plainBody();
				text = text.mid( (s->maxSize())*i, s->maxSize() );
				KopeteMessage m( msg.from(), msg.to(), text, KopeteMessage::Outbound);
				s->send(m);
			}
			break;
		case KMessageBox::No:
			break;
		default:
			break;
		}
	}
	else
		s->send(msg);

	delete s;
}

void SMSContact::messageSent(KopeteMessage& msg)
{
	manager()->appendMessage(msg);
}

void SMSContact::slotUserInfo()
{
}

void SMSContact::slotDeleteContact()
{
	deleteLater();
}

QString SMSContact::phoneNumber()
{
	return m_phoneNumber;
}

void SMSContact::setPhoneNumber( const QString phoneNumber )
{
	m_phoneNumber = phoneNumber;
}

KActionCollection* SMSContact::customContextMenuActions()
{
	if( m_actionCollection != 0L )
		delete m_actionCollection;

	m_actionCollection = new KActionCollection(this, "userColl");
	m_actionCollection->insert(m_actionPrefs);
	return m_actionCollection;
}

void SMSContact::userPrefs()
{
	SMSUserPreferences* p = new SMSUserPreferences( this );
	p->show();
}

#include "smscontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

