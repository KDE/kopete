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

#undef KDE_NO_COMPAT
#include <kaction.h>
#include <kpopupmenu.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kopetemetacontact.h>

#include "kopeteuiglobal.h"

#include "smsaccount.h"
#include "smsprotocol.h"
#include "smscontact.h"

SMSAccount::SMSAccount( SMSProtocol *parent, const QString &accountID, const char *name )
	: Kopete::Account( parent, accountID, name )
{
	setMyself( new SMSContact(this, accountID, accountID, 0L) );
	loadConfig();
	connect();
}

SMSAccount::~SMSAccount()
{
}

void SMSAccount::loadConfig()
{
	theSubEnable = pluginData(protocol(), "SubEnable") == "true";
	theSubCode = pluginData(protocol(), "SubCode");
	theLongMsgAction = (SMSMsgAction)pluginData(protocol(), "MsgAction").toInt();
}

void SMSAccount::translateNumber(QString &theNumber)
{
	if(theNumber[0] == QChar('0') && theSubEnable)
		theNumber.replace(0, 1, theSubCode);
}

const bool SMSAccount::splitNowMsgTooLong(int max, int msgLength)
{
	if(theLongMsgAction == ACT_CANCEL) return false;
	if(theLongMsgAction == ACT_SPLIT) return true;
	if(KMessageBox::questionYesNo(Kopete::UI::Global::mainWidget(), i18n("This message is longer than the maximum length (%1). Should it be divided to %2 messages?").arg(max).arg(msgLength / max + 1),
		i18n("Message Too Long")) == KMessageBox::Yes)
		return true;
	else
		return false;
}

void SMSAccount::setAway( bool /*away*/, const QString &)
{
}

void SMSAccount::connect(const Kopete::OnlineStatus&)
{
	myself()->setOnlineStatus( SMSProtocol::protocol()->SMSOnline );
}

KActionMenu* SMSAccount::actionMenu()
{
	KActionMenu *theActionMenu = Kopete::Account::actionMenu();

	return theActionMenu;
}

void SMSAccount::disconnect()
{
	myself()->setOnlineStatus( SMSProtocol::protocol()->SMSOffline );
}

bool SMSAccount::createContact( const QString &contactId,
	Kopete::MetaContact * parentContact )
{
	if (new SMSContact(this, contactId, parentContact->displayName(), parentContact))
		return true;
	else
		return false;
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

#include "smsaccount.moc"
