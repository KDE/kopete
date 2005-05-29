/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/
#include "skypecontact.h"
#include "skypeaccount.h"
#include "skypeprotocol.h"
#include "skypechatsession.h"

#include <kdebug.h>
#include <qstring.h>
#include <kopetemessage.h>
#include <kopetechatsession.h>
#include <kopetechatsessionmanager.h>
#include <kopetecontactproperty.h>
#include <qptrlist.h>
#include <kaction.h>
#include <klocale.h>

typedef enum {
	osOffline,
	osOnline,
	osAway,
	osNA,
	osDND,
	osSkypeOut,
	osSkypeMe
} onlineStatus;

typedef enum {
	bsNotInList,
	bsNoAuth,
	bsInList
} buddyStatus;

class SkypeContactPrivate {
	public:
		///Full name of the contact
		QString fullName;
		///Acount that this contact belongs to
		SkypeAccount *account;
		///Is it some user or is it something special (myself contact or such)
		bool user;
		///Online status
		onlineStatus status;
		///Buddy status
		buddyStatus buddy;
		///The chat session
		SkypeChatSession *session;
		///The action to call the user
		KAction *callContactAction;
};

SkypeContact::SkypeContact(SkypeAccount *account, const QString &id, Kopete::MetaContact *parent, bool user)
	: Kopete::Contact(account, id, parent, QString::null) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d = new SkypeContactPrivate;//create the insides
	d->session = 0L;//no session yet
	d->account = account;//save the account for future, it will be needed
	account->prepareContact(this);//let the account prepare us
	d->user = user;
	d->callContactAction = 0L;
	connect(this, SIGNAL(setCallPossible(bool )), this, SLOT(enableCall(bool )));
	connect(this, SIGNAL(onlineStatusChanged(Kopete::Contact*,const Kopete::OnlineStatus&,const Kopete::OnlineStatus&)), this, SLOT(statusChanged()));
	if (account->canComunicate() && user)
		emit infoRequest(contactId());//retrieve information
}

SkypeContact::~SkypeContact() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	//free memory
	delete d;
}

Kopete::ChatSession *SkypeContact::manager(Kopete::Contact::CanCreateFlags CanCreate) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if ((!d->session) && (CanCreate)) {//It is not there and I can create it
		d->session = new SkypeChatSession(d->account, this);
		connect(d->session, SIGNAL(destroyed()), this, SLOT(removeChat()));//Care about loosing the session
		connect(d->session, SIGNAL(becameMultiChat()), this, SLOT(removeChat()));//This means it no longer belongs to this user
	}

	return d->session;//and return it
}

void SkypeContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	serializedData["contactId"] = contactId();//save the ID
}

void SkypeContact::requestInfo() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (d->user)
		emit infoRequest(contactId());//just ask for the info
}

void SkypeContact::setInfo(const QString &change) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	kdDebug(14311) << "info is: " << change << endl;//some debug info

	const QString &receivedProperty = change.section(' ', 0, 0).stripWhiteSpace().upper();//get the first part
	if (receivedProperty == "FULLNAME") {
		setProperty( Kopete::Global::Properties::self()->fullName(), change.section(' ', 1).stripWhiteSpace() );//save the name
	} else if (receivedProperty == "DISPLAYNAME") {
		const QString &name = change.section(' ', 1).stripWhiteSpace();//get the name
		if (name.isEmpty())
			setNickName( property( Kopete::Global::Properties::self()->fullName() ).value().toString() );
		else
			setNickName(name);//set the display name
	} else if (receivedProperty == "ONLINESTATUS") {//The online status eather changed or we just logged in and I asked for it
		const QString &status = change.section(' ', 1, 1).stripWhiteSpace().upper();//get the status

		if (status == "OFFLINE") {
			d->status = osOffline;
		} else if (status == "ONLINE") {
			d->status = osOnline;
		} else if (status == "AWAY") {
			d->status = osAway;
		} else if (status == "NA") {
			d->status = osNA;
		} else if (status == "DND") {
			d->status = osDND;	kdDebug(14311) << k_funcinfo << endl;//some debug info

		} else if (status == "SKYPEOUT") {
			d->status = osSkypeOut;
		} else if (status == "SKYPEME") {
			d->status = osSkypeMe;
		}

		resetStatus();
	} else if (receivedProperty == "BUDDYSTATUS") {
		int value = change.section(' ', 1, 1).stripWhiteSpace().toInt();//get the value

		switch (value) {
			case 0:
				d->buddy = bsNotInList;
				break;
			case 1:
				deleteLater();
				return;
			case 2:
				d->buddy = bsNoAuth;
				break;
			case 3:
				d->buddy = bsInList;
				break;
		}

		resetStatus();
	} else
	{
		QString propValue = change.section(' ', 1).stripWhiteSpace();
		if ( !propValue.isEmpty() )
		{
			if ( receivedProperty == "PHONE_HOME" ) {
				setProperty( d->account->protocol()->propPrivatePhone, change.section(' ', 1).stripWhiteSpace() );
			} else if ( receivedProperty == "PHONE_OFFICE" ) {
				setProperty( d->account->protocol()->propWorkPhone, change.section(' ', 1).stripWhiteSpace() );
			} else if ( receivedProperty == "PHONE_MOBILE" ) {
				setProperty( d->account->protocol()->propPrivateMobilePhone, change.section(' ', 1).stripWhiteSpace() );
			} else if ( receivedProperty == "HOMEPAGE" ) {
				setProperty( d->account->protocol()->propPrivateMobilePhone, change.section(' ', 1).stripWhiteSpace() );
			}
		}
	}
}

QString SkypeContact::formattedName() const {
	return d->fullName;
}

void SkypeContact::resetStatus() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	SkypeProtocol * protocol = d->account->protocol();//get the protocol

	if (d->status == osSkypeOut) {
		setOnlineStatus(protocol->Phone);//this is the SkypeOut contact, in many ways special
		return;
	}

	switch (d->buddy) {
		case bsNotInList:
			setOnlineStatus(protocol->NotInList);
			return;
		case bsNoAuth:
			setOnlineStatus(protocol->NoAuth);
			return;
		case bsInList://just put there normal status
			break;
	}

	switch (d->status) {
		case osOffline:
			setOnlineStatus(protocol->Offline);
			break;
		case osOnline:
			setOnlineStatus(protocol->Online);
			break;
		case osAway:
			setOnlineStatus(protocol->Away);
			break;
		case osNA:
			setOnlineStatus(protocol->NotAvailable);
			break;
		case osDND:
			setOnlineStatus(protocol->DoNotDisturb);
			break;
		case osSkypeOut:
			setOnlineStatus(protocol->Phone);
			break;
		case osSkypeMe:
			setOnlineStatus(protocol->SkypeMe);
			break;
	}
}

bool SkypeContact::isReachable() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	switch (d->buddy) {
		case bsNotInList:
		case bsNoAuth://I do not know, weather he is online, but I will send it trough the server
			return true;
		case bsInList:
			break;//Do it by online status
	}

	switch (d->status) {
		case osOffline://he is offline
		case osSkypeOut://This one can not get messages, it is skype-out contact
			return false;
		default://some kind of online
			return true;
	}
}

void SkypeContact::removeChat() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->session = 0L;//it exists no more or it is no longer of this contact
}

bool SkypeContact::hasChat() const {
	return d->session;//does it have a chat session?
}

void SkypeContact::receiveIm(const QString &message) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (!hasChat()) {
		manager(CanCreate);//create it
		if (!hasChat())//something failed
			return;
	}

	Kopete::Message mes(this, d->account->myself(), message, Kopete::Message::Inbound);//create the message
	d->session->appendMessage(mes);//add it to the session
}

QPtrList<KAction> *SkypeContact::customContextMenuActions() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	QPtrList<KAction> *actions = new QPtrList<KAction>();

	if (!d->callContactAction) {
		d->callContactAction = new KAction(i18n("Call Contact"), "call_contact", KShortcut(), this, SLOT(call()), this, "call_contact");
		statusChanged();//This one takes care of disabling/enabling this action depending on the user's status.
	}

	actions->append(d->callContactAction);

	return actions;
}

void SkypeContact::enableCall(bool value) {
	if (d->callContactAction)
		d->callContactAction->setEnabled(value);
}

void SkypeContact::statusChanged() {
	const Kopete::OnlineStatus &myStatus = onlineStatus();
	SkypeProtocol * protocol = d->account->protocol();
	if ((myStatus == protocol->Online) || (myStatus == protocol->Away) || (myStatus == protocol->NotAvailable) || (myStatus == protocol->DoNotDisturb) || (myStatus == protocol->NoAuth) || (myStatus == protocol->NotInList) || (myStatus == protocol->Phone) || (myStatus == protocol->SkypeMe))
		emit setCallPossible(true);
	else
		emit setCallPossible(false);
}

void SkypeContact::call() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->account->makeCall(this);
}

void SkypeContact::connectionStatus(bool connected) {
	if (connected) {
		statusChanged();
	} else
		emit setCallPossible(false);
}

#include "skypecontact.moc"
