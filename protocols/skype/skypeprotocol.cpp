/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>
    Copyright (C) 2008 Pali Rohár <pali.rohar@gmail.com>

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
#include "skypeprotocol.h"
#include "skypeeditaccount.h"
#include "skypeaccount.h"
#include "skypeaddcontact.h"
#include "skypecontact.h"

#include <kopeteonlinestatusmanager.h>
#include <kgenericfactory.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kdebug.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kshortcut.h>
#include <kopetecontactlist.h>
#include <kopetemetacontact.h>

K_PLUGIN_FACTORY( SkypeProtocolFactory, registerPlugin<SkypeProtocol>(); )
K_EXPORT_PLUGIN( SkypeProtocolFactory( "kopete_skype" ) )

SkypeProtocol *SkypeProtocol::s_protocol = 0L;

class SkypeProtocolPrivate {
	private:
	public:
		///The "call contact" action
		KAction *callContactAction;
		///Pointer to the account
		SkypeAccount *account;
		///Constructor
		SkypeProtocolPrivate() {
			account = 0L;//no account yet
			callContactAction = 0L;
		}
};

SkypeProtocol::SkypeProtocol(QObject *parent, const QList<QVariant>&) :
	Kopete::Protocol(SkypeProtocolFactory::componentData(), parent),//create the parent
	Offline(Kopete::OnlineStatus::Offline, 0, this, 1, QStringList("skype_contact_offline"), i18n("Offline"), i18n("Offline"), Kopete::OnlineStatusManager::Offline),//and online statuses
	Online(Kopete::OnlineStatus::Online, 1, this, 2, QStringList("skype_contact_online"), i18n("Online"), i18n("Online"), Kopete::OnlineStatusManager::Online),
	SkypeMe(Kopete::OnlineStatus::Online, 0, this, 3, QStringList("skype_contact_skypeme"), i18n("Skype Me"), i18n("Skype Me"), Kopete::OnlineStatusManager::FreeForChat),
	Away(Kopete::OnlineStatus::Away, 2, this, 4, QStringList("skype_contact_away"), i18n("Away"), i18n("Away"), Kopete::OnlineStatusManager::Away),
	NotAvailable(Kopete::OnlineStatus::Away, 1, this, 5, QStringList("skype_contact_not_available"), i18n("Not Available"), i18n("Not Available"), Kopete::OnlineStatusManager::Away),
	DoNotDisturb(Kopete::OnlineStatus::Away, 0, this, 6, QStringList("skype_contact_dnd"), i18n("Do Not Disturb"), i18n("Do Not Disturb"), Kopete::OnlineStatusManager::Busy),
	Invisible(Kopete::OnlineStatus::Invisible, 0, this, 7, QStringList("skype_contact_invisible"), i18n("Invisible"), i18n("Invisible"), Kopete::OnlineStatusManager::Invisible),
	Connecting(Kopete::OnlineStatus::Connecting, 0, this, 8, QStringList("skype_connecting"), i18n("Connecting")),
	NotInList(Kopete::OnlineStatus::Offline, 0, this, 9, QStringList("skype_contact_unknown"), i18n("Not in Skype list")),
	NoAuth(Kopete::OnlineStatus::Offline, 0, this, 10, QStringList("skype_contact_unknown"), i18n("Not authorized")),
	Phone(Kopete::OnlineStatus::Online, 0, this, 11, QStringList("skype_contact_skypeout"), i18n("SkypeOut contact")),
	/** Contact property templates */
	propFullName(Kopete::Global::Properties::self()->fullName()),
	propPrivatePhone(Kopete::Global::Properties::self()->privatePhone()),
	propPrivateMobilePhone(Kopete::Global::Properties::self()->privateMobilePhone()),
	propWorkPhone(Kopete::Global::Properties::self()->workPhone()),
	propLastSeen(Kopete::Global::Properties::self()->lastSeen())

{
	kDebug() << k_funcinfo << endl;//some debug info
	//create the d pointer
	d = new SkypeProtocolPrivate();
	//add address book field
	addAddressBookField("messaging/skype", Kopete::Plugin::MakeIndexField);

	setXMLFile("skypeui.rc");

	d->callContactAction = new KAction( this );
	d->callContactAction->setIcon( (KIcon("skype_call") ) );
	d->callContactAction->setText( i18n ("Call") );
	connect(d->callContactAction, SIGNAL(triggered(bool)), SLOT(callContacts()));

	actionCollection()->addAction("callSkypeContact", d->callContactAction);

	updateCallActionStatus();
	connect(Kopete::ContactList::self(), SIGNAL(metaContactSelected(bool)), this, SLOT(updateCallActionStatus()));

	s_protocol = this;
}

SkypeProtocol::~SkypeProtocol() {
	kDebug() << k_funcinfo << endl;//some debug info
	//release the memory
	delete d;
}

Kopete::Account *SkypeProtocol::createNewAccount(const QString & accountID) {
	kDebug() << k_funcinfo << endl;//some debug info
	//just create one
	return new SkypeAccount(this, accountID);
}

AddContactPage *SkypeProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account) {
	kDebug() << k_funcinfo << endl;//some debug info
	return new SkypeAddContact(this, parent, (SkypeAccount *)account, 0L);
}

KopeteEditAccountWidget *SkypeProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent) {
	kDebug() << k_funcinfo << endl;//some debug info
	return new skypeEditAccount(this, account, parent);//create the widget and return it
}

void SkypeProtocol::registerAccount(SkypeAccount *account) {
	kDebug() << k_funcinfo << endl;//some debug info

	d->account = account;
}

void SkypeProtocol::unregisterAccount() {
	kDebug() << k_funcinfo << endl;//some debug info

	d->account = 0L;//forget everything about the account
}

bool SkypeProtocol::hasAccount() const {
	kDebug() << k_funcinfo << endl;//some debug info

	return (d->account);
}

Kopete::Contact *SkypeProtocol::deserializeContact(Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData, const QMap<QString, QString> &) {
	kDebug() << k_funcinfo << "Name: " << serializedData["contactId"] << endl;//some debug info

	QString contactID = serializedData["contactId"];//get the contact ID
	QString accountId = serializedData["accountId"];

	if (!d->account) {
		kDebug() << "Account does not exists, skiping contact creation" << endl;//write error for debugging
		return 0L;//create nothing
	}

	if (d->account->contact(contactID)){
		kDebug() << "Contact" << contactID << "exists in contact list, skipping contact creation" << endl;
		return 0L;
	}

	return new SkypeContact(d->account, contactID, metaContact);//create the contact
}

void SkypeProtocol::updateCallActionStatus() {
	kDebug() << k_funcinfo << endl;//some debug info

	bool enab = false;

	if ((Kopete::ContactList::self()->selectedMetaContacts().count() != 1) && ((!d->account) || (!d->account->ableMultiCall()))) {
		d->callContactAction->setEnabled(false);
		return;
	}

	//Run trough all selected contacts and find if there is any skype contact
	const QList<Kopete::MetaContact*> &selected = Kopete::ContactList::self()->selectedMetaContacts();
	for (QList<Kopete::MetaContact*>::const_iterator met = selected.begin(); met != selected.end(); ++met) {
		const QList<Kopete::Contact*> &metaCont = (*met)->contacts();
		for (QList<Kopete::Contact*>::const_iterator con = metaCont.begin(); con != metaCont.end(); ++con) {
			if ((*con)->protocol() == this) {//This is skype contact, ask it if it can be called
				SkypeContact *thisCont = static_cast<SkypeContact *> (*con);
				if (thisCont->canCall()) {
					enab = true;
					goto OUTSIDE;
				}
			}
		}
	}
	OUTSIDE:
	d->callContactAction->setEnabled(enab);
}

void SkypeProtocol::callContacts() {
	kDebug() << k_funcinfo << endl;//some debug info

	QString list;

	const QList<Kopete::MetaContact*> &selected = Kopete::ContactList::self()->selectedMetaContacts();
	for (QList<Kopete::MetaContact*>::const_iterator met = selected.begin(); met != selected.end(); ++met) {
		const QList<Kopete::Contact*> &metaCont = (*met)->contacts();
		for (QList<Kopete::Contact*>::const_iterator con = metaCont.begin(); con != metaCont.end(); ++con) {
			if ((*con)->protocol() == this) {//This is skype contact, ask it if it can be called
				SkypeContact *thisCont = static_cast<SkypeContact *> (*con);
				if (thisCont->canCall()) {
					if (!list.isEmpty())
						list += ", ";
					list += thisCont->contactId();
				}
			}
		}
	}

	if (!list.isEmpty()) {
		d->account->makeCall(list);
	}
}

SkypeProtocol *SkypeProtocol::protocol()
{
	return s_protocol;
}

#include "skypeprotocol.moc"
