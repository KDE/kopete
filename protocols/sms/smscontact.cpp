/*
  smscontact.cpp  -  SMS Plugin Contact

  Copyright (c) 2003      by Richard Lärkäng        <nouseforaname@home.se>
  Copyright (c) 2003      by Gav Wood               <gav@kde.org>

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

#undef KDE_NO_COMPAT
#include <kconfigbase.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

#include "kopetechatsessionmanager.h"
#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include "smsprotocol.h"
#include "smsservice.h"
#include "smsaccount.h"
#include "smsuserpreferences.h"

SMSContact::SMSContact( Kopete::Account* _account, const QString &phoneNumber,
	Kopete::MetaContact *parent )
: Kopete::Contact( _account, phoneNumber, parent ), m_phoneNumber( phoneNumber )
{
//	kWarning( 14160 ) << " this = " << this << ", phone = " << phoneNumber;

	m_msgManager = 0L;
	m_actionPrefs = 0L;

	if( account()->isConnected() )
		setOnlineStatus( SMSProtocol::protocol()->SMSOnline );
}

void SMSContact::slotSendingSuccess(const Kopete::Message &msg)
{
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
	manager(Kopete::Contact::CanCreate)->appendMessage((Kopete::Message &)msg);
}

void SMSContact::slotSendingFailure(const Kopete::Message &/*msg*/, const QString &error)
{
	KMessageBox::detailedError(Kopete::UI::Global::mainWidget(), i18n("Something went wrong while sending the message."), error,
			i18n("Could Not Send Message"));
//	manager()->messageFailed();
	// TODO: swap for failed as above. show it anyway for now to allow closing of window.
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void SMSContact::serialize( QMap<QString, QString> &serializedData,
	QMap<QString, QString> & /* addressBookData */ )
{
	// Contact id and display name are already set for us
	if (m_phoneNumber != contactId())
		serializedData[ "contactId" ] = m_phoneNumber;
}

Kopete::ChatSession* SMSContact::manager( Kopete::Contact::CanCreateFlags canCreate  )
{
	if ( m_msgManager || canCreate != Kopete::Contact::CanCreate )
	{
		return m_msgManager;
	}
	else
	{
		QList<Kopete::Contact*> contacts;
		contacts.append(this);
		m_msgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), contacts, protocol());
		connect(m_msgManager, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
			account(), SLOT(slotSendMessage(Kopete::Message&)));
		connect(m_msgManager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()));
		return m_msgManager;
	}
}

void SMSContact::slotChatSessionDestroyed()
{
	m_msgManager = 0L;
}


void SMSContact::slotUserInfo()
{
}

void SMSContact::deleteContact()
{
	deleteLater();
}

const QString SMSContact::qualifiedNumber()
{
	QString number = m_phoneNumber;
	dynamic_cast<SMSAccount *>(account())->translateNumber(number);
	return number;
}

const QString &SMSContact::phoneNumber()
{
	return m_phoneNumber;
}

void SMSContact::setPhoneNumber( const QString phoneNumber )
{
	deleteLater();
	new SMSContact(account(), phoneNumber, metaContact());
}

QList<KAction *>* SMSContact::customContextMenuActions()
{
	QList<KAction*> *actions = new QList<KAction*>();

	if( !m_actionPrefs )
	{
		m_actionPrefs = new KAction(i18n("&Contact Settings"),
					    this );

//       dynamic_cast<QObject*>(this),
//          SLOT(userPrefs()),
//          dynamic_cast<QObject*>(this),
//          "userPrefs");
		QObject::connect(m_actionPrefs,
				 SIGNAL(triggered(bool)),
				 dynamic_cast<QObject*>(this),
				 SLOT(userPrefs()));
	}

	actions->append(m_actionPrefs);

	return actions;
}

void SMSContact::userPrefs()
{
	SMSUserPreferences* p = new SMSUserPreferences( this );
	p->show();
}

#include "smscontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

