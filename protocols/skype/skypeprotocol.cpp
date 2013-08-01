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

SkypeProtocolHandler::SkypeProtocolHandler() : Kopete::MimeTypeHandler(false) {
	registerAsMimeHandler("x-skype"); //This is still not enough for javascript web buttons checker, we still need skypebuttons netscape plugin
	registerAsProtocolHandler("callto");
	registerAsProtocolHandler("skype");
	registerAsProtocolHandler("tell");
}

void SkypeProtocolHandler::handleURL(const QString &, const KUrl &url) const {
	kDebug(SKYPE_DEBUG_GLOBAL);
	if ( ! SkypeProtocol::protocol()->hasAccount() ) {
		kDebug(SKYPE_DEBUG_GLOBAL) << "No Skype account registered";
		return;
	}
	SkypeProtocol::protocol()->account()->SkypeActionHandler(url.url());
}

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
		///Url handler
		SkypeProtocolHandler *handler;
};

SkypeProtocol::SkypeProtocol(QObject *parent, const QList<QVariant>&) :
	Kopete::Protocol(SkypeProtocolFactory::componentData(), parent),//create the parent
	Offline(Kopete::OnlineStatus::Offline, 0, this, 1, QStringList(), i18n("Offline"), i18n("Offline"), Kopete::OnlineStatusManager::Offline, Kopete::OnlineStatusManager::HasStatusMessage),//and online statuses
	Online(Kopete::OnlineStatus::Online, 0, this, 2, QStringList(), i18n("Online"), i18n("Online"), Kopete::OnlineStatusManager::Online, Kopete::OnlineStatusManager::HasStatusMessage),
	SkypeMe(Kopete::OnlineStatus::Online, 1, this, 3, QStringList("contact_freeforchat_overlay"), i18n("Skype Me"), i18n("Skype Me"), Kopete::OnlineStatusManager::FreeForChat, Kopete::OnlineStatusManager::HasStatusMessage),
	Away(Kopete::OnlineStatus::Away, 1, this, 4, QStringList("contact_away_overlay"), i18n("Away"), i18n("Away"), Kopete::OnlineStatusManager::Away, Kopete::OnlineStatusManager::HasStatusMessage),
	NotAvailable(Kopete::OnlineStatus::Away, 0, this, 5, QStringList("contact_xa_overlay"), i18n("Not Available"), i18n("Not Available"), Kopete::OnlineStatusManager::ExtendedAway, Kopete::OnlineStatusManager::HasStatusMessage),
	DoNotDisturb(Kopete::OnlineStatus::Busy, 0, this, 6, QStringList("contact_busy_overlay"), i18n("Do Not Disturb"), i18n("Do Not Disturb"), Kopete::OnlineStatusManager::Busy, Kopete::OnlineStatusManager::HasStatusMessage),
	Invisible(Kopete::OnlineStatus::Invisible, 0, this, 7, QStringList("contact_invisible_overlay"), i18n("Invisible"), i18n("Invisible"), Kopete::OnlineStatusManager::Invisible, Kopete::OnlineStatusManager::HasStatusMessage),
	Connecting(Kopete::OnlineStatus::Connecting, 0, this, 8, QStringList(), i18n("Connecting")),
	NotInList(Kopete::OnlineStatus::Unknown, 0, this, 9, QStringList("status_unknown_overlay"), i18n("Not in Skype list")),
	NoAuth(Kopete::OnlineStatus::Unknown, 1, this, 10, QStringList("status_unknown_overlay"), i18n("Not authorized")),
	Phone(Kopete::OnlineStatus::Unknown, 2, this, 11, QStringList("contact_phone_overlay"), i18n("SkypeOut contact")), //Skype Out contact permanently offline TODO: add option for changing
	/** Contact property templates */
	propFullName(Kopete::Global::Properties::self()->fullName()),
	propPrivatePhone(Kopete::Global::Properties::self()->privatePhone()),
	propPrivateMobilePhone(Kopete::Global::Properties::self()->privateMobilePhone()),
	propWorkPhone(Kopete::Global::Properties::self()->workPhone()),
	propLastSeen(Kopete::Global::Properties::self()->lastSeen())

{
	kDebug(SKYPE_DEBUG_GLOBAL);
	//create the d pointer
	d = new SkypeProtocolPrivate();
	//add address book field
	addAddressBookField("messaging/skype", Kopete::Plugin::MakeIndexField);

	setXMLFile("skypeui.rc");

	d->callContactAction = new KAction( this );
	d->callContactAction->setIcon( (KIcon("voicecall") ) );
	d->callContactAction->setText( i18n ("Call") );
	connect(d->callContactAction, SIGNAL(triggered(bool)), SLOT(callContacts()));

	actionCollection()->addAction("callSkypeContact", d->callContactAction);

	d->handler = new SkypeProtocolHandler;

	updateCallActionStatus();
	connect(Kopete::ContactList::self(), SIGNAL(metaContactSelected(bool)), this, SLOT(updateCallActionStatus()));

	s_protocol = this;
}

SkypeProtocol::~SkypeProtocol() {
	kDebug(SKYPE_DEBUG_GLOBAL);
	//release the memory
	delete d->handler;
	delete d;
	d = 0L;
}

Kopete::Account *SkypeProtocol::createNewAccount(const QString & accountID) {
	kDebug(SKYPE_DEBUG_GLOBAL);
	//just create one
	return new SkypeAccount(this, accountID);
}

AddContactPage *SkypeProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account) {
	kDebug(SKYPE_DEBUG_GLOBAL);
	return new SkypeAddContact(this, parent, (SkypeAccount *)account, 0L);
}

KopeteEditAccountWidget *SkypeProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent) {
	kDebug(SKYPE_DEBUG_GLOBAL);
	return new skypeEditAccount(this, account, parent);//create the widget and return it
}

void SkypeProtocol::registerAccount(SkypeAccount *account) {
	kDebug(SKYPE_DEBUG_GLOBAL);

	d->account = account;
}

void SkypeProtocol::unregisterAccount() {
	kDebug(SKYPE_DEBUG_GLOBAL);

	if (d)
		d->account = 0L;//forget everything about the account
}

bool SkypeProtocol::hasAccount() const {
	kDebug(SKYPE_DEBUG_GLOBAL);

	return (d->account);
}

SkypeAccount * SkypeProtocol::account() {
	kDebug(SKYPE_DEBUG_GLOBAL);
	return d->account;
}

Kopete::Contact *SkypeProtocol::deserializeContact(Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData, const QMap<QString, QString> &) {
	kDebug(SKYPE_DEBUG_GLOBAL) << "Name: " << serializedData["contactId"];

	QString contactID = serializedData["contactId"];//get the contact ID
	QString accountId = serializedData["accountId"];
	Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ "preferredNameType" ]);

	if (!d->account) {
		kDebug(SKYPE_DEBUG_GLOBAL) << "Account does not exists, skiping contact creation";//write error for debugging
		return 0L;//create nothing
	}

	if (d->account->contact(contactID)) {
		kDebug(SKYPE_DEBUG_GLOBAL) << "Contact" << contactID << "exists in contact list, skipping contact creation";
		return 0L;
	}

	SkypeContact *contact = new SkypeContact(d->account, contactID, metaContact);//create the contact
	contact->setPreferredNameType(nameType);
	return contact;
}

void SkypeProtocol::updateCallActionStatus() {
	kDebug(SKYPE_DEBUG_GLOBAL);

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
	kDebug(SKYPE_DEBUG_GLOBAL);

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
