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
#include "skypeprotocol.h"
#include "skypeeditaccount.h"
#include "skypeaccount.h"
#include "skypeaddcontact.h"
#include "skypecontact.h"

#include <kopeteonlinestatusmanager.h>
#include <kgenericfactory.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>
#include <kdebug.h>
#include <kaction.h>
#include <kshortcut.h>
#include <kopetecontactlist.h>
#include <kopetemetacontact.h>

typedef KGenericFactory<SkypeProtocol> SkypeProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_skype, SkypeProtocolFactory( "kopete_skype" )  )

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

SkypeProtocol::SkypeProtocol(QObject *parent, const char *name, const QStringList &) :
	Kopete::Protocol(SkypeProtocolFactory::instance(), parent, name),//create the parent
	Offline(Kopete::OnlineStatus::Offline, 0, this, 1, QString::null, i18n("Offline"), i18n("Offline"), Kopete::OnlineStatusManager::Offline),//and online statuses
	Online(Kopete::OnlineStatus::Online, 1, this, 2, QString::null, i18n("Online"), i18n("Online"), Kopete::OnlineStatusManager::Online),
	SkypeMe(Kopete::OnlineStatus::Online, 0, this, 3, "contact_ffc_overlay", i18n("Skype Me"), i18n("Skype Me"), Kopete::OnlineStatusManager::FreeForChat),
	Away(Kopete::OnlineStatus::Away, 2, this, 4, "contact_away_overlay", i18n("Away"), i18n("Away"), Kopete::OnlineStatusManager::Away),
	NotAvailable(Kopete::OnlineStatus::Away, 1, this, 5, "contact_xa_overlay", i18n("Not Available"), i18n("Not Available"), Kopete::OnlineStatusManager::Away),
	DoNotDisturb(Kopete::OnlineStatus::Away, 0, this, 6, "contact_busy_overlay", i18n("Do Not Disturb"), i18n("Do Not Disturb"), Kopete::OnlineStatusManager::Busy),
	Invisible(Kopete::OnlineStatus::Invisible, 0, this, 7, "contact_invisible_overlay", i18n("Invisible"), i18n("Invisible"), Kopete::OnlineStatusManager::Invisible),
	Connecting(Kopete::OnlineStatus::Connecting, 0, this, 8, "skype_connect", i18n("Connecting")),
	NotInList(Kopete::OnlineStatus::Offline, 0, this, 9, "contact_unknown_overlay", i18n("Not in skype list")),
	NoAuth(Kopete::OnlineStatus::Offline, 0, this, 10, "contact_unknown_overlay", i18n("Not authorized")),
	Phone(Kopete::OnlineStatus::Online, 0, this, 11, "contact_phone_overlay", i18n("SkypeOut contact")),
	/** Contact property templates */
	propFullName(Kopete::Global::Properties::self()->fullName()),
	propPrivatePhone(Kopete::Global::Properties::self()->privatePhone()),
	propPrivateMobilePhone(Kopete::Global::Properties::self()->privateMobilePhone()),
	propWorkPhone(Kopete::Global::Properties::self()->workPhone()),
	propLastSeen(Kopete::Global::Properties::self()->lastSeen())

{
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	//create the d pointer
	d = new SkypeProtocolPrivate();
	//add address book field
	addAddressBookField("messaging/skype", Kopete::Plugin::MakeIndexField);
	
	setXMLFile("skypeui.rc"); 

	d->callContactAction = new KAction(i18n("Call (by Skype)"), QString::fromLatin1("call"), 0, this, SLOT(callContacts()), actionCollection(), "callSkypeContact");

	updateCallActionStatus();
	connect(Kopete::ContactList::self(), SIGNAL(metaContactSelected(bool)), this, SLOT(updateCallActionStatus()));
}


SkypeProtocol::~SkypeProtocol() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	//release the memory
	delete d;
}

Kopete::Account *SkypeProtocol::createNewAccount(const QString &) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	//just create one
	return new SkypeAccount(this);
}

AddContactPage *SkypeProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	return new SkypeAddContact(this, parent, (SkypeAccount *)account, 0L);
}

KopeteEditAccountWidget *SkypeProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	return new skypeEditAccount(this, account, parent);//create the widget and return it
}

void SkypeProtocol::registerAccount(SkypeAccount *account) {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->account = account;
}

void SkypeProtocol::unregisterAccount() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	d->account = 0L;//forget everything about the account
}

bool SkypeProtocol::hasAccount() const {
	kdDebug(14311) << k_funcinfo << endl;//some debug info

	return (d->account);
}

Kopete::Contact *SkypeProtocol::deserializeContact(Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData, const QMap<QString, QString> &) {
	kdDebug(14311) << k_funcinfo << "Name: " << serializedData["contactId"] << endl;//some debug info

	QString contactID = serializedData["contactId"];//get the contact ID

	if (!d->account) {
		kdDebug(14311) << "Account does not exists, skiping contact creation" << endl;//write error for debuging
		return 0L;//create nothing
	}

	return new SkypeContact(d->account, contactID, metaContact);//create the contact
}

void SkypeProtocol::updateCallActionStatus() {
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	
	bool enab = false;
	
	//Run trough all selected contacts and find if there is any skype contact
	const QPtrList<Kopete::MetaContact> &selected = Kopete::ContactList::self()->selectedMetaContacts();
	for (QPtrList<Kopete::MetaContact>::const_iterator met = selected.begin(); met != selected.end(); ++met) {
		const QPtrList<Kopete::Contact> &metaCont = (*met)->contacts();
		for (QPtrList<Kopete::Contact>::const_iterator con = metaCont.begin(); con != metaCont.end(); ++con) {
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
	kdDebug(14311) << k_funcinfo << endl;//some debug info
	
	QString list;
	
	const QPtrList<Kopete::MetaContact> &selected = Kopete::ContactList::self()->selectedMetaContacts();
	for (QPtrList<Kopete::MetaContact>::const_iterator met = selected.begin(); met != selected.end(); ++met) {
		const QPtrList<Kopete::Contact> &metaCont = (*met)->contacts();
		for (QPtrList<Kopete::Contact>::const_iterator con = metaCont.begin(); con != metaCont.end(); ++con) {
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

#include "skypeprotocol.moc"
