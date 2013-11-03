/*
  smsaccount.cpp  -  SMS Plugin Account

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

#undef KDE_NO_COMPAT

#include "smsaccount.h"

#include <kconfigbase.h>
#include <kaction.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kopetemetacontact.h>
#include <kopetecontactlist.h>

#include "kopeteuiglobal.h"

#include "serviceloader.h"

#include "smsprotocol.h"
#include "smscontact.h"

SMSAccount::SMSAccount( SMSProtocol *parent, const QString &accountID, const char *name )
	: Kopete::Account( parent, accountID )
{
	Q_UNUSED(name);

	setMyself( new SMSContact(this, accountID, Kopete::ContactList::self()->myself()) );
	loadConfig();
	myself()->setOnlineStatus( SMSProtocol::protocol()->SMSOffline );

	QString sName = configGroup()->readEntry("ServiceName", QString());
	theService = ServiceLoader::loadService(sName, this);
	
	if( theService )
	{
		QObject::connect (theService, SIGNAL(messageSent(Kopete::Message)), 
					this, SLOT(slotSendingSuccess(Kopete::Message)));
		QObject::connect (theService, SIGNAL(messageNotSent(Kopete::Message,QString)), 
					this, SLOT(slotSendingFailure(Kopete::Message,QString)));
		QObject::connect (theService, SIGNAL(connected()), this, SLOT(slotConnected()));
		QObject::connect (theService, SIGNAL(disconnected()), this, SLOT(slotDisconnected()));
	}
	
}

SMSAccount::~SMSAccount()
{
	delete theService;
	theService = NULL;
}

void SMSAccount::loadConfig()
{
	theSubEnable = configGroup()->readEntry("SubEnable", false);
	theSubCode = configGroup()->readEntry("SubCode", QString());
	theLongMsgAction = (SMSMsgAction)configGroup()->readEntry("MsgAction", 0);
}

void SMSAccount::translateNumber(QString &theNumber)
{
	if(theNumber[0] == QChar('0') && theSubEnable)
		theNumber.replace(0, 1, theSubCode);
}

bool SMSAccount::splitNowMsgTooLong(int msgLength) const
{
	if( theService == NULL )
		return false;
	
	int max = theService->maxSize();
	if(theLongMsgAction == ACT_CANCEL) return false;
	if(theLongMsgAction == ACT_SPLIT) return true;
	if(KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(), i18n("This message is longer than the maximum length (%1). Should it be divided to %2 messages?", max, msgLength / max + 1),
		i18n("Message Too Long"), KGuiItem( i18n("Divide") ), KGuiItem( i18n("Do Not Divide") )) == KMessageBox::Yes)
		return true;
	else
		return false;
}

void SMSAccount::setAway( bool /*away*/, const QString &)
{
}

void SMSAccount::connect(const Kopete::OnlineStatus&)
{
	myself()->setOnlineStatus( SMSProtocol::protocol()->SMSConnecting );
	if( theService )
		theService->connect();
}

void SMSAccount::slotConnected()
{
	myself()->setOnlineStatus( SMSProtocol::protocol()->SMSOnline );
	setAllContactsStatus( SMSProtocol::protocol()->SMSOnline );
}

void SMSAccount::disconnect()
{
	if( theService )
		theService->disconnect();
}

void SMSAccount::slotDisconnected()
{
	myself()->setOnlineStatus( SMSProtocol::protocol()->SMSOffline );
	setAllContactsStatus( SMSProtocol::protocol()->SMSOffline );
}

void SMSAccount::slotSendMessage(Kopete::Message &msg)
{
	kWarning( 14160 ) << " this = " << this;

	if(theService == 0L)
		return;

	int msgLength = msg.plainBody().length();

	if( theService->maxSize() == -1 )
	{
		theService->send(msg);
	}
	else if( theService->maxSize() < msgLength )
	{
		if( splitNowMsgTooLong(msgLength) )
		{
			for (int i=0; i < msgLength / theService->maxSize() + 1; i++)
			{
				QString text = msg.plainBody();
				text = text.mid( theService->maxSize() * i, theService->maxSize() );
				Kopete::Message m( msg.from(), msg.to() );
				m.setPlainBody( text );
				m.setDirection( Kopete::Message::Outbound );
				
				theService->send(m);
			}
		}
		else
			slotSendingFailure(msg, i18n("Message too long."));
	}
	else
	{
		theService->send(msg);
	}

}

void SMSAccount::slotSendingSuccess(const Kopete::Message &msg)
{
	SMSContact* c = dynamic_cast<SMSContact*>(msg.to().first());
	if( c )
		c->slotSendingSuccess(msg);
}

void SMSAccount::slotSendingFailure(const Kopete::Message &msg, const QString &error)
{
	SMSContact* c = dynamic_cast<SMSContact*>(msg.to().first());
	if( c )
		c->slotSendingFailure(msg, error);
}

bool SMSAccount::createContact( const QString &contactId,
	Kopete::MetaContact * parentContact )
{
	if (new SMSContact(this, contactId, parentContact))
		return true;
	else
		return false;
}

void SMSAccount::fillActionMenu( KActionMenu *actionMenu )
{
	Kopete::Account::fillActionMenu( actionMenu );
}

void SMSAccount::setOnlineStatus( const Kopete::OnlineStatus & status , const Kopete::StatusMessage &reason, const OnlineStatusOptions& /*options*/ )
{
	if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Online )
		connect();
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Offline )
		disconnect();
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Away )
		setAway( true, reason.message() );
}

void SMSAccount::setStatusMessage( const Kopete::StatusMessage& msg )
{
	setOnlineStatus( myself()->onlineStatus(), msg, Kopete::Account::KeepSpecialFlags );
}

SMSService* SMSAccount::service()
{
	return theService;
}

#include "smsaccount.moc"
