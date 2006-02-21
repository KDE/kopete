/*  *************************************************************************
    *   copyright: (C) 2003 Richard L�k�g <nouseforaname@home.se>         *
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

#undef KDE_NO_COMPAT

#include <kconfigbase.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kopetemetacontact.h>
#include <kopetecontactlist.h>

#include "kopeteuiglobal.h"

#include "serviceloader.h"

#include "smsaccount.h"
#include "smsprotocol.h"
#include "smscontact.h"

SMSAccount::SMSAccount( SMSProtocol *parent, const QString &accountID, const char *name )
	: Kopete::Account( parent, accountID, name )
{
	setMyself( new SMSContact(this, accountID, accountID, Kopete::ContactList::self()->myself()) );
	loadConfig();
	myself()->setOnlineStatus( SMSProtocol::protocol()->SMSOffline );

	QString sName = configGroup()->readEntry("ServiceName", QString::null);
	theService = ServiceLoader::loadService(sName, this);
	
	if( theService )
	{
		QObject::connect (theService, SIGNAL(messageSent(const Kopete::Message &)), 
					this, SLOT(slotSendingSuccess(const Kopete::Message &)));
		QObject::connect (theService, SIGNAL(messageNotSent(const Kopete::Message &, const QString &)), 
					this, SLOT(slotSendingFailure(const Kopete::Message &, const QString &)));
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
	theSubEnable = configGroup()->readBoolEntry("SubEnable", false);
	theSubCode = configGroup()->readEntry("SubCode", QString::null);
	theLongMsgAction = (SMSMsgAction)configGroup()->readNumEntry("MsgAction", 0);
}

void SMSAccount::translateNumber(QString &theNumber)
{
	if(theNumber[0] == QChar('0') && theSubEnable)
		theNumber.replace(0, 1, theSubCode);
}

const bool SMSAccount::splitNowMsgTooLong(int msgLength)
{
	if( theService == NULL )
		return false;
	
	int max = theService->maxSize();
	if(theLongMsgAction == ACT_CANCEL) return false;
	if(theLongMsgAction == ACT_SPLIT) return true;
	if(KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(), i18n("This message is longer than the maximum length (%1). Should it be divided to %2 messages?").arg(max).arg(msgLength / max + 1),
		i18n("Message Too Long"), i18n("Divide"), i18n("Do Not Divide")) == KMessageBox::Yes)
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
	kdWarning( 14160 ) << k_funcinfo << " this = " << this << endl;

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
				Kopete::Message m( msg.from(), msg.to(), text, Kopete::Message::Outbound);
				
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
	if (new SMSContact(this, contactId, parentContact->displayName(), parentContact))
		return true;
	else
		return false;
}

KActionMenu* SMSAccount::actionMenu()
{
	KActionMenu *theActionMenu = Kopete::Account::actionMenu();
	return theActionMenu;
}

void SMSAccount::setOnlineStatus( const Kopete::OnlineStatus & status , const QString &reason)
{
	if ( myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Online )
		connect();
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Offline )
		disconnect();
	else if ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline && status.status() == Kopete::OnlineStatus::Away )
		setAway( true, reason );
}

SMSService* SMSAccount::service()
{
	return theService;
}

#include "smsaccount.moc"
