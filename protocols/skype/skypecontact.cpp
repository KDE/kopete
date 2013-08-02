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
#include "skypecontact.h"
#include "skypeaccount.h"
#include "skypeprotocol.h"
#include "skypechatsession.h"
#include "skypedetails.h"

#include <kdebug.h>
#include <qstring.h>
#include <kopetemessage.h>
#include <kopetechatsession.h>
#include <kopetechatsessionmanager.h>
#include <kopeteproperty.h>
#include <kopetemetacontact.h>
#include <kaction.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QDateTime>

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
		///Account that this contact belongs to
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
		///Authorization action
		KAction *authorizeAction;
		///Remove authorization action
		KAction *disAuthorAction;
		///Block user action
		KAction *blockAction;
		///The private phone
		QString privatePhone;
		///The private mobile phone
		QString privateMobile;
		///The work phone
		QString workPhone;
		///The homepage
		QString homepage;
		///The contacts sex
		QString sex;
		///Skype id
		QString id;
};

SkypeContact::SkypeContact(SkypeAccount *account, const QString &id, Kopete::MetaContact *parent, bool user)
	: Kopete::Contact(account, id, parent, QString()) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	d = new SkypeContactPrivate;//create the insides
	d->session = 0L;//no session yet
	d->account = account;//save the account for future, it will be needed
	connect(this, SIGNAL(setActionsPossible(bool)), this, SLOT(enableActions(bool)));
	account->prepareContact(this);//let the account prepare us
	d->user = user;

	d->callContactAction = new KAction( this );
	d->callContactAction->setText( i18n ("Call contact") );
	d->callContactAction->setIcon( (KIcon("voicecall") ) );
	connect(d->callContactAction, SIGNAL(triggered()), SLOT(call()));

	d->authorizeAction = new KAction( this );
	d->authorizeAction->setText( i18n ("(Re)send Authorization To") );
	d->authorizeAction->setIcon( (KIcon("mail-forward") ) );
	connect(d->authorizeAction, SIGNAL(triggered()), SLOT(authorize()));

	d->disAuthorAction = new KAction( this );
	d->disAuthorAction->setText( i18n ("Remove Authorization From") );
	d->disAuthorAction->setIcon( (KIcon("edit-delete") ) );
	connect(d->disAuthorAction, SIGNAL(triggered()), SLOT(disAuthor()));

	d->blockAction = new KAction( this );
	d->blockAction->setText( i18n("Block contact") );
	d->blockAction->setIcon( (KIcon("dialog-cancel") ) );
	connect(d->blockAction, SIGNAL(triggered()), SLOT(block()));

	statusChanged();//This one takes care of disabling/enabling this action depending on the user's status.

	connect(this, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)), this, SLOT(statusChanged()));
	if (account->canComunicate() && user)
		emit infoRequest(contactId());//retrieve information

	setOnlineStatus(account->protocol()->Offline);

	d->id = id;

	setFileCapable(true); //Enable sending files
}

SkypeContact::~SkypeContact() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	//free memory
	delete d;
}

Kopete::ChatSession *SkypeContact::manager(Kopete::Contact::CanCreateFlags CanCreate) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if ((!d->session) && (CanCreate)) {//It is not there and I can create it
		d->session = new SkypeChatSession(d->account, this);
		connect(d->session, SIGNAL(destroyed()), this, SLOT(removeChat()));//Care about loosing the session
		connect(d->session, SIGNAL(becameMultiChat(QString,SkypeChatSession*)), this, SLOT(removeChat()));//This means it no longer belongs to this user
	}

	return d->session;//and return it
}

void SkypeContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	serializedData["contactId"] = contactId();//save the ID
}

void SkypeContact::requestInfo() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (d->user)
		emit infoRequest(contactId());//just ask for the info
}

void SkypeContact::setInfo(const QString &change) {
	kDebug(SKYPE_DEBUG_GLOBAL) << "info is: " << change;

	const QString &receivedProperty = change.section(' ', 0, 0).trimmed().toUpper();//get the first part
	if (receivedProperty == "FULLNAME") {
		const QString &name = change.section(' ', 1).trimmed();//get the name
		setProperty( Kopete::Global::Properties::self()->fullName(), name );//save the name
		setNickName( name );//kopete nick name comes from contact, but skype contact has only one non custom name (=fullname), so set it also here
	} else if (receivedProperty == "DISPLAYNAME") {
		const QString &name = change.section(' ', 1).trimmed();//get the name
		setCustomName( name );//set the custom display name
	} else if (receivedProperty == "ONLINESTATUS") {//The online status eather changed or we just logged in and I asked for it
		const QString &status = change.section(' ', 1, 1).trimmed().toUpper();//get the status

		if (status == "OFFLINE") {
			d->status = osOffline;
		} else if (status == "ONLINE") {
			d->status = osOnline;
		} else if (status == "AWAY") {
			d->status = osAway;
		} else if (status == "NA") {
			d->status = osNA;
		} else if (status == "DND") {
			d->status = osDND;
		} else if (status == "SKYPEOUT") {
			d->status = osSkypeOut;
		} else if (status == "SKYPEME") {
			d->status = osSkypeMe;
		}

		resetStatus();
	} else if (receivedProperty == "BUDDYSTATUS") {
		int value = change.section(' ', 1, 1).trimmed().toInt();//get the value

		switch (value) {
			case 0:
				d->buddy = bsNotInList;
				break;
			case 1:
				d->buddy = bsNotInList;
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
		QString propValue = change.section(' ', 1).trimmed();
		if ( !propValue.isEmpty() )
		{
			if ( receivedProperty == "PHONE_HOME" ) {
				setProperty( d->account->protocol()->propPrivatePhone, change.section(' ', 1).trimmed() );
				d->privatePhone = change.section(' ', 1).trimmed();
			} else if ( receivedProperty == "PHONE_OFFICE" ) {
				setProperty( d->account->protocol()->propWorkPhone, change.section(' ', 1).trimmed() );
				d->workPhone = change.section(' ', 1).trimmed();
			} else if ( receivedProperty == "PHONE_MOBILE" ) {
				setProperty(d->account->protocol()->propPrivateMobilePhone, change.section(' ', 1).trimmed() );
				d->privateMobile = change.section(' ', 1).trimmed();
			} else if ( receivedProperty == "HOMEPAGE" ) {
				//setProperty( d->account->protocol()->propPrivateMobilePhone, change.section(' ', 1).trimmed() ); << This is odd, isn't it?
				d->homepage = change.section(' ', 1).trimmed();
			} else if (receivedProperty == "SEX") {
				if (change.section(' ', 1).trimmed().toUpper() == "MALE") {
					d->sex = i18n("Male");
				} else if (change.section(' ', 1).trimmed().toUpper() == "FEMALE") {
					d->sex = i18n("Female");
				} else
					d->sex = "";
			}
		}
	}
}

void SkypeContact::resetStatus() {
	kDebug(SKYPE_DEBUG_GLOBAL);

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
	kDebug(SKYPE_DEBUG_GLOBAL);

	const Kopete::OnlineStatus &st = d->account->myself()->onlineStatus();
	if ((st == d->account->protocol()->Offline) || (st == d->account->protocol()->Connecting))
		return false;

	switch (d->buddy) {
		case bsNotInList:
		case bsNoAuth://I do not know, weather he is online, but I will send it trough the server
			return true;
		case bsInList:
			break;//Do it by online status
	}

	switch (d->status) {
		//case osOffline://he is offline
		case osSkypeOut://This one can not get messages, it is SkypeOut contact
			return false;
		default://some kind of online
			return true;
	}
}

void SkypeContact::removeChat() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	d->session = 0L;//it exists no more or it is no longer of this contact
}

bool SkypeContact::hasChat() const {
	return d->session;//does it have a chat session?
}

void SkypeContact::receiveIm(const QString &message, const QString &chat, const QDateTime &timeStamp) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (!hasChat()) {
		manager(CanCreate);//create it
		if (!hasChat())//something failed
			return;
	}

	Kopete::Message mes(this, d->account->myself());//create the message
	mes.setDirection(Kopete::Message::Inbound);
	mes.setPlainBody(message);
	mes.setTimestamp(timeStamp);
	d->session->setChatId(chat);
	d->session->appendMessage(mes);//add it to the session
}

QList<KAction*> *SkypeContact::customContextMenuActions() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (d->account->myself() == this)
		return 0L;

	QList<KAction*> *actions = new QList<KAction*>();

	actions->append(d->callContactAction);
	actions->append(d->authorizeAction);
	actions->append(d->disAuthorAction);
	actions->append(d->blockAction);

	return actions;
}

void SkypeContact::enableActions(bool value) {
	d->callContactAction->setEnabled(value);
	d->authorizeAction->setEnabled(value);
	d->disAuthorAction->setEnabled(value);
	d->blockAction->setEnabled(value);
}

void SkypeContact::statusChanged() {
	SkypeProtocol * protocol = d->account->protocol();
	const Kopete::OnlineStatus &myStatus = (d->account->myself()) ? d->account->myself()->onlineStatus() : protocol->Offline;
	if (d->account->canAlterAuth()) {
		d->authorizeAction->setEnabled(true);
		d->disAuthorAction->setEnabled(true);
		d->blockAction->setEnabled(true);
	} else {
		d->authorizeAction->setEnabled(false);
		d->disAuthorAction->setEnabled(false);
		d->blockAction->setEnabled(false);
	}
	if (this == d->account->myself()) {
		emit setActionsPossible(false);
	} else if (myStatus != protocol->Offline && myStatus != protocol->Connecting)
		emit setActionsPossible(true);
	else
		emit setActionsPossible(false);
}

void SkypeContact::call() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	d->account->makeCall(this);
}

void SkypeContact::connectionStatus(bool connected) {
	if (connected) {
		statusChanged();
	} else
		emit setActionsPossible(false);
}

SkypeChatSession *SkypeContact::getChatSession() {
	return d->session;
}

bool SkypeContact::canCall() const {
	if (!d->account->canComunicate())
		return false;
	if (!d->callContactAction)
		return false;
	return d->callContactAction->isEnabled();
}

void SkypeContact::slotUserInfo() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	(new SkypeDetails)->setNames(contactId(), customName(), formattedName()).setPhones(d->privatePhone, d->privateMobile, d->workPhone).setHomepage(d->homepage).setAuthor(d->account->getAuthor(contactId()), d->account).setSex(d->sex).show();
}

void SkypeContact::deleteContact() {
	kDebug(SKYPE_DEBUG_GLOBAL);
	d->account->removeContact(contactId());
	deleteLater();
}

void SkypeContact::sync(unsigned int changed) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if ( ! account()->isConnected() )
		return;

	if ( changed & Kopete::Contact::MovedBetweenGroup ) {

		d->account->registerContact(contactId());
		d->account->MovedBetweenGroup(this);

	}

	if ( changed & Kopete::Contact::DisplayNameChanged ) {

		// if metacontact using display name from remote skype contact (not customized), reset skype contact display name to default
		if ( metaContact()->displayNameSource() == Kopete::MetaContact::SourceCustom && metaContact()->displayNameSourceContact() == this
			&& ( preferredNameType() == Kopete::Contact::NickName || preferredNameType() == Kopete::Contact::FormattedName ) )
			d->account->setContactDisplayName(contactId(), QString());
		else
			d->account->setContactDisplayName(contactId(), metaContact()->displayName());

	}
}

void SkypeContact::authorize() {
	kDebug(SKYPE_DEBUG_GLOBAL);
	d->account->authorizeUser(contactId());
}

void SkypeContact::disAuthor() {
	kDebug(SKYPE_DEBUG_GLOBAL);
	d->account->disAuthorUser(contactId());
}

void SkypeContact::block() {
	kDebug(SKYPE_DEBUG_GLOBAL);
	d->account->blockUser(contactId());
}

void SkypeContact::sendFile(const KUrl &url, const QString &, uint) {
	kDebug(SKYPE_DEBUG_GLOBAL);
	d->account->openFileTransfer(contactId(), url.toLocalFile());
}

#include "skypecontact.moc"
