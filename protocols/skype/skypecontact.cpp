/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <vorner@seznam.cz>

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
#include <kopetemessagemanager.h>
#include <kopetemessagemanagerfactory.h>

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
};

SkypeContact::SkypeContact(SkypeAccount *account, const QString &id, Kopete::MetaContact *parent, bool user)
	: Kopete::Contact(account, id, parent, QString::null) {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	d = new SkypeContactPrivate;//create the insides
	d->session = 0L;//no session yet
	d->account = account;//save the account for future, it will be needed
	account->prepareContact(this);//let the account prepare us
	d->user = user;
	if (account->canComunicate() && user)
		emit infoRequest(contactId());//retrieve information
}

SkypeContact::~SkypeContact() {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	//free memory
	delete d;
}

Kopete::ChatSession *SkypeContact::manager(Kopete::Contact::CanCreateFlags CanCreate) {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	if ((!d->session) && (CanCreate)) {//It is not there and I can create it
		d->session = new SkypeChatSession(d->account, this);
		connect(d->session, SIGNAL(destroyed()), this, SLOT(removeChat()));//Care about loosing the session
		connect(d->session, SIGNAL(becameMultiChat()), this, SLOT(removeChat()));//This means it no longer belongs to this user
	}

	return d->session;//and return it
}

void SkypeContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData) {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	serializedData["contactId"] = contactId();//save the ID
}

void SkypeContact::requestInfo() {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	if (d->user)
		emit infoRequest(contactId());//just ask for the info
}

void SkypeContact::setInfo(const QString &change) {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	const QString &property = change.section(' ', 0, 0).stripWhiteSpace().upper();//get the first part
	if (property == "FULLNAME") {
		d->fullName = change.section(' ', 1).stripWhiteSpace();//save the name
	} else if (property == "DISPLAYNAME") {
		const QString &name = change.section(' ', 1).stripWhiteSpace();//get the name
		if (name.isEmpty())
			setNickName(d->fullName);//no display name provided, use full name as a replace (if it is empty as well, kopete will use the id instead automatically)
		else
			setNickName(name);//set the display name
	} else if (property == "ONLINESTATUS") {//The online status eather changed or we just logged in and I asked for it
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
			d->status = osDND;	kdDebug(65320) << k_funcinfo << endl;//some debug info

		} else if (status == "SKYPEOUT") {
			d->status = osSkypeOut;
		} else if (status == "SKYPEME") {
			d->status = osSkypeMe;
		}

		resetStatus();
	} else if (property == "BUDDYSTATUS") {
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
	}
}

QString SkypeContact::formattedName() const {
	return d->fullName;
}

void SkypeContact::resetStatus() {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	SkypeProtocol &protocol = d->account->protocol();//get the protocol

	if (d->status == osSkypeOut) {
		setOnlineStatus(protocol.Phone);//this is the SkypeOut contact, in many ways special
		return;
	}

	switch (d->buddy) {
		case bsNotInList:
			setOnlineStatus(protocol.NotInList);
			return;
		case bsNoAuth:
			setOnlineStatus(protocol.NoAuth);
			return;
		case bsInList://just put there normal status
			break;
	}

	switch (d->status) {
		case osOffline:
			setOnlineStatus(protocol.Offline);
			break;
		case osOnline:
			setOnlineStatus(protocol.Online);
			break;
		case osAway:
			setOnlineStatus(protocol.Away);
			break;
		case osNA:
			setOnlineStatus(protocol.NotAvailable);
			break;
		case osDND:
			setOnlineStatus(protocol.DoNotDisturb);
			break;
		case osSkypeOut:
			setOnlineStatus(protocol.Phone);
			break;
		case osSkypeMe:
			setOnlineStatus(protocol.SkypeMe);
			break;
	}
}

bool SkypeContact::isReachable() {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

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
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	d->session = 0L;//it exists no more or it is no longer of this contact
}

bool SkypeContact::hasChat() const {
	return d->session;//does it have a chat session?
}

void SkypeContact::receiveIm(const QString &message) {
	kdDebug(65320) << k_funcinfo << endl;//some debug info

	if (!hasChat()) {
		manager(CanCreate);//create it
		if (!hasChat())//something failed
			return;
	}

	Kopete::Message mes(this, d->account->myself(), message, Kopete::Message::Inbound);//create the message
	d->session->appendMessage(mes);//add it to the session
}

#include "skypecontact.moc"
