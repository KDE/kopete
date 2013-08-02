/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>
    Copyright (C) 2008-2009 Pali Rohár <pali.rohar@gmail.com>

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
#include <kopetestatusmanager.h>
#include <kopetecontactlist.h>
#include <kopetecontact.h>
#include <kopetegroup.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kconfigbase.h>
#include <QHash>
#include <kopetemessage.h>
#include <qprocess.h>
#include <kopeteaddedinfoevent.h>

#include <QDateTime>
#include <QPointer>

class SkypeAccountPrivate {
	public:
		///The skype protocol pointer
		QPointer <SkypeProtocol> protocol;
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
		///Constructor
		SkypeAccountPrivate(SkypeAccount &account) : skype(account) {};//just an empty constructor
		///Automatic close of call window when the call finishes (in seconds, 0 -> disabled)
		int callWindowTimeout;
		///Are the pings enabled?
		bool pings;
		///What bus are we using, session (0) or system (1)?
		int bus;
		///How long can I keep trying connect to newly started skype, before I give up (seconds)
		int launchTimeout;
		///By what command is the skype started?
		QString skypeCommand;
		///What is my name, by the way?
		QString myName;
		///Do we wait before connecting?
		int waitBeforeConnect;
		///List of chat all chat sessions
		QHash<QString, QPointer<SkypeChatSession> > sessions;
		///Last used chat session
		QPointer <SkypeChatSession> lastSession;
		///List of the conference calls
		QHash<QString, SkypeConference*> conferences;
		///List of existing calls
		QHash<QString, SkypeCallDialog*> calls;
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

SkypeAccount::SkypeAccount(SkypeProtocol *protocol, const QString& accountID) : Kopete::Account(protocol, accountID ) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	//keep track of what accounts the protocol has
	protocol->registerAccount(this);

	//the d pointer
	d = new SkypeAccountPrivate(*this);
	//remember the protocol, it will be needed
	d->protocol = protocol;

	//load the properties
	KConfigGroup *config = configGroup();
	author = config->readEntry("Authorization");//get the name how to authorize myself
	launchType = config->readEntry("Launch", 0);//launch the skype?
	setScanForUnread(config->readEntry("ScanForUnread", true));
	setCallControl(config->readEntry("CallControl", false));
	setPings(config->readEntry("Pings", true));
	setBus(config->readEntry("Bus", 1));
	setLaunchTimeout(config->readEntry("LaunchTimeout", 70));
	d->myName = config->readEntry("MyselfName", "Skype");
	setSkypeCommand(config->readEntry("SkypeCommand", "skype"));
	setWaitBeforeConnect(config->readEntry("WaitBeforeConnect", 0));
	setLeaveOnExit(config->readEntry("LeaveOnExit", false));
	setStartCallCommand(config->readEntry("StartCallCommand", ""));
	setEndCallCommand(config->readEntry("EndCallCommand", ""));
	setWaitForStartCallCommand(config->readEntry("WaitForStartCallCommand", false));
	setEndCallCommandOnlyForLast(config->readEntry("EndCallCommandOnlyLast", false));
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
	QObject::connect(&d->skype, SIGNAL(newUser(QString,int)), this, SLOT(newUser(QString,int)));
	QObject::connect(&d->skype, SIGNAL(contactInfo(QString,QString)), this, SLOT(updateContactInfo(QString,QString)));
	QObject::connect(&d->skype, SIGNAL(receivedIM(QString,QString,QString,QDateTime)), this, SLOT(receivedIm(QString,QString,QString,QDateTime)));
	QObject::connect(&d->skype, SIGNAL(gotMessageId(QString)), this, SLOT(gotMessageId(QString)));//every time some ID is known inform the contacts
	QObject::connect(&d->skype, SIGNAL(newCall(QString,QString)), this, SLOT(newCall(QString,QString)));
	QObject::connect(&d->skype, SIGNAL(setMyselfName(QString)), this, SLOT(setMyselfName(QString)));
	QObject::connect(&d->skype, SIGNAL(receivedMultiIM(QString,QString,QString,QString,QDateTime)), this, SLOT(receiveMultiIm(QString,QString,QString,QString,QDateTime)));
	QObject::connect(&d->skype, SIGNAL(outgoingMessage(QString,QString,QString)), this, SLOT(sentMessage(QString,QString,QString)));
	QObject::connect(&d->skype, SIGNAL(groupCall(QString,QString)), this, SLOT(groupCall(QString,QString)));
	QObject::connect(&d->skype, SIGNAL(receivedAuth(QString,QString)), this, SLOT(receivedAuth(QString,QString)));
	QObject::connect(Kopete::ContactList::self(), SIGNAL(groupRemoved(Kopete::Group*)), this, SLOT(deleteGroup(Kopete::Group*)) );
	QObject::connect(Kopete::ContactList::self(), SIGNAL(groupRenamed(Kopete::Group*,QString)), this, SLOT(renameGroup(Kopete::Group*,QString)) );

	//set values for the connection (should be updated if changed)
	d->skype.setValues(launchType, author);
	setHitchHike(config->readEntry("Hitch", true));
	setMarkRead(config->readEntry("MarkRead", true));//read the modes of account
	d->callWindowTimeout = config->readEntry("CloseWindowTimeout", 4);
	setPings(config->readEntry("Pings", true));
	d->lastSession = 0L;
	d->callCount = 0;
}


SkypeAccount::~SkypeAccount() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	save();

	if (d->protocol)
		d->protocol->unregisterAccount();//This account no longer exists

	//free memory
	delete d;
}

bool SkypeAccount::createContact(const QString &contactID, Kopete::MetaContact *parentContact) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (!contact(contactID)) {//check weather it is not used already
		SkypeContact *newContact = new SkypeContact(this, contactID, parentContact);//create the contact

		return newContact != 0L;//test weather it was created
	} else {
		kDebug(SKYPE_DEBUG_GLOBAL) << "Contact already exists:" << contactID;//Tell that it is not OK

		return false;
	}
}

void SkypeAccount::setAway(bool away, const QString &reason) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (away)
		setOnlineStatus(d->protocol->Away, reason);
	else
		setOnlineStatus(d->protocol->Online, reason);
}

void SkypeAccount::setOnlineStatus(const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& options) {
	kDebug(SKYPE_DEBUG_GLOBAL) << "status message:" << reason.message();
	Q_UNUSED(options);
	if (status == d->protocol->Online){
		d->skype.setOnline();//Go online
		setStatusMessage(reason);
	}
	else if (status == d->protocol->Offline){
		if (!reason.isEmpty()) setStatusMessage(reason);
		d->skype.setOffline();//Go offline
	}
	else if (status == d->protocol->Away){
		d->skype.setAway();
		setStatusMessage(reason);
	}
	else if (status == d->protocol->NotAvailable){
		d->skype.setNotAvailable();
		setStatusMessage(reason);
	}
	else if (status == d->protocol->DoNotDisturb){
		d->skype.setDND();
		setStatusMessage(reason);
	}
	else if (status == d->protocol->Invisible){
		d->skype.setInvisible();
		setStatusMessage(reason);
	}
	else if (status == d->protocol->SkypeMe){
		d->skype.setSkypeMe();
		setStatusMessage(reason);
	}
	else
		kDebug(SKYPE_DEBUG_GLOBAL) << "Unknown online status";//Just a warning that I do not know that status;
}

void SkypeAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
	d->skype.setUserProfileRichMoodText(statusMessage.message());
	myself()->setStatusMessage(statusMessage.message());
}

void SkypeAccount::disconnect() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	setOnlineStatus(d->protocol->Offline, Kopete::StatusManager::self()->globalStatusMessage());
}

SkypeContact *SkypeAccount::contact(const QString &id) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	return static_cast<SkypeContact *>(contacts().value(id));//get the contact and convert it into the skype contact, there are no other contacts anyway
}

void SkypeAccount::connect(const Kopete::OnlineStatus &Status) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if ((Status != d->protocol->Online) && (Status != d->protocol->Away) &&
		(Status != d->protocol->NotAvailable) && (Status != d->protocol->DoNotDisturb) &&
		(Status != d->protocol->SkypeMe))//some strange online status, taje a default one
			setOnlineStatus(d->protocol->Online, Kopete::StatusManager::self()->globalStatusMessage());
	else
		setOnlineStatus(Status, Kopete::StatusManager::self()->globalStatusMessage());//just change the status

}

void SkypeAccount::save() {
	kDebug(SKYPE_DEBUG_GLOBAL);

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
	kDebug(SKYPE_DEBUG_GLOBAL);

	myself()->setOnlineStatus(d->protocol->Online);//just set the icon
	d->skype.enablePings(d->pings);
	emit connectionStatus(true);
}

void SkypeAccount::wentOffline() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (myself())
		myself()->setOnlineStatus(d->protocol->Offline);//just change the icon
	emit connectionStatus(false);
}

void SkypeAccount::wentAway() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	myself()->setOnlineStatus(d->protocol->Away);//just change the icon
	emit connectionStatus(true);
}

void SkypeAccount::wentNotAvailable() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	myself()->setOnlineStatus(d->protocol->NotAvailable);
	emit connectionStatus(true);
}

void SkypeAccount::wentDND() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	myself()->setOnlineStatus(d->protocol->DoNotDisturb);
	emit connectionStatus(true);
}

void SkypeAccount::wentInvisible() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	myself()->setOnlineStatus(d->protocol->Invisible);
	emit connectionStatus(true);
}

void SkypeAccount::wentSkypeMe() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	myself()->setOnlineStatus(d->protocol->SkypeMe);
	emit connectionStatus(true);
}

void SkypeAccount::statusConnecting() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	myself()->setOnlineStatus(d->protocol->Connecting);
	emit connectionStatus(false);
}

void SkypeAccount::newUser(const QString &name, int groupID) {
	kDebug(SKYPE_DEBUG_GLOBAL) << QString("name = %1").arg(name) << QString("groupID = %1").arg(groupID);

	if (name == "echo123")// echo123 - Make Test Call has moved to Skype protocol toolbar
		return;

	QString group = d->skype.getGroupName(groupID);

	Kopete::Group * skypeGroup;

	bool root = false;
	if (group.isEmpty() || groupID == -1) { // If skype group has not name, in kopete will be in top
		skypeGroup = Kopete::Group::topLevel();
		root = true;
	} else {
		skypeGroup = Kopete::ContactList::self()->findGroup(group); //get kopete group by skype group name. If skype group in kopete does not exist, create it automatically
		if ( skypeGroup == Kopete::Group::topLevel() ){ //if group in skype has name i18n("Top Level") kopete get top level group, but in skype top level group is group without name
			QList <Kopete::Group *> groups = Kopete::ContactList::self()->groups(); //get all groups
			bool found = false;
			for (QList <Kopete::Group *>::iterator it = groups.begin(); it != groups.end(); ++it ){ //search all groups, if one isnt top level and has skype group name
				if ( (*it)->displayName() == group && (*it) != Kopete::Group::topLevel() ){
					skypeGroup = (*it);
					found = true; //if found skip creating new
				}
			}
			if ( !found ){
				skypeGroup = new Kopete::Group(group); //create new group with name
				Kopete::ContactList::self()->addGroup(skypeGroup); //add this new group to contact list
			}
		}
	}

	Kopete::Contact * contact = contacts().value(name);
	if (contact){
		if (!root) {
			if (skypeGroup != contact->metaContact()->groups().first()){ //if skype Group is different like kopete group (and skype group isnt root), move metacontact to skype group
				kDebug(SKYPE_DEBUG_GLOBAL) << "Moving contact" << name << "to group" << group;
				contact->metaContact()->moveToGroup(contact->metaContact()->groups().first(), skypeGroup);
			}
		} else { //if skype contact is in root group, move it to kopete group
			kDebug(SKYPE_DEBUG_GLOBAL) << "Moving contact" << name << "in skype client to kopete group";
			MovedBetweenGroup(static_cast <SkypeContact *> (contact));
		}
		return;
	}

	addContact(name, d->skype.getContactDisplayName(name), skypeGroup, ChangeKABC);
}

void SkypeAccount::prepareContact(SkypeContact *contact) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	QObject::connect(&d->skype, SIGNAL(updateAllContacts()), contact, SLOT(requestInfo()));//all contacts will know that
	QObject::connect(contact, SIGNAL(infoRequest(QString)), &d->skype, SLOT(getContactInfo(QString)));//How do we ask for info?
	QObject::connect(this, SIGNAL(connectionStatus(bool)), contact, SLOT(connectionStatus(bool)));
	QObject::connect(contact, SIGNAL(setActionsPossible(bool)), d->protocol, SLOT(updateCallActionStatus()));
}

void SkypeAccount::updateContactInfo(const QString &contact, const QString &change) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	SkypeContact *cont = static_cast<SkypeContact *> (contacts().value(contact));//get the contact
	if (cont)
		cont->setInfo(change);//give it the message
	else {//it does not yet exist, create it if it is in skype contact list (can be got by buddystatus)
		const QString &type = change.section(' ', 0, 0).trimmed().toUpper();//get the first part of the message, it should be BUDDYSTATUS
		const QString &value = change.section(' ', 1, 1).trimmed();//get the second part if it is some reasonable value
		if ((type == "BUDDYSTATUS") && ((value == "2") || (value == "3"))) {//the user is in skype contact list
			newUser(contact, d->skype.getContactGroupID(contact));
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
	kDebug(SKYPE_DEBUG_GLOBAL);

	QString id;
	if (chat.isEmpty()) {
		const QString &user = message.to().at(0)->contactId();//get id of the first contact, messages to multiple people are not yet possible
		const QString &body = message.plainBody().trimmed();//get the text of the message

		id = d->skype.send(user, body);//send it by skype
	} else {
		const QString &body = message.plainBody().trimmed();
		id = d->skype.sendToChat(chat, body);
	}

	// Append message to chat session
	QString chatId = d->skype.getMessageChat(id);

	// Use last session if available (could be set by skypechatsession)
	SkypeChatSession *session = d->lastSession?d->lastSession:d->sessions.value(chatId);

	if(session)
		session->sentMessage(message, id);
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
	SkypeContact *cont = static_cast<SkypeContact *> (contacts().value(userId));//get the contact

	if (cont)//it exists
		return cont->hasChat();//so ask it
	else
		return false;//if it does not exist it can not have a chat opened
}

void SkypeAccount::receivedIm(const QString &user, const QString &message, const QString &messageId, const QDateTime &timeStamp) {
	kDebug(SKYPE_DEBUG_GLOBAL) << "User: " << user << ", message: " << message;
	getContact(user)->receiveIm(message, getMessageChat(messageId), timeStamp);//let the contact show the message
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

void SkypeAccount::makeTestCall() {
	makeCall("echo123");
}

bool SkypeAccount::getCallControl() const {
	return d->callControl;
}

void SkypeAccount::setCallControl(bool value) {
	d->callControl = value;
}

void SkypeAccount::newCall(const QString &callId, const QString &userId) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (d->callControl) {//Show the kopete call control window
		SkypeCallDialog *dialog = new SkypeCallDialog(callId, userId, this);//It should free itself when it is closed

		QObject::connect(&d->skype, SIGNAL(callStatus(QString,QString)), dialog, SLOT(updateStatus(QString,QString)));
		QObject::connect(dialog, SIGNAL(acceptTheCall(QString)), &d->skype, SLOT(acceptCall(QString)));
		QObject::connect(dialog, SIGNAL(hangTheCall(QString)), &d->skype, SLOT(hangUp(QString)));
		QObject::connect(dialog, SIGNAL(toggleHoldCall(QString)), &d->skype, SLOT(toggleHoldCall(QString)));
		QObject::connect(&d->skype, SIGNAL(callError(QString,QString)), dialog, SLOT(updateError(QString,QString)));
		QObject::connect(&d->skype, SIGNAL(skypeOutInfo(int,QString)), dialog, SLOT(skypeOutInfo(int,QString)));
		QObject::connect(dialog, SIGNAL(updateSkypeOut()), &d->skype, SLOT(getSkypeOut()));
		QObject::connect(dialog, SIGNAL(callFinished(QString)), this, SLOT(removeCall(QString)));
		QObject::connect(&d->skype, SIGNAL(startReceivingVideo(QString)), dialog, SLOT(startReceivingVideo(QString)));
		QObject::connect(&d->skype, SIGNAL(stopReceivingVideo(QString)), dialog, SLOT(stopReceivingVideo(QString)));

		dialog->show();//Show Call dialog

		d->skype.getSkypeOut();
		d->calls.insert(callId, dialog);
	}

	if ((!d->incommingCommand.isEmpty()) && (d->skype.isCallIncoming(callId))) {
		kDebug(SKYPE_DEBUG_GLOBAL) << "Running ring command";
		QProcess *proc = new QProcess();
		QStringList args = d->incommingCommand.split(' ');
		QString bin = args.takeFirst();
		proc->start(bin, args);
	}
}

bool SkypeAccount::isCallIncoming(const QString &callId) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	return d->skype.isCallIncoming(callId);
}

void SkypeAccount::setCloseWindowTimeout(int timeout) {
	d->callWindowTimeout = timeout;
}

int SkypeAccount::closeCallWindowTimeout() const {
	return d->callWindowTimeout;
}

QString SkypeAccount::getUserLabel(const QString &userId) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (userId.contains(' ')) {//there are more people than just one
		QStringList users = userId.split(' ');
		for (QStringList::iterator it = users.begin(); it != users.end(); ++it) {
			(*it) = getUserLabel((*it).trimmed());
		}
		return users.join("\n");
	}

	Kopete::Contact *cont = contact(userId);

	if (!cont) {
		addContact(userId, d->skype.getContactDisplayName(userId), 0L, Temporary);//create a temporary contact

		cont = (contacts().value(userId));//It should be there now
		if (!cont)
			return userId;//something odd,.but better do nothing than crash
	}

	return QString("%1 (%2)").arg(cont->displayName()).arg(userId);
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
	SkypeContact *cont = static_cast<SkypeContact *> (contacts().value(userId));//get the contact
	if (!cont) {//We do not know such contact
		addContact(userId, d->skype.getContactDisplayName(userId), 0L, Temporary);//create a temporary contact

		cont = static_cast<SkypeContact *> (contacts().value(userId));//It should be there now
	}
	return cont;
}

void SkypeAccount::prepareChatSession(SkypeChatSession *session) {
	QObject::connect(session, SIGNAL(updateChatId(QString,QString,SkypeChatSession*)), this, SLOT(setChatId(QString,QString,SkypeChatSession*)));
	QObject::connect(session, SIGNAL(wantTopic(QString)), &d->skype, SLOT(getTopic(QString)));
	QObject::connect(&d->skype, SIGNAL(joinUser(QString,QString)), session, SLOT(joinUser(QString,QString)));
	QObject::connect(&d->skype, SIGNAL(leftUser(QString,QString,QString)), session, SLOT(leftUser(QString,QString,QString)));
	QObject::connect(&d->skype, SIGNAL(setTopic(QString,QString)), session, SLOT(setTopic(QString,QString)));
	QObject::connect(session, SIGNAL(inviteUserToChat(QString,QString)), &d->skype, SLOT(inviteUser(QString,QString)));
	QObject::connect(session, SIGNAL(leaveChat(QString)), &d->skype, SLOT(leaveChat(QString)));
}

void SkypeAccount::setChatId(const QString &oldId, const QString &newId, SkypeChatSession *sender) {
	d->sessions.remove(oldId);//remove the old one
	if (!newId.isEmpty()) {
		d->sessions.insert(newId, sender);
	}
}

bool SkypeAccount::chatExists(const QString &chat) {
	return d->sessions.value(chat);
}

void SkypeAccount::receiveMultiIm(const QString &chatId, const QString &body, const QString &messageId, const QString &user, const QDateTime &timeStamp) {
	SkypeChatSession *session = d->sessions.value(chatId);

	if (!session) {
		QStringList users = d->skype.getChatUsers(chatId);
		Kopete::ContactPtrList list;
		for (QStringList::iterator it = users.begin(); it != users.end(); ++it) {
			list.append(getContact(*it));
		}

		session = new SkypeChatSession(this, chatId, list);
	}

	Kopete::Message mes(getContact(user), myself());
	mes.setDirection(Kopete::Message::Inbound);
	mes.setPlainBody(body);
	mes.setTimestamp(timeStamp);
	session->appendMessage(mes);

	Q_UNUSED(messageId);
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

void SkypeAccount::sentMessage(const QString &id, const QString &body, const QString &chat) {
	kDebug(SKYPE_DEBUG_GLOBAL) << "chat: " << chat;

	SkypeChatSession *session = d->sessions.value(chat);

	if ( ! session || session->ackMessage(id, false) )
		return;

	const QStringList &users = d->skype.getChatUsers(chat);
	QList<Kopete::Contact*> *recv = 0L;

	if (!session) {
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
	}

	if (!recv)
		recv = constructContactList(users);

	session->sentMessage(recv, body);
	delete recv;
}

QList<Kopete::Contact*> *SkypeAccount::constructContactList(const QStringList &users) {
	QList<Kopete::Contact*> *list= new QList<Kopete::Contact*> ();
	for (QStringList::const_iterator it = users.begin(); it != users.end(); ++it) {
		list->append(getContact(*it));
	}

	return list;
}

void SkypeAccount::groupCall(const QString &callId, const QString &groupId) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	//TODO: Find out a way to embet qdialog into another one after creation
	return;

	if (!d->callControl)
		return;

	SkypeConference *conf;
	if (!(conf = d->conferences.value(groupId))) {//does it already exist?
		conf = new SkypeConference(groupId);//no, create one then..
		d->conferences.insert(groupId, conf);

		QObject::connect(conf, SIGNAL(removeConference(QString)), this, SLOT(removeCallGroup(QString)));
	}

	conf->embedCall(d->calls.value(callId));
}

void SkypeAccount::removeCall(const QString &callId) {
	kDebug(SKYPE_DEBUG_GLOBAL);
	d->calls.remove(callId);
}

void SkypeAccount::removeCallGroup(const QString &groupId) {
	kDebug(SKYPE_DEBUG_GLOBAL);
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
	kDebug(SKYPE_DEBUG_GLOBAL);

	QProcess *proc = new QProcess();
	QStringList args = d->startCallCommand.split(' ');
	QString bin = args.takeFirst();
	if (d->waitForStartCallCommand)
		proc->execute(bin, args);
	else
		proc->start(bin, args);
	++d->callCount;
}

void SkypeAccount::endCall() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if ((--d->callCount == 0) || (!d->endCallCommandOnlyLats)) {
		QProcess *proc = new QProcess();
		QStringList args = d->endCallCommand.split(' ');
		QString bin = args.takeFirst();
		proc->start(bin, args);
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
	kDebug(SKYPE_DEBUG_GLOBAL);
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

int SkypeAccount::getAuthor(const QString &contactId) {
	switch (d->skype.getAuthor(contactId)) {
		case Skype::Author:
			return 0;
		case Skype::Deny:
			return 1;
		case Skype::Block:
			return 2;
	}
	return 0;
}

bool SkypeAccount::hasCustomStatusMenu() const {
	kDebug(SKYPE_DEBUG_GLOBAL);
	return true;
}

void SkypeAccount::fillActionMenu( KActionMenu *actionMenu ) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	actionMenu->setIcon( myself()->onlineStatus().iconFor(this) );
	actionMenu->menu()->addTitle( QIcon(myself()->onlineStatus().iconFor(this)), i18n("Skype (%1)", accountId()));

	if (d->protocol)
	{
		KAction *setOnline = new KAction( KIcon(QIcon(d->protocol->Online.iconFor(this))), i18n("Online"), this );
		QObject::connect( setOnline, SIGNAL(triggered(bool)), &d->skype, SLOT(setOnline()) );
		actionMenu->addAction(setOnline);

		KAction *setOffline = new KAction( KIcon(QIcon(d->protocol->Offline.iconFor(this))), i18n("Offline"), this );
		QObject::connect( setOffline, SIGNAL(triggered(bool)), &d->skype, SLOT(setOffline()) );
		actionMenu->addAction(setOffline);

		KAction *setAway = new KAction( KIcon(QIcon(d->protocol->Away.iconFor(this))), i18n("Away"), this );
		QObject::connect( setAway, SIGNAL(triggered(bool)), &d->skype, SLOT(setAway()) );
		actionMenu->addAction(setAway);

		KAction *setNotAvailable = new KAction( KIcon(QIcon(d->protocol->NotAvailable.iconFor(this))), i18n("Not Available"), this );
		QObject::connect( setNotAvailable, SIGNAL(triggered(bool)), &d->skype, SLOT(setNotAvailable()) );
		actionMenu->addAction(setNotAvailable);

		KAction *setDND = new KAction( KIcon(QIcon(d->protocol->DoNotDisturb.iconFor(this))), i18n("Do Not Disturb"), this );
		QObject::connect( setDND, SIGNAL(triggered(bool)), &d->skype, SLOT(setDND()) );
		actionMenu->addAction(setDND);

		KAction *setInvisible = new KAction( KIcon(QIcon(d->protocol->Invisible.iconFor(this))), i18n("Invisible"), this );
		QObject::connect( setInvisible, SIGNAL(triggered(bool)), &d->skype, SLOT(setInvisible()) );
		actionMenu->addAction(setInvisible);

		KAction *setSkypeMe = new KAction( KIcon(QIcon(d->protocol->SkypeMe.iconFor(this))), i18n("Skype Me"), this );
		QObject::connect( setSkypeMe, SIGNAL(triggered(bool)), &d->skype, SLOT(setSkypeMe()) );
		actionMenu->addAction(setSkypeMe);

		actionMenu->addSeparator();

		KAction *makeTestCall = new KAction( KIcon("voicecall"), i18n("Make Test Call"), this );
		QObject::connect( makeTestCall, SIGNAL(triggered(bool)), this, SLOT(makeTestCall()) );

		const Kopete::OnlineStatus &myStatus = (myself()) ? myself()->onlineStatus() : protocol()->Offline;
		if (myStatus == protocol()->Offline || myStatus == protocol()->Connecting){
			makeTestCall->setEnabled(false);
		}

		actionMenu->addAction(makeTestCall);

		actionMenu->addSeparator();

		KAction *properties = new KAction( i18n("Properties"), this );
		QObject::connect( properties, SIGNAL(triggered(bool)), this, SLOT(editAccount()) );
		actionMenu->addAction(properties);
	}
}

QString SkypeAccount::getMyselfSkypeName() {
	kDebug(SKYPE_DEBUG_GLOBAL);
	return d->skype.getMyself();
}

void SkypeAccount::MovedBetweenGroup(SkypeContact *contact) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	int newGroup = d->skype.getGroupID(contact->metaContact()->groups().first()->displayName());
	int oldGroup = d->skype.getContactGroupID(contact->contactId());

	kDebug(SKYPE_DEBUG_GLOBAL) << "oldGroup:" << oldGroup << "newGroup:" << newGroup;

	if ( oldGroup != -1 ){
		kDebug(SKYPE_DEBUG_GLOBAL) << "Removing contact" << contact->contactId() << "from group" << d->skype.getContactGroupID(contact->contactId());
		d->skype.removeFromGroup(contact->contactId(), oldGroup);
	}

	if ( newGroup == -1 ){
		if ( contact->metaContact()->groups().first() != Kopete::Group::topLevel() ){
			d->skype.createGroup(contact->metaContact()->groups().first()->displayName());
			newGroup = d->skype.getGroupID(contact->metaContact()->groups().first()->displayName());
		} else {
			kDebug(SKYPE_DEBUG_GLOBAL) << "Contact is in top level, so in no skype group, skipping";
			return;
		}
	}

	if ( newGroup != -1 ){
		kDebug(SKYPE_DEBUG_GLOBAL) << "Adding contact" << contact->contactId() << "to group" << d->skype.getGroupID(contact->metaContact()->groups().first()->displayName());
		d->skype.addToGroup(contact->contactId(), newGroup);
	} else
		kDebug(SKYPE_DEBUG_GLOBAL) << "Error: Cant create new skype group" << contact->metaContact()->groups().first()->displayName();
}

void SkypeAccount::deleteGroup (Kopete::Group * group){
	kDebug(SKYPE_DEBUG_GLOBAL) << group->displayName();
	int groupID = d->skype.getGroupID( group->displayName() );
	if ( groupID != -1 )
		d->skype.deleteGroup(groupID);
	else
		kDebug(SKYPE_DEBUG_GLOBAL) << "Group" << group->displayName() << "in skype does not exist, skipping";
}

void SkypeAccount::renameGroup (Kopete::Group * group, const QString &oldname){
	kDebug(SKYPE_DEBUG_GLOBAL) << "Renaming skype group" << oldname << "to" << group->displayName();
	int groupID = d->skype.getGroupID( oldname );
	if ( groupID != -1 )
		d->skype.renameGroup( groupID, group->displayName() );
	else
		kDebug(SKYPE_DEBUG_GLOBAL) << "Old group" << oldname << "in skype does not exist, skipping";
}

void SkypeAccount::openFileTransfer(const QString &user, const QString &url) {
	kDebug(SKYPE_DEBUG_GLOBAL) << user << url;
	d->skype.openFileTransfer(user, url);
}

void SkypeAccount::setContactDisplayName(const QString &user, const QString &name) {
	kDebug(SKYPE_DEBUG_GLOBAL) << user << name;
	d->skype.setContactDisplayName(user, name);
}

void SkypeAccount::receivedAuth(const QString &user, const QString &info) {
	Kopete::AddedInfoEvent* event = new Kopete::AddedInfoEvent(user, this);

	QObject::connect( event, SIGNAL(actionActivated(uint)), this, SLOT(authEvent(uint)) );

	Kopete::AddedInfoEvent::ShowActionOptions actions = Kopete::AddedInfoEvent::AuthorizeAction;
	actions |= Kopete::AddedInfoEvent::BlockAction;
	actions |= Kopete::AddedInfoEvent::InfoAction;

	Kopete::Contact * ct = contacts().value( user );
	if( !ct || !ct->metaContact() || ct->metaContact()->isTemporary() )
		actions |= Kopete::AddedInfoEvent::AddAction;

	if( ct )
		event->setContactNickname( ct->displayName() );

	event->showActions( actions );

	event->setAdditionalText(info);
	event->sendEvent();
}

void SkypeAccount::authEvent(uint actionId) {
	Kopete::AddedInfoEvent *event = dynamic_cast<Kopete::AddedInfoEvent *>(sender());

	if ( !event )
		return;

	switch ( actionId ) {
		case Kopete::AddedInfoEvent::AddContactAction:
			event->addContact();
			break;
		case Kopete::AddedInfoEvent::AuthorizeAction:
			authorizeUser(event->contactId());
			break;
		case Kopete::AddedInfoEvent::BlockAction:
			blockUser(event->contactId());
			break;
		case Kopete::AddedInfoEvent::InfoAction:
			userInfo(event->contactId());
			break;
	}
}

void SkypeAccount::startSendingVideo(const QString &callId) {
	d->skype.startSendingVideo(callId);
}

void SkypeAccount::stopSendingVideo(const QString &callId) {
	d->skype.stopSendingVideo(callId);
}

void SkypeAccount::userInfo(const QString &user) {
	kDebug(SKYPE_DEBUG_GLOBAL) << user;
	if ( ! contact(user) ) {
		addContact(user, d->skype.getContactDisplayName(user), 0L, Temporary);//create a temporary contact
		if ( ! contact(user) ) {
			KMessageBox::error(0L, i18n("Cannot open info about user %1", user), i18n("Skype protocol"));
			return;//contact arent in contact list - skip it
		}
	}
	contact(user)->slotUserInfo();//Open user info dialog
}

void SkypeAccount::SkypeActionHandler(const QString &message) {
	kDebug(SKYPE_DEBUG_GLOBAL) << message;

	//TODO:
	//Add all actions from https://developer.skype.com/Docs/ApiDoc/Skype_URI_handler

	if ( message.isEmpty() ) {
		KMessageBox::error(0L, i18n("Unknown action from SkypeActionHandler"), i18n("Skype protocol"));
		return;//Empty message
	}

	QString command;
	QString subcommands;
	QString user;

	if ( message.startsWith("callto:", Qt::CaseInsensitive) ) {
		command = "call";
		user = message.section(':', -1).section('/', -1).trimmed();
	} else if ( message.startsWith("tel:", Qt::CaseInsensitive) ) {
		command = "chat";
		user = message.section(':', -1).section('/', -1).trimmed();
	} else if ( message.startsWith("skype:", Qt::CaseInsensitive) ) {
		command = message.section('?', -1).section('&', 0, 0).trimmed();
		//TODO: Add support for subcommands
//		subcommands = QUrl::fromPercentEncoding(message.section('&', 1, -1).toUtf8());
		user = message.section(':', -1).section('?', 0, 0).trimmed();
		if ( command.isEmpty() ) //set default double click action = open chat window
			command = "chat";
	} else {
		KMessageBox::error(0L, i18n("Unknown action from SkypeActionHandler"), i18n("Skype protocol"));
		return;//Unknown message
	}

	if ( command.isEmpty() || user.isEmpty() ) {
		KMessageBox::error(0L, i18n("Unknown action from SkypeActionHandler"), i18n("Skype protocol"));
		return;//Unknown message
	}

	kDebug(SKYPE_DEBUG_GLOBAL) << "user:" << user << "command:" << command;

	if ( command == "add" ) {
		//TODO: Open add user dialog
		KMessageBox::error(0L, i18n("Add contact from SkypeActionHandler is not supported yet"), i18n("Skype protocol"));
	} else if ( command == "call" ) {
		//TODO: Add support for conference call
		makeCall(user);//Start call with user
	} else if ( command == "chat" ) {
		//TODO: Add support for multichat
		//TODO: Add support for topic of chat
		chatUser(user);//Open chat window
	} else if ( command == "sendfile" ) {
		openFileTransfer(user);//Open file transfer dialog
	} else if ( command == "voicemail" ) {
		//TODO: Send voicemail
		KMessageBox::error(0L, i18n("Send voicemail from SkypeActionHandler is not supported yet"), i18n("Skype protocol"));
	} else if ( command == "userinfo" ) {//TODO: Open option dialog (with all thisa options instead userinfo) and support unknown contacts who arent in contact list
		userInfo(user);
	} else {
		kDebug(SKYPE_DEBUG_GLOBAL) << "Unknown action command from SkypeActionHandler:" << command;
		KMessageBox::error(0L, i18n("Unknown action from SkypeActionHandler"), i18n("Skype protocol"));
		return;//Unknow command
	}
}

#include "skypeaccount.moc"
