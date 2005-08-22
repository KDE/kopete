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

#include "skypeaccount.h"
#include "skypeprotocol.h"
#include "skypecontact.h"
#include "skype.h"
#include "skypecalldialog.h"
#include "skypechatsession.h"
#include "skypeconference.h"

#include <qstring.h>
#include <kopetemetacontact.h>
#include <kopeteonlinestatus.h>
#include <kopetecontactlist.h>
#include <kopetecontact.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kconfigbase.h>
#include <qdict.h>
#include <kopetemessage.h>
#include <kprocess.h>

class SkypeAccountPrivate {
	public:
		///The skype protocol pointer
		SkypeProtocol *protocol;
		///ID of this account (means my skype name)
		QString ID;
		///The skype back-end
		Skype skype;
		///The hitchhike mode of incoming messages
		bool hitch;
		///The mark read messages mode
		bool markRead;
		///Search for unread messages on login?
		bool searchForUnread;
		///Do we show call control window?
		bool callControl;
		///Metacontact for all users that aren't in the list
		Kopete::MetaContact notInListUsers;
		///Constructor
		SkypeAccountPrivate(SkypeAccount &account) : skype(account) {};//just an empty constructor
		///Automatic close of call window when the call finishes (in seconds, 0 -> disabled)
		int callWindowTimeout;
		///Are the pings enabled?
		bool pings;
		///What bus are we using, session (0) or system (1)?
		int bus;
		///Do we start DBus if needed?
		bool startDBus;
		///How long can I keep trying connect to newly started skype, before I give up (seconds)
		int launchTimeout;
		///By what command is the skype started?
		QString skypeCommand;
		///What is my name, by the way?
		QString myName;
		///Do we wait before connecting?
		int waitBeforeConnect;
		///List of chat all chat sessions
		QDict<SkypeChatSession> sessions;
		///Last used chat session
		SkypeChatSession *lastSession;
		///List of the conference calls
		QDict<SkypeConference> conferences;
		///List of existing calls
		QDict<SkypeCallDialog> calls;
		///Shall chat window leave the chat whenit is closed
		bool leaveOnExit;
		///Executed before making the call
		QString startCallCommand;
		///Executed after finished the call
		QString endCallCommand;
		///Wait for the start call command to finitsh?
		bool waitForStartCallCommand;
		///Execute the end call command only if no other calls exists?
		bool endCallCommandOnlyLats;
		///How many calls are opened now?
		int callCount;
		///Command executed on incoming call
		QString incommingCommand;
};

SkypeAccount::SkypeAccount(SkypeProtocol *protocol) : Kopete::Account(protocol, "Skype", (char *)0) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	//keep track of what accounts the protocol has
	protocol->registerAccount(this);

	//the d pointer
	d = new SkypeAccountPrivate(*this);
	d->calls.setAutoDelete(false);
	d->conferences.setAutoDelete(false);
	//remember the protocol, it will be needed
	d->protocol = protocol;

	//load the properties
	KConfigGroup *config = configGroup();
	author = config->readEntry("Authorization");//get the name how to authorize myself
	launchType = config->readNumEntry("Launch");//launch the skype?
	setScanForUnread(config->readBoolEntry("ScanForUnread"));
	setCallControl(config->readBoolEntry("CallControl"));
	setPings(config->readBoolEntry("Pings", true));
	setBus(config->readNumEntry("Bus", 1));
	//setStartDBus(config->readBoolEntry("StartDBus", false));
	///@todo Once Dbus launching works, remove this v and uncomment this ^
	setStartDBus(false);
	setLaunchTimeout(config->readNumEntry("LaunchTimeout", 30));
	d->myName = config->readEntry("MyselfName", "Skype");
	setSkypeCommand(config->readEntry("SkypeCommand", "artsdsp skype --use-session-dbus"));
	setWaitBeforeConnect(config->readNumEntry("WaitBeforeConnect", 10));
	setLeaveOnExit(config->readBoolEntry("LeaveOnExit", true));
	setStartCallCommand(config->readEntry("StartCallCommand", ""));
	setEndCallCommand(config->readEntry("EndCallCommand", ""));
	setWaitForStartCallCommand(config->readBoolEntry("WaitForStartCallCommand", false));
	setEndCallCommandOnlyForLast(config->readBoolEntry("EndCallCommandOnlyLast", false));
	setIncomingCommand(config->readEntry("IncomingCall", ""));

	//create myself contact
	SkypeContact *_myself = new SkypeContact(this, "Skype", Kopete::ContactList::self()->myself(), false);
	setMyself(_myself);
	//and set default online status (means offline)
	myself()->setOnlineStatus(protocol->Offline);

	//Now, connect the signals
	QObject::connect(&d->skype, SIGNAL(wentOnline()), this, SLOT(wentOnline()));
	QObject::connect(&d->skype, SIGNAL(wentOffline()), this, SLOT(wentOffline()));
	QObject::connect(&d->skype, SIGNAL(wentAway()), this, SLOT(wentAway()));
	QObject::connect(&d->skype, SIGNAL(wentNotAvailable()), this, SLOT(wentNotAvailable()));
	QObject::connect(&d->skype, SIGNAL(wentDND()), this, SLOT(wentDND()));
	QObject::connect(&d->skype, SIGNAL(wentInvisible()), this, SLOT(wentInvisible()));
	QObject::connect(&d->skype, SIGNAL(wentSkypeMe()), this, SLOT(wentSkypeMe()));
	QObject::connect(&d->skype, SIGNAL(statusConnecting()), this, SLOT(statusConnecting()));
	QObject::connect(&d->skype, SIGNAL(newUser(const QString&)), this, SLOT(newUser(const QString&)));
	QObject::connect(&d->skype, SIGNAL(contactInfo(const QString&, const QString& )), this, SLOT(updateContactInfo(const QString&, const QString& )));
	QObject::connect(&d->skype, SIGNAL(receivedIM(const QString&, const QString&, const QString& )), this, SLOT(receivedIm(const QString&, const QString&, const QString& )));
	QObject::connect(&d->skype, SIGNAL(gotMessageId(const QString& )), this, SLOT(gotMessageId(const QString& )));//every time some ID is known inform the contacts
	QObject::connect(&d->skype, SIGNAL(newCall(const QString&, const QString&)), this, SLOT(newCall(const QString&, const QString&)));
	QObject::connect(&d->skype, SIGNAL(setMyselfName(const QString&)), this, SLOT(setMyselfName(const QString& )));
	QObject::connect(&d->skype, SIGNAL(receivedMultiIM(const QString&, const QString&, const QString&, const QString& )), this, SLOT(receiveMultiIm(const QString&, const QString&, const QString&, const QString& )));
	QObject::connect(&d->skype, SIGNAL(outgoingMessage(const QString&, const QString&)), this, SLOT(sentMessage(const QString&, const QString& )));
	QObject::connect(&d->skype, SIGNAL(groupCall(const QString&, const QString& )), this, SLOT(groupCall(const QString&, const QString& )));

	//set values for the connection (should be updated if changed)
	d->skype.setValues(launchType, author);
	setHitchHike(config->readBoolEntry("Hitch", true));
	setMarkRead(config->readBoolEntry("MarkRead", true));//read the modes of account
	d->callWindowTimeout = config->readNumEntry("CloseWindowTimeout", 3);
	setPings(config->readBoolEntry("Pings", true));
	d->sessions.setAutoDelete(false);
	d->lastSession = 0L;
	d->callCount = 0;
}


SkypeAccount::~SkypeAccount() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	save();

	d->protocol->unregisterAccount();//This account no longer exists

	//free memory
	delete d;
}

bool SkypeAccount::createContact(const QString &contactID, Kopete::MetaContact *parentContact) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (!contact(contactID)) {//check weather it is not used already
		SkypeContact *newContact = new SkypeContact(this, contactID, parentContact);//create the contact

		return newContact != 0L;//test weather it was created
	} else {
		kdDebug(14311) << k_funcinfo << "Contact already exists:" << contactID << endl;//Tell that it is not OK

		return false;
	}
}

void SkypeAccount::setAway(bool away, const QString &reason) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (away)
		setOnlineStatus(d->protocol->Away, reason);
	else
		setOnlineStatus(d->protocol->Online, reason);
}

void SkypeAccount::setOnlineStatus(const Kopete::OnlineStatus &status, const QString &) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (status == d->protocol->Online)
		d->skype.setOnline();//Go online
	else if (status == d->protocol->Offline)
		d->skype.setOffline();//Go offline
	else if (status == d->protocol->Away)
		d->skype.setAway();
	else if (status == d->protocol->NotAvailable)
		d->skype.setNotAvailable();
	else if (status == d->protocol->DoNotDisturb)
		d->skype.setDND();
	else if (status == d->protocol->Invisible)
		d->skype.setInvisible();
	else if (status == d->protocol->SkypeMe)
		d->skype.setSkypeMe();
	else
		kdDebug(14311) << "Unknown online status" << endl;//Just a warning that I do not know that status
}

void SkypeAccount::disconnect() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	setOnlineStatus(d->protocol->Offline, "");
}

SkypeContact *SkypeAccount::contact(const QString &id) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	return static_cast<SkypeContact *>(contacts()[id]);//get the contact and convert it into the skype contact, there are no other contacts anyway
}

void SkypeAccount::connect(const Kopete::OnlineStatus &Status) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if ((Status != d->protocol->Online) && (Status != d->protocol->Away) &&
		(Status != d->protocol->NotAvailable) && (Status != d->protocol->DoNotDisturb) &&
		(Status != d->protocol->SkypeMe))//some strange online status, taje a default one
			setOnlineStatus(d->protocol->Online, "");
	else
		setOnlineStatus(Status, "");//just change the status
}

void SkypeAccount::save() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	KConfigGroup *config = configGroup();//get the config
	config->writeEntry("Authorization", author);//write the authorization name
	config->writeEntry("Launch", launchType);//and the launch type
	config->writeEntry("Hitch", getHitchHike());//save the hitch hike messages mode
	config->writeEntry("MarkRead", getMarkRead());//save the Mark read messages mode
	config->writeEntry("ScanForUnread", getScanForUnread());
	config->writeEntry("CallControl", getCallControl());
	config->writeEntry("CloseWindowTimeout", d->callWindowTimeout);
	config->writeEntry("Pings", getPings());
	config->writeEntry("Bus", getBus());
	config->writeEntry("StartDBus", getStartDBus());
	config->writeEntry("LaunchTimeout", getLaunchTimeout());
	config->writeEntry("SkypeCommand", getSkypeCommand());
	config->writeEntry("MyselfName", d->myName);
	config->writeEntry("WaitBeforeConnect", getWaitBeforeConnect());
	config->writeEntry("LeaveOnExit", leaveOnExit());
	config->writeEntry("StartCallCommand", startCallCommand());
	config->writeEntry("EndCallCommand", endCallCommand());
	config->writeEntry("WaitForStartCallCommand", waitForStartCallCommand());
	config->writeEntry("EndCallCommandOnlyLast", endCallCommandOnlyLast());
	config->writeEntry("IncomingCall", incomingCommand());

	//save it into the skype connection as well
	d->skype.setValues(launchType, author);
}

void SkypeAccount::wentOnline() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	myself()->setOnlineStatus(d->protocol->Online);//just set the icon
	d->skype.enablePings(d->pings);
	emit connectionStatus(true);
}

void SkypeAccount::wentOffline() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	myself()->setOnlineStatus(d->protocol->Offline);//just change the icon
	emit connectionStatus(false);
}

void SkypeAccount::wentAway() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	myself()->setOnlineStatus(d->protocol->Away);//just change the icon
	emit connectionStatus(true);
}

void SkypeAccount::wentNotAvailable() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	myself()->setOnlineStatus(d->protocol->NotAvailable);
	emit connectionStatus(true);
}

void SkypeAccount::wentDND() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	myself()->setOnlineStatus(d->protocol->DoNotDisturb);
	emit connectionStatus(true);
}

void SkypeAccount::wentInvisible() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	myself()->setOnlineStatus(d->protocol->Invisible);
	emit connectionStatus(true);
}

void SkypeAccount::wentSkypeMe() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	myself()->setOnlineStatus(d->protocol->SkypeMe);
	emit connectionStatus(true);
}

void SkypeAccount::statusConnecting() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	myself()->setOnlineStatus(d->protocol->Connecting);
	emit connectionStatus(false);
}

void SkypeAccount::newUser(const QString &name) {
	kdDebug(14311) << k_funcinfo << QString("name = %1").arg(name) << endl;//some debug info
	if (contacts().find(name))
		return;
	addContact(name);
}

void SkypeAccount::prepareContact(SkypeContact *contact) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	QObject::connect(&d->skype, SIGNAL(updateAllContacts()), contact, SLOT(requestInfo()));//all contacts will know that
	QObject::connect(contact, SIGNAL(infoRequest(const QString& )), &d->skype, SLOT(getContactInfo(const QString& )));//How do we ask for info?
	QObject::connect(this, SIGNAL(connectionStatus(bool )), contact, SLOT(connectionStatus(bool )));
	QObject::connect(contact, SIGNAL(setCallPossible(bool )), d->protocol, SLOT(updateCallActionStatus()));
}

void SkypeAccount::updateContactInfo(const QString &contact, const QString &change) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	SkypeContact *cont = static_cast<SkypeContact *> (contacts().find(contact));//get the contact
	if (cont)
		cont->setInfo(change);//give it the message
	else {//it does not yet exist, create it if it is in skype contact list (can be got by buddystatus)
		const QString &type = change.section(' ', 0, 0).stripWhiteSpace().upper();//get the first part of the message, it should be BUDDYSTATUS
		const QString &value = change.section(' ', 1, 1).stripWhiteSpace();//get the second part if it is some reasonable value
		if ((type == "BUDDYSTATUS") && ((value == "2") || (value == "3"))) {//the user is in skype contact list
			newUser(contact);
		} else if (type != "BUDDYSTATUS")//this is some other info
			d->skype.getContactBuddy(contact);//get the buddy status for the account and check, if it is in contact list or not
	}
}

bool SkypeAccount::canComunicate() {
	return d->skype.canComunicate();
}

SkypeProtocol * SkypeAccount::protocol() {
	return d->protocol;
}

void SkypeAccount::sendMessage(Kopete::Message &message, const QString &chat) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (chat.isEmpty()) {
		const QString &user = message.to().at(0)->contactId();//get id of the first contact, messages to multiple people are not yet possible
		const QString &body = message.plainBody();//get the text of the message

		d->skype.send(user, body);//send it by skype
	} else {
		const QString &body = message.plainBody();

		d->skype.sendToChat(chat, body);
	}
}

bool SkypeAccount::getHitchHike() const {
	return d->hitch;
}

bool SkypeAccount::getMarkRead() const {
	return d->markRead;
}

void SkypeAccount::setHitchHike(bool value) {
	d->hitch = value;//save it
	d->skype.setHitchMode(value);//set it in the skype
}

void SkypeAccount::setMarkRead(bool value) {
	d->markRead = value;//remember it
	d->skype.setMarkMode(value);
}

bool SkypeAccount::userHasChat(const QString &userId) {
	SkypeContact *cont = static_cast<SkypeContact *> (contacts().find(userId));//get the contact

	if (cont)//it exists
		return cont->hasChat();//so ask it
	else
		return false;//if it does not exist it can not have a chat opened
}

void SkypeAccount::receivedIm(const QString &user, const QString &message, const QString &messageId) {
	kdDebug(14311) << k_funcinfo << "User: " << user << ", message: " << message << endl;//some debug info
	getContact(user)->receiveIm(message, getMessageChat(messageId));//let the contact show the message
}

void SkypeAccount::setScanForUnread(bool value) {
	d->searchForUnread = value;
	d->skype.setScanForUnread(value);
}

bool SkypeAccount::getScanForUnread() const {
	return d->searchForUnread;
}

void SkypeAccount::makeCall(SkypeContact *user) {
	makeCall(user->contactId());
}

void SkypeAccount::makeCall(const QString &users) {
	startCall();
	d->skype.makeCall(users);
}

bool SkypeAccount::getCallControl() const {
	return d->callControl;
}

void SkypeAccount::setCallControl(bool value) {
	d->callControl = value;
}

void SkypeAccount::newCall(const QString &callId, const QString &userId) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (d->callControl) {//Show the skype call control window
		SkypeCallDialog *dialog = new SkypeCallDialog(callId, userId, this);//It should free itself when it is closed
		QObject::connect(&d->skype, SIGNAL(callStatus(const QString&, const QString& )), dialog, SLOT(updateStatus(const QString&, const QString& )));
		QObject::connect(dialog, SIGNAL(acceptTheCall(const QString& )), &d->skype, SLOT(acceptCall(const QString& )));
		QObject::connect(dialog, SIGNAL(hangTheCall(const QString& )), &d->skype, SLOT(hangUp(const QString& )));
		QObject::connect(dialog, SIGNAL(toggleHoldCall(const QString& )), &d->skype, SLOT(toggleHoldCall(const QString& )));
		QObject::connect(&d->skype, SIGNAL(callError(const QString&, const QString& )), dialog, SLOT(updateError(const QString&, const QString& )));
		QObject::connect(&d->skype, SIGNAL(skypeOutInfo(int, const QString& )), dialog, SLOT(skypeOutInfo(int, const QString& )));
		QObject::connect(dialog, SIGNAL(updateSkypeOut()), &d->skype, SLOT(getSkypeOut()));
		QObject::connect(dialog, SIGNAL(callFinished(const QString& )), this, SLOT(removeCall(const QString& )));
		d->skype.getSkypeOut();

		d->calls.insert(callId, dialog);
	}

	if ((!d->incommingCommand.isEmpty()) && (d->skype.isCallIncoming(callId))) {
		kdDebug(14311) << "Running ring command" << endl;
		KProcess *proc = new KProcess();
		(*proc) << QStringList::split(' ', d->incommingCommand);
		QObject::connect(proc, SIGNAL(processExited(KProcess* )), proc, SLOT(deleteLater()));
		proc->start();
	}
}

bool SkypeAccount::isCallIncoming(const QString &callId) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	return d->skype.isCallIncoming(callId);
}

void SkypeAccount::setCloseWindowTimeout(int timeout) {
	d->callWindowTimeout = timeout;
}

int SkypeAccount::closeCallWindowTimeout() const {
	return d->callWindowTimeout;
}

QString SkypeAccount::getUserLabel(const QString &userId) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	if (userId.find(' ') != -1) {//there are more people than just one
		QStringList users = QStringList::split(' ', userId);
		for (QStringList::iterator it = users.begin(); it != users.end(); it++) {
			(*it) = getUserLabel((*it).stripWhiteSpace());
		}
		return users.join("\n");
	}

	Kopete::Contact *cont = contact(userId);

	if (!cont) {
		addContact(userId, QString::null, 0L, Temporary);//create a temporary contact

		cont = (contacts().find(userId));//It should be there now
		if (!cont)
			return userId;//something odd,.but better do nothing than crash
	}

	return QString("%1 (%2)").arg(cont->nickName()).arg(userId);
}

void SkypeAccount::setPings(bool enabled) {
	d->skype.enablePings(enabled);
	d->pings = enabled;
}

bool SkypeAccount::getPings() const {
	return d->pings;
}

int SkypeAccount::getBus() const {
	return d->bus;
}

void SkypeAccount::setBus(int bus) {
	d->bus = bus;
	d->skype.setBus(bus);
}

void SkypeAccount::setStartDBus(bool enable) {
	d->startDBus = enable;
	d->skype.setStartDBus(enable);
}

bool SkypeAccount::getStartDBus() const {
	return d->startDBus;
}

void SkypeAccount::setLaunchTimeout(int seconds) {
	d->launchTimeout = seconds;
	d->skype.setLaunchTimeout(seconds);
}

int SkypeAccount::getLaunchTimeout() const {
	return d->launchTimeout;
}

void SkypeAccount::setSkypeCommand(const QString &command) {
	d->skypeCommand = command;
	d->skype.setSkypeCommand(command);
}

const QString &SkypeAccount::getSkypeCommand() const {
	return d->skypeCommand;
}

void SkypeAccount::setMyselfName(const QString &name) {
	d->myName = name;
	myself()->setNickName(name);
}

void SkypeAccount::setWaitBeforeConnect(int value) {
	d->waitBeforeConnect = value;
	d->skype.setWaitConnect(value);
}

int SkypeAccount::getWaitBeforeConnect() const {
	return d->waitBeforeConnect;
}

SkypeContact *SkypeAccount::getContact(const QString &userId) {
	SkypeContact *cont = static_cast<SkypeContact *> (contacts().find(userId));//get the contact
	if (!cont) {//We do not know such contact
		addContact(userId, QString::null, 0L, Temporary);//create a temporary contact

		cont = static_cast<SkypeContact *> (contacts().find(userId));//It should be there now
	}
	return cont;
}

void SkypeAccount::prepareChatSession(SkypeChatSession *session) {
	QObject::connect(session, SIGNAL(updateChatId(const QString&, const QString&, SkypeChatSession* )), this, SLOT(setChatId(const QString&, const QString&, SkypeChatSession* )));
	QObject::connect(session, SIGNAL(wantTopic(const QString& )), &d->skype, SLOT(getTopic(const QString& )));
	QObject::connect(&d->skype, SIGNAL(joinUser(const QString&, const QString& )), session, SLOT(joinUser(const QString&, const QString& )));
	QObject::connect(&d->skype, SIGNAL(leftUser(const QString&, const QString&, const QString& )), session, SLOT(leftUser(const QString&, const QString&, const QString& )));
	QObject::connect(&d->skype, SIGNAL(setTopic(const QString&, const QString& )), session, SLOT(setTopic(const QString&, const QString& )));
	QObject::connect(session, SIGNAL(inviteUserToChat(const QString&, const QString& )), &d->skype, SLOT(inviteUser(const QString&, const QString& )));
	QObject::connect(session, SIGNAL(leaveChat(const QString& )), &d->skype, SLOT(leaveChat(const QString& )));
}

void SkypeAccount::setChatId(const QString &oldId, const QString &newId, SkypeChatSession *sender) {
	d->sessions.remove(oldId);//remove the old one
	if (!newId.isEmpty()) {
		d->sessions.insert(newId, sender);
	}
}

bool SkypeAccount::chatExists(const QString &chat) {
	return d->sessions.find(chat);
}

void SkypeAccount::receiveMultiIm(const QString &chatId, const QString &body, const QString &messageId, const QString &user) {
	SkypeChatSession *session = d->sessions.find(chatId);

	if (!session) {
		QStringList users = d->skype.getChatUsers(chatId);
		Kopete::ContactPtrList list;
		for (QStringList::iterator it = users.begin(); it != users.end(); ++it) {
			list.append(getContact(*it));
		}

		session = new SkypeChatSession(this, chatId, list);
	}

	Kopete::Message mes(getContact(user), myself(), body, Kopete::Message::Inbound);
	session->appendMessage(mes);
}

QString SkypeAccount::getMessageChat(const QString &messageId) {
	return d->skype.getMessageChat(messageId);
}

void SkypeAccount::registerLastSession(SkypeChatSession *lastSession) {
	d->lastSession = lastSession;
}

void SkypeAccount::gotMessageId(const QString &messageId) {
	if ((d->lastSession) && (!messageId.isEmpty())) {
		d->lastSession->setChatId(d->skype.getMessageChat(messageId));
	}

	d->lastSession = 0L;
}

void SkypeAccount::sentMessage(const QString &body, const QString &chat) {
	kdDebug(14311) << k_funcinfo << "chat: " << chat << endl;//some debug info

	SkypeChatSession *session = d->sessions.find(chat);
	const QStringList &users = d->skype.getChatUsers(chat);
	QPtrList<Kopete::Contact> *recv = 0L;

	if (!session)
		if (d->hitch) {
			recv = constructContactList(users);
			if (recv->count() == 1) {
				SkypeContact *cont = static_cast<SkypeContact *> (recv->at(0));
				cont->startChat();
				session = cont->getChatSession();
				session->setChatId(chat);
			} else {
				session = new SkypeChatSession(this, chat, *recv);
			}
		} else {
			return;
		}

	if (!recv)
		recv = constructContactList(users);

	session->sentMessage(recv, body);
	delete recv;
}

QPtrList<Kopete::Contact> *SkypeAccount::constructContactList(const QStringList &users) {
	QPtrList<Kopete::Contact> *list= new QPtrList<Kopete::Contact> ();
	for (QStringList::const_iterator it = users.begin(); it != users.end(); ++it) {
		list->append(getContact(*it));
	}

	return list;
}

void SkypeAccount::groupCall(const QString &callId, const QString &groupId) {
	kdDebug(14311) << k_funcinfo << endl;

	//TODO: Find out a way to embet qdialog into another one after creation
	return;

	if (!d->callControl)
		return;

	SkypeConference *conf;
	if (!(conf = d->conferences[groupId])) {//does it already exist?
		conf = new SkypeConference(groupId);//no, create one then..
		d->conferences.insert(groupId, conf);

		QObject::connect(conf, SIGNAL(removeConference(const QString& )), this, SLOT(removeCallGroup(const QString& )));
	}

	conf->embedCall(d->calls[callId]);
}

void SkypeAccount::removeCall(const QString &callId) {
	kdDebug(14311) << k_funcinfo << endl;
	d->calls.remove(callId);
}

void SkypeAccount::removeCallGroup(const QString &groupId) {
	kdDebug(14311) << k_funcinfo << endl;
	d->conferences.remove(groupId);
}

QString SkypeAccount::createChat(const QString &users) {
	return d->skype.createChat(users);
}

bool SkypeAccount::leaveOnExit() const {
	return d->leaveOnExit;
}

void SkypeAccount::setLeaveOnExit(bool value) {
	d->leaveOnExit = value;
}

void SkypeAccount::chatUser(const QString &userId) {
	SkypeContact *contact = getContact(userId);

	contact->execute();
}

void SkypeAccount::setStartCallCommand(const QString &value) {
	d->startCallCommand = value;
}

void SkypeAccount::setEndCallCommand(const QString &value) {
	d->endCallCommand = value;
}

void SkypeAccount::setWaitForStartCallCommand(bool value) {
	d->waitForStartCallCommand = value;
}
void SkypeAccount::setEndCallCommandOnlyForLast(bool value) {
	d->endCallCommandOnlyLats = value;
}

QString SkypeAccount::startCallCommand() const {
	return d->startCallCommand;
}

QString SkypeAccount::endCallCommand() const {
	return d->endCallCommand;
}

bool SkypeAccount::waitForStartCallCommand() const {
	return d->waitForStartCallCommand;
}

bool SkypeAccount::endCallCommandOnlyLast() const {
	return d->endCallCommandOnlyLats;
}

void SkypeAccount::startCall() {
	kdDebug(14311) << k_funcinfo << endl;

	KProcess *proc = new KProcess();
	QObject::connect(proc, SIGNAL(processExited(KProcess* )), proc, SLOT(deleteLater()));
	QStringList args = QStringList::split(' ', d->startCallCommand);
	(*proc) << args;//set what will be executed
	KProcess::RunMode mode = d->waitForStartCallCommand ? KProcess::Block : KProcess::NotifyOnExit;
	proc->start(mode);
	++d->callCount;
}

void SkypeAccount::endCall() {
	kdDebug(14311) << k_funcinfo << endl;

	if ((--d->callCount == 0) || (!d->endCallCommandOnlyLats)) {
		KProcess *proc = new KProcess();
		QObject::connect(proc, SIGNAL(processExited(KProcess* )), proc, SLOT(deleteLater()));
		(*proc) << QStringList::split(' ', d->endCallCommand);
		proc->start();
	}
	if (d->callCount < 0)
		d->callCount = 0;
}

void SkypeAccount::setIncomingCommand(const QString &command) {
	d->incommingCommand = command;
}

QString SkypeAccount::incomingCommand() const {
	return d->incommingCommand;
}

void SkypeAccount::registerContact(const QString &contactId) {
	d -> skype.addContact(contactId);
}

void SkypeAccount::removeContact(const QString &contactId) {
	d -> skype.removeContact(contactId);
}

bool SkypeAccount::ableMultiCall() {
	return (d->skype.ableConference());
}

bool SkypeAccount::canAlterAuth() {
	return (d->skype.canComunicate());
}

void SkypeAccount::authorizeUser(const QString &userId) {
	d->skype.setAuthor(userId, Skype::Author);
}

void SkypeAccount::disAuthorUser(const QString &userId) {
	d->skype.setAuthor(userId, Skype::Deny);
}

void SkypeAccount::blockUser(const QString &userId) {
	d->skype.setAuthor(userId, Skype::Block);
}

#include "skypeaccount.moc"
