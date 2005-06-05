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
#include "skype.h"
#include <skypeconnection.h>
#include <skypeaccount.h>

#include <kdebug.h>
#include <qvaluelist.h>
#include <qstring.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qtimer.h>

#define PROTOCOL_MAX 4
#define PROTOCOL_MIN 3
#define TEST_QUIT if (!d->connection.connected()) return;

///This one indicates, weather the Skype is connected (does not mean weather it is marked as online, just if it has connection to the site)
typedef enum {
	csOffline,
	csConnecting,
	csPausing,
	csOnline,
	csLoggedOut
} connectionStatus;

///This describes what the user is marked as. If online here and not connected to skype site, he is probably offline
typedef enum {
	usUnknown,
	usOffline,
	usOnline,
	usSkypeMe,
	usAway,
	usNA,
	usDND,
	usInvisible
} userStatus;

class SkypePrivate {
	public:
		///The connection
		SkypeConnection connection;
		///The queue
		QValueList<QString> messageQueue;
		///How do we start skype?
		int launchType;
		///What is our name?
		QString appName;
		///Should the skypeconnection start skype automatically if it is not running?
		bool start;
		///Is the skype connected?
		connectionStatus connStatus;
		///What is the online status for the user?
		userStatus onlineStatus;
		///This contains last search request to know, what we were searching for
		QString searchFor;
		///Is the hitch-mode enabled?
		bool hitch;
		///Is the mark read messages mode enabled?
		bool mark;
		///The skype account this connection belongs to
		SkypeAccount &account;
		///Should we show the message that Skype died? It if off when going offline, this removes that onnoying message when logging off and skype finishes first.
		bool showDeadMessage;
		///Do we automatically scan for unread messages on login?
		bool scanForUnread;
		///Constructor
		SkypePrivate(SkypeAccount &_account) : account(_account) {};//initialize all that needs it
		///List of known calls, so they are not showed twice
		QValueList<QString> knownCalls;
		///Are the pings enabled?
		bool pings;
		///Pinging timer
		QTimer *pingTimer;
		///What bus is used now?
		int bus;
		///Do we start DBus as well if needed?
		bool startDBus;
		///The launch timeout (after that no connection -> unsuccessfull -> error)
		int launchTimeout;
		///By what command is skype started?
		QString skypeCommand;
		///Do we wait before connecting?
		int waitBeforeConnect;
};

Skype::Skype(SkypeAccount &account) : QObject() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d = new SkypePrivate(account);//create the d-pointer

	//initial values
	d->connStatus = csOffline;
	d->onlineStatus = usOffline;
	d->searchFor = "";
	d->pings = false;
	d->pingTimer = new QTimer;

	connect(&d->connection, SIGNAL(connectionClosed(int)), this, SLOT(closed(int)));//tell me if you close/lose the connection
	connect(&d->connection, SIGNAL(connectionDone(int, int)), this, SLOT(connectionDone(int, int)));//Do something whe he finishes connecting
	connect(&d->connection, SIGNAL(error(const QString&)), this, SLOT(error(const QString&)));//Listen for errors
	connect(&d->connection, SIGNAL(received(const QString&)), this, SLOT(skypeMessage(const QString&)));//Take all incoming messages
	connect(d->pingTimer, SIGNAL(timeout()), this, SLOT(ping()));
}


Skype::~Skype() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (d->connection.connected())
		d->connection << QString("SET USERSTATUS OFFLINE");

	d->pingTimer->stop();
	d->pingTimer->deleteLater();

	delete d;//release the memory
}

void Skype::setOnline() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	d->showDeadMessage = true;

	if ((d->onlineStatus == usOnline) && (d->connStatus == csOnline) && (d->connection.connected()))
		return;//Already online

	queueSkypeMessage("SET USERSTATUS ONLINE", true);//just send the message
}

void Skype::setOffline() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	d->showDeadMessage = false;

	d->connection << QString("SET USERSTATUS OFFLINE");//this one special, do not connect to skype because of that
}

void Skype::setAway() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	d->showDeadMessage = true;

	queueSkypeMessage("SET USERSTATUS AWAY", true);
}

void Skype::setNotAvailable() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	d->showDeadMessage = true;

	queueSkypeMessage("SET USERSTATUS NA", true);
}

void Skype::setDND() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	d->showDeadMessage = true;

	queueSkypeMessage("SET USERSTATUS DND", true);
}

void Skype::setInvisible() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	d->showDeadMessage = true;

	queueSkypeMessage("SET USERSTATUS INVISIBLE", true);
}

void Skype::setSkypeMe() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	d->showDeadMessage = true;

	queueSkypeMessage("SET USERSTATUS SKYPEME", true);
}

void Skype::queueSkypeMessage(const QString &message, bool deleteQueue) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (d->connection.connected()) {//we are connected, so just send it
		d->connection << message;//just send it
	} else {
		emit statusConnecting();//Started connecting to skype
		if (deleteQueue)
			d->messageQueue.clear();//delete all old messages
		d->messageQueue << message;//add the new one
		d->connection.connectSkype((d->start) ? d->skypeCommand : "", d->appName, PROTOCOL_MAX, d->bus, d->startDBus, d->launchTimeout, d->waitBeforeConnect);//try to connect
	}
}

void Skype::setValues(int launchType, const QString &appName) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->appName = appName;
	if (d->appName.isEmpty()) //The defaut one?
		d->appName = "Kopete";
	d->launchType = launchType;
	switch (launchType) {
		case 0: //start the skype if it is needed
			d->start = true;//just set autostart
			break;
		case 1: //do not start
			d->start = false;//do not start
			break;
	}
}

void Skype::closed(int) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	emit wentOffline();//No longer connected
	d->messageQueue.clear();//no messages will wait, it was lost
	d->pingTimer->stop();
}

void Skype::connectionDone(int error, int protocolVer) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (d->pings) {
		d->pingTimer->start(1000);
	}

	if (error == seSuccess) {//It worked
		if (protocolVer < PROTOCOL_MIN) {//The protocol is too old, it is not useable
			this->error(i18n("This version of Skype is too old, consider upgrading"));
			connectionDone(seUnknown, 0);//So act like there was an error
			return;//and it is all fo now
		}

		while (d->messageQueue.size()) {//It isn't empty yet?
			QValueList<QString>::iterator it = d->messageQueue.begin();//take the first one
			d->connection << (*it);//send the message
			d->messageQueue.remove(it);//remove this one
		}
		emit updateAllContacts();//let all contacts update their information
		search("FRIENDS");//search for friends - to add them all
		TEST_QUIT;//if it failed, do not continue
		d->connection.send("GET USERSTATUS");
		TEST_QUIT;
		d->connection.send("GET CONNSTATUS");//
	} else {
		closed(crLost);//OK, this is wrong, justclose the connection/atempt and delete the queue
	}
}

void Skype::error(const QString &message) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	disconnect(&d->connection, SIGNAL(error(const QString&)), this, SLOT(error(const QString&)));//One arror at a time is enough, stop flooding the user

	if (d->showDeadMessage)//just skip the eror message if we are going offline, noone ever cares.
		KMessageBox::error(0L, message, i18n("Skype protocol"));//Show the message

	connect(&d->connection, SIGNAL(error(const QString&)), this, SLOT(error(const QString&)));//Continue showing more errors in future
}

void Skype::skypeMessage(const QString &message) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	QString messageType = message.section(' ', 0, 0).stripWhiteSpace().upper();//get the first part of the message
	if (messageType == "CONNSTATUS") {//the connection status
		QString value = message.section(' ', 1, 1).stripWhiteSpace().upper();//get the second part of the message
		if (value == "OFFLINE")
			d->connStatus = csOffline;
		else if (value == "CONNECTING")
			d->connStatus = csConnecting;
		else if (value == "PAUSING")
			d->connStatus = csPausing;
		else if (value == "ONLINE")
			d->connStatus = csOnline;
		else if (value == "LOGGEDOUT")
			d->connStatus = csLoggedOut;

		resetStatus();//set new status
	} else if (messageType == "USERSTATUS") {//Status of this user
		QString value = message.section(' ', 1, 1).stripWhiteSpace().upper();//get the second part
		if (value == "UNKNOWN")
			d->onlineStatus = usUnknown;
		else if (value == "OFFLINE")
			d->onlineStatus = usOffline;
		else if (value == "ONLINE")
			d->onlineStatus = usOnline;
		else if (value == "SKYPEME")
			d->onlineStatus = usSkypeMe;
		else if (value == "AWAY")
			d->onlineStatus = usAway;
		else if (value == "NA")
			d->onlineStatus = usNA;
		else if (value == "DND")
			d->onlineStatus = usDND;
		else if (value == "INVISIBLE")
			d->onlineStatus = usInvisible;

		resetStatus();
	} else if (messageType == "USERS") {//some user info
		QString theRest = message.section(' ', 1).stripWhiteSpace();//take the rest
		if (d->searchFor == "FRIENDS") {//it was initial search for al users
			QStringList names = QStringList::split(",", theRest);//divide it into names by comas
			kdDebug(14311) << "Names: " << names << endl;//write what you have done with that
			for (QStringList::iterator it = names.begin(); it != names.end(); ++it) {//run trough the names
				QString name = (*it).stripWhiteSpace();//get the name only
				if (name.isEmpty())
					continue;//just skip the empty names
				emit newUser(name);//add the user to list
			}
			if (d->scanForUnread)
				search("MISSEDMESSAGES");
		}
	} else if (messageType == "USER") {//This is for some contact
		const QString &contactId = message.section(' ', 1, 1);//take the second part, it is the user name
		const QString &type = message.section(' ', 2, 2).stripWhiteSpace().upper();//get what it is
		if ((type == "FULLNAME") || (type == "DISPLAYNAME") || (type == "SEX") ||
			(type == "PHONE_HOME") || (type == "PHONE_OFFICE") ||
			(type == "PHONE_MOBILE") ||
			(type == "ONLINESTATUS") || (type == "BUDDYSTATUS")) {
			const QString &info = message.section(' ', 2);//and the rest is just the message for that contact
			emit contactInfo(contactId, info);//and let the contact know
		} else kdDebug(14311) << "Unknown message for contact, ignored" << endl;
	} else if (messageType == "CHATMESSAGE") {//something with message, maebe incoming/sent
		QString messageId = message.section(' ', 1, 1).stripWhiteSpace();//get the second part of message - it is the message ID
		QString type = message.section(' ', 2, 2).stripWhiteSpace().upper();//This part significates what about the message are we talking about (status, body, etc..)
		if (type == "STATUS") {//OK, status of some message has changed, check what is it
			QString value = message.section(' ', 3, 3).stripWhiteSpace().upper();//get the last part, what status it is
			if (value == "RECEIVED") {//OK, received new message, possibly read it
				QString type = (d->connection % QString("GET CHATMESSAGE %1 TYPE").arg(messageId)).section(' ', 3, 3).stripWhiteSpace().upper();//Ask skype for the type and filter out only the type, delete all the bloat
				if (type == "SAID") {//OK, it is some IM
					hitchHike(messageId);//receive the message
				}
				///@todo other types of messages (like topic etc)
			} else if (value == "SENT") {//the message has been successfully sent
				emit sentMessage(messageId);//just inform others it is sent OK
			} else if (value == "SENDING") {//Sendign out some message, that means it is a new one
				if ((d->connection % QString("GET CHATMESSAGE %1 TYPE").arg(messageId)).section(' ', 3, 3).stripWhiteSpace().upper() == "SAID")//it is some message I'm interested in
					emit gotMessageId(messageId);//Someone may be interested in its ID
			}
		}
	} else if (messageType == "CHATMESSAGES") {
		if (d->searchFor == "MISSEDMESSAGES") {//Theese are messages we did not read yet
			QStringList messages = QStringList::split(' ', message.section(' ', 1));//get the meassage IDs
			for (QStringList::iterator it = messages.begin(); it != messages.end(); ++it) {
				QString Id = (*it).stripWhiteSpace();
				if (Id.isEmpty())
					continue;
				skypeMessage(QString("CHATMESSAGE %1 STATUS RECEIVED").arg(Id));//simulate incoming message notification
			}
		}
	} else if (messageType == "CALL") {
		const QString &callId = message.section(' ', 1, 1).stripWhiteSpace();
		if (message.section(' ', 2, 2).stripWhiteSpace().upper() == "STATUS") {
			if (d->knownCalls.findIndex(callId) == -1) {//new call
				d->knownCalls << callId;
				const QString &userId = (d->connection % QString("GET CALL %1 PARTNER_HANDLE").arg(callId)).section(' ', 3, 3).stripWhiteSpace();
				emit newCall(callId, userId);
			}
			const QString &status = message.section(' ', 3, 3).stripWhiteSpace().upper();
			if (status == "FAILED") {
				int reason = (d->connection % QString("GET CALL %1 FAILUREREASON").arg(callId)).section(' ', 3, 3).stripWhiteSpace().toInt();
				QString errorText = i18n("Unknown error");
				switch (reason) {
					case 1:
						errorText = i18n("Misc error");
						break;
					case 2:
						errorText = i18n("User or phone number does not exist");
						break;
					case 3:
						errorText = i18n("User is offline");
						break;
					case 4:
						errorText = i18n("No proxy found");
						break;
					case 5:
						errorText = i18n("Session terminated");
						break;
					case 6:
						errorText = i18n("No common codec found");
						break;
					case 7:
						errorText = i18n("Sound I/O error");
						break;
					case 8:
						errorText = i18n("Problem with remote sound device");
						break;
					case 9:
						errorText = i18n("Call blocked by recipient");
						break;
					case 10:
						errorText = i18n("Recipient not a friend");
						break;
					case 11:
						errorText = i18n("User not authorized by recipient");
						break;
					case 12:
						errorText = i18n("Sound recording error");
						break;
				}
				emit callError(callId, errorText);
			}
			emit callStatus(callId, status);
		}
	} else if (messageType == "CURRENTUSERHANDLE") {
		QString user = message.section(' ', 1, 1).stripWhiteSpace();
		QString name = (d->connection % QString("GET USER %1 DISPLAYNAME").arg(user)).section(' ', 3).stripWhiteSpace();
		if (name.isEmpty())
			name = (d->connection % QString("GET USER %1 FULLNAME").arg(user)).section(' ', 3).stripWhiteSpace();
		if (name.isEmpty())
			name = user;
		emit setMyselfName(name);
	}
}

void Skype::getContactBuddy(const QString &contact) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->connection << QString("GET USER %1 BUDDYSTATUS").arg(contact);//just make a message asking for the buddystatus of user and send it
}

void Skype::resetStatus() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	switch (d->connStatus) {
		case csOffline:
		case csLoggedOut:
			emit wentOffline();//Do not care what is the user marked as, this is more importatnt
			return;
		case csConnecting:
			if (d->onlineStatus == usOffline)//not connecting, user wants to be offline
				break;
			emit statusConnecting();//still connecting, wait a minute
			return;
		default://just remove the compile-time warning about not handled value
			break;
	}

	switch (d->onlineStatus) {
		case usUnknown:
			emit statusConnecting();
			break;
		case usOffline:
			emit wentOffline();
			break;
		case usOnline:
			emit wentOnline();
			break;
		case usSkypeMe:
			emit wentSkypeMe();
			break;
		case usAway:
			emit wentAway();
			break;
		case usNA:
			emit wentNotAvailable();
			break;
		case usDND:
			emit wentDND();
			break;
		case usInvisible:
			emit wentInvisible();
			break;
	}
}

void Skype::search(const QString &what) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->searchFor = what.section(' ', 0, 0).stripWhiteSpace().upper();
	d->connection << QString("SEARCH %1").arg(what.upper());//search for that
}

void Skype::getContactInfo(const QString &contact) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->connection << QString("GET USER %1 FULLNAME").arg(contact)//ask for full name
	<< QString("GET USER %1 SEX").arg(contact)//ask for sex
	<< QString("GET USER %1 DISPLAYNAME").arg(contact)//ask for the nick-name
	<< QString("GET USER %1 PHONE_HOME").arg(contact)//ask for the nick-name
	<< QString("GET USER %1 PHONE_OFFICE").arg(contact)//ask for the nick-name
	<< QString("GET USER %1 PHONE_MOBILE").arg(contact)//ask for the nick-name
	<< QString("GET USER %1 ONLINESTATUS").arg(contact)//ask for the online status
	<< QString("GET USER %1 BUDDYSTATUS").arg(contact);//and the rest of info
}

bool Skype::canComunicate() {
	return d->connection.connected();
}

void Skype::setHitchMode(bool value) {
	d->hitch = value;
}

void Skype::setMarkMode(bool value) {
	d->mark = value;
}

void Skype::hitchHike(const QString &messageId) {
	kdDebug(14311) << k_funcinfo << "Message: " << messageId << endl;//some debug info

	const QString &user = (d->connection % QString("GET CHATMESSAGE %1 FROM_HANDLE").arg(messageId)).section(' ', 3, 3).stripWhiteSpace();//ask skyp for a sender of that message and filter out the blouat around (like CHATMESSAGE 123...)

	if ((d->hitch) || (d->account.userHasChat(user))) {//it can be read eather if the hitchhiking non-chat messages is enabled or if the user already has opened a chat
		emit receivedIM(user, (d->connection % QString("GET CHATMESSAGE %1 BODY").arg(messageId)).section(' ', 3));//ask skype for the body and filter out the bload, we want only the text and make everyone aware that we received a message
		if (d->mark) //We should mark it as read
			d->connection << QString("SET CHATMESSAGE %1 SEEN").arg(messageId);//OK, just tell skype it is read
	}
}

void Skype::send(const QString &user, const QString &message) {
	kdDebug(14311) << k_funcinfo <<  endl;//some debug info

	d->connection << QString("MESSAGE %1 %2").arg(user).arg(message);//just ask skype to send it
}

void Skype::setScanForUnread(bool value) {
	d->scanForUnread = value;
}

void Skype::makeCall(const QString &userId) {
	kdDebug(14311) << k_funcinfo <<  endl;//some debug info

	d->connection << QString("CALL %1").arg(userId);
}

void Skype::acceptCall(const QString &callId) {
	kdDebug(14311) << k_funcinfo <<  endl;//some debug info

	d->connection << QString("SET CALL %1 STATUS INPROGRESS").arg(callId);
}

void Skype::hangUp(const QString &callId) {
	kdDebug(14311) << k_funcinfo <<  endl;//some debug info

	d->connection << QString("SET CALL %1 STATUS FINISHED").arg(callId);
}

void Skype::toggleHoldCall(const QString &callId) {
	kdDebug(14311) << k_funcinfo <<  endl;//some debug info

	const QString &status = (d->connection % QString("GET CALL %1 STATUS").arg(callId)).section(' ', 3, 3).stripWhiteSpace().upper();
	if ((status == "ONHOLD") || (status == "LOCALHOLD"))
		d->connection << QString("SET CALL %1 STATUS INPROGRESS").arg(callId);
	else
		d->connection << QString("SET CALL %1 STATUS ONHOLD").arg(callId);
}

bool Skype::isCallIncoming(const QString &callId) {
	const QString &type = (d->connection % QString("GET CALL %1 TYPE").arg(callId)).section(' ', 3, 3).stripWhiteSpace().upper();
	return ((type == "INCOMING_P2P") || (type == "INCOMING_PSTN"));
}

void Skype::getSkypeOut() {
	const QString &curr = (d->connection % QString("GET PROFILE PSTN_BALANCE_CURRENCY")).section(' ', 2, 2).stripWhiteSpace().upper();
	if (curr.isEmpty()) {
		emit skypeOutInfo(0, "");
	} else {
		int value = (d->connection % QString("GET PROFILE PSTN_BALANCE")).section(' ', 2, 2).stripWhiteSpace().toInt();
		emit skypeOutInfo(value, curr);
	}
}

void Skype::enablePings(bool enabled) {
	kdDebug(14311) << k_funcinfo <<  endl;//some debug info

	d->pings = enabled;

	if (!enabled) {
		d->pingTimer->stop();
		return;
	}

	if (d->connStatus != csOffline) {
		d->pingTimer->start(1000);
	}
}

void Skype::ping() {
	d->connection << QString("PING");
}

void Skype::setBus(int bus) {
	d->bus = bus;
}

void Skype::setStartDBus(bool enabled) {
	d->startDBus = enabled;
}

void Skype::setLaunchTimeout(int seconds) {
	d->launchTimeout = seconds;
}

void Skype::setSkypeCommand(const QString &command) {
	d->skypeCommand = command;
}

void Skype::setWaitConnect(int value) {
	d->waitBeforeConnect = value;
}

#include "skype.moc"
