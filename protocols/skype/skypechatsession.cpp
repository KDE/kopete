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

#include "skypechatsession.h"
#include "skypeaccount.h"
#include "skypeprotocol.h"
#include "skypecontact.h"

#include <kdebug.h>
#include <kopetechatsessionmanager.h>
#include <qdict.h>
#include <qstring.h>

class SkypeChatSessionPrivate {
	public:
		///Referenco to the protocol
		SkypeProtocol *protocol;
		///Reference to the account
		SkypeAccount *account;
		///List of unsent messages
		QDict<Kopete::Message> pending;
		///Last sent message, it is used for getting its ID so it can be put in the pending messages
		Kopete::Message *lastMessage;
		///Am I connected to the messageSent signal?
		bool connectedSent;
		/**
		 * Constructor
		 * @param _protocol Reference to the Skype protocol
		 * @param _account Reference to the account this chat belongs to
		 */
		SkypeChatSessionPrivate(SkypeProtocol *_protocol, SkypeAccount *_account) {
			kdDebug(14311) << k_funcinfo << endl;//some debug info
			//save given values
			account = _account;
			protocol = _protocol;

			lastMessage = 0L;
			connectedSent = false;
		};
};

static Kopete::ContactPtrList constructList(SkypeContact *contact) {
	Kopete::ContactPtrList list;//create the contact
	list.append(contact);//add there the contact

	return list;//and return the list
}

SkypeChatSession::SkypeChatSession(SkypeAccount *account, SkypeContact *contact) :
		Kopete::ChatSession(account->myself(), constructList(contact), &account->protocol(), (char *)0L) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	//create the D-pointer
	d = new SkypeChatSessionPrivate(&account->protocol(), account);
	Kopete::ChatSessionManager::self()->registerChatSession( this );
	connect(this, SIGNAL(messageSent(Kopete::Message&, Kopete::ChatSession*)), this, SLOT(message(Kopete::Message& )));//this will send the messages from this user going out
}


SkypeChatSession::~SkypeChatSession() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	delete d;//remove the D pointer
}

void SkypeChatSession::message(Kopete::Message &message) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->lastMessage = new Kopete::Message(message);//copy the message, I need to store it for a while
	connect(d->account, SIGNAL(gotMessageId(const QString& )), this, SLOT(knowId(const QString& )));//get the Id when it is known
	d->account->sendMessage(*d->lastMessage);//send it
}

void SkypeChatSession::knowId(const QString &id) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	disconnect(d->account, SIGNAL(gotMessageId(const QString& )), this, SLOT(knowId(const QString& )));//OK, I know it, it is enough, nothing more is needed. THIS IS NEEDED HERE, IT WOULD NOT WORK WITHOUT THIS
	d->pending.insert(id, d->lastMessage);//put the message into list of unsent
	if (!d->connectedSent) {//I do not listen for sent message
		connect(d->account, SIGNAL(sentMessage(const QString& )), this, SLOT(messageSent(const QString& )));//SO start to do so
		d->connectedSent = true;//Already connected, do not connect again
	}

	d->lastMessage = 0L;//so, it is not needed now
}

void SkypeChatSession::messageSent(const QString &id) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	Kopete::Message *mes = d->pending.take(id);//get the message out of the dictionary
	if (mes) {//We had the message, it was ours
		appendMessage(*mes);//show it there
		delete mes;//it is no longer needed, it is a copy I made for myself

		if ((d->pending.isEmpty()) && (!d->lastMessage)) {//everything sent
			messageSucceeded();//OK, stop spining the icon in the session window
			disconnect(d->account, SIGNAL(sentMessage(const QString& )), this, SLOT(messageSent(const QString& )));//No longer listen for sent messages, they are all sent
			d->connectedSent = false;//no longer connected
		}
	}
}

#include "skypechatsession.moc"
