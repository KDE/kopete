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
#include "smsglobal.h"

#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"
#include "kopete.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <kdialogbase.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

SMSContact::SMSContact( SMSProtocol *protocol, const QString &phoneNumber,
	const QString &displayName, KopeteMetaContact *parent )
: KopeteContact( protocol, parent )
{
	setPhoneNumber( phoneNumber );
	setDisplayName( displayName );
	m_serviceName = QString::null;
	m_protocol = protocol;

	initActions();

	m_msgManager = 0L;
}

SMSContact::~SMSContact()
{
}


QString SMSContact::id() const
{
	return m_phoneNumber;
}

void SMSContact::execute()
{
	msgManager()->readMessages();
}

KopeteMessageManager* SMSContact::msgManager()
{
	if ( m_msgManager )
	{
		return m_msgManager;
	}
	else
	{
		QPtrList<KopeteContact> contacts;
		contacts.append(this);
		m_msgManager = KopeteMessageManagerFactory::factory()->create(m_protocol->myself(), contacts, m_protocol, KopeteMessageManager::Email);
		connect(m_msgManager, SIGNAL(messageSent(const KopeteMessage&, KopeteMessageManager*)),
		this, SLOT(slotSendMessage(const KopeteMessage&)));
		return m_msgManager;
	}
}

void SMSContact::slotSendMessage(const KopeteMessage &msg)
{
	QString sName = SMSGlobal::readConfig("SMS", "ServiceName", this);

	SMSService* s = ServiceLoader::loadService( sName, this );

	if ( s == 0L)
		return;

	connect ( s, SIGNAL(messageSent(const KopeteMessage&)),
		this, SLOT(messageSent(const KopeteMessage&)));

	int msgLength = msg.plainBody().length();

	if (s->maxSize() == -1)
		s->send(msg);
	else if (s->maxSize() < msgLength)
	{
		int res = KMessageBox::questionYesNo( 0L, i18n("This message is longer than the maximum length (%1). Should it be divided to %2 messages?").arg(s->maxSize()).arg(msgLength/(s->maxSize())+1), i18n("Message too long") );
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

void SMSContact::messageSent(const KopeteMessage& msg)
{
	msgManager()->appendMessage(msg);
}

void SMSContact::slotUserInfo()
{
}

void SMSContact::slotDeleteContact()
{
	deleteLater();
}

KopeteContact::ContactStatus SMSContact::status() const
{
	return Unknown;
}

int SMSContact::importance() const
{
	return 20;
}

QString SMSContact::phoneNumber()
{
	return m_phoneNumber;
}

void SMSContact::setPhoneNumber( const QString phoneNumber )
{
	m_phoneNumber = phoneNumber;
}

QString SMSContact::serviceName()
{
	return m_serviceName;
}

void SMSContact::setServiceName(QString name)
{
	m_serviceName = name;
}


QString SMSContact::servicePref(QString name)
{
	return m_servicePrefs[name];
}

void SMSContact::setServicePref(QString name, QString value)
{
	m_servicePrefs.insert(name, value);
}

void SMSContact::deleteServicePref(QString name)
{
	m_servicePrefs.erase(name);
}

void SMSContact::clearServicePrefs()
{
	m_servicePrefs.clear();
	m_serviceName = QString::null;
}

QString SMSContact::servicePrefsString()
{
	if (m_servicePrefs.count() > 0)
	{
		QStringList prefs;

		for (QMap<QString, QString>::Iterator it = m_servicePrefs.begin();
				it != m_servicePrefs.end(); ++it)
		{
			prefs.append(QString("%1=%2").arg(it.key()).arg(it.data()));
		}

		return prefs.join(";");
	}
	else
		return QString::null;
}

void SMSContact::setServicePrefsString(QString servicePrefs)
{
	QStringList prefs;
	prefs = QStringList::split(";", servicePrefs);

	for ( QStringList::Iterator it = prefs.begin(); it != prefs.end(); ++it)
	{
		QRegExp r("(.*)=(.*)");
		r.search(*it);
		setServicePref(r.cap(1), r.cap(2));
	}
}

void SMSContact::initActions()
{
	m_actionCollection = 0L;
	m_actionPrefs = new KAction(i18n("&User preferences"), 0, this, SLOT(userPrefs()), m_actionCollection, "userPrefs");
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



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

