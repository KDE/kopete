 /*
  *   jabberprotocol.cpp  -  Base class for the Kopete Jabber protocol
  *
  * Copyright (c) 2002-2003 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
  * Copyright (c) 2006      by Olivier Goffart  <ogoffart at kde.org>
  *
  *  Kopete   (c) by the Kopete developers  <kopete-devel@kde.org>
  *
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <klineeditdlg.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qmap.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <qstringlist.h>

#include "im.h"
#include "xmpp.h"

#include <sys/utsname.h>

#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetechatsession.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteaway.h"
#include "kopeteglobal.h"
#include "kopeteprotocol.h"
#include "kopeteplugin.h"
#include "kopeteaccountmanager.h"
#include "addcontactpage.h"
#include "kopetecommandhandler.h"

#include "jabbercontact.h"
#include "jabberaddcontactpage.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "jabbereditaccountwidget.h"
#include "jabbercapabilitiesmanager.h"
#include "jabbertransport.h"
#include "dlgjabbersendraw.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"
#include "dlgjabberregister.h"

JabberProtocol *JabberProtocol::protocolInstance = 0;

typedef KGenericFactory<JabberProtocol> JabberProtocolFactory;

K_EXPORT_COMPONENT_FACTORY( kopete_jabber, JabberProtocolFactory( "kopete_jabber" )  )

JabberProtocol::JabberProtocol (QObject * parent, const char *name, const QStringList &)
: Kopete::Protocol( JabberProtocolFactory::instance(), parent, name ),
	JabberKOSChatty(Kopete::OnlineStatus::Online,        100, this, JabberFreeForChat, "jabber_chatty", i18n ("Free for Chat"), i18n ("Free for Chat"), Kopete::OnlineStatusManager::FreeForChat, Kopete::OnlineStatusManager::HasAwayMessage ),
	JabberKOSOnline(Kopete::OnlineStatus::Online,         90, this, JabberOnline, QString::null, i18n ("Online"), i18n ("Online"), Kopete::OnlineStatusManager::Online, Kopete::OnlineStatusManager::HasAwayMessage ),
	JabberKOSAway(Kopete::OnlineStatus::Away,             80, this, JabberAway, "contact_away_overlay", i18n ("Away"), i18n ("Away"), Kopete::OnlineStatusManager::Away, Kopete::OnlineStatusManager::HasAwayMessage),
	JabberKOSXA(Kopete::OnlineStatus::Away,               70, this, JabberXA, "contact_xa_overlay", i18n ("Extended Away"), i18n ("Extended Away"), 0, Kopete::OnlineStatusManager::HasAwayMessage),
	JabberKOSDND(Kopete::OnlineStatus::Away,              60, this, JabberDND, "contact_busy_overlay", i18n ("Do not Disturb"), i18n ("Do not Disturb"), Kopete::OnlineStatusManager::Busy, Kopete::OnlineStatusManager::HasAwayMessage),
	JabberKOSOffline(Kopete::OnlineStatus::Offline,       50, this, JabberOffline, QString::null, i18n ("Offline") ,i18n ("Offline"), Kopete::OnlineStatusManager::Offline, Kopete::OnlineStatusManager::HasAwayMessage ),
	JabberKOSInvisible(Kopete::OnlineStatus::Invisible,   40, this, JabberInvisible, "contact_invisible_overlay",   i18n ("Invisible") ,i18n ("Invisible"), Kopete::OnlineStatusManager::Invisible),
	JabberKOSConnecting(Kopete::OnlineStatus::Connecting, 30, this, JabberConnecting, "jabber_connecting",  i18n("Connecting")),
	propLastSeen(Kopete::Global::Properties::self()->lastSeen()),
	propAwayMessage(Kopete::Global::Properties::self()->awayMessage()),
	propFirstName(Kopete::Global::Properties::self()->firstName()),
	propLastName(Kopete::Global::Properties::self()->lastName()),
	propFullName(Kopete::Global::Properties::self()->fullName()),
	propEmailAddress(Kopete::Global::Properties::self()->emailAddress()),
	propPrivatePhone(Kopete::Global::Properties::self()->privatePhone()),
	propPrivateMobilePhone(Kopete::Global::Properties::self()->privateMobilePhone()),
	propWorkPhone(Kopete::Global::Properties::self()->workPhone()),
	propWorkMobilePhone(Kopete::Global::Properties::self()->workMobilePhone()),
	propNickName(Kopete::Global::Properties::self()->nickName()),
	propSubscriptionStatus("jabberSubscriptionStatus", i18n ("Subscription"), QString::null, true, false),
	propAuthorizationStatus("jabberAuthorizationStatus", i18n ("Authorization Status"), QString::null, true, false),
	propAvailableResources("jabberAvailableResources", i18n ("Available Resources"), "jabber_chatty", false, true),
	propVCardCacheTimeStamp("jabberVCardCacheTimeStamp", i18n ("vCard Cache Timestamp"), QString::null, true, false, true),
	propPhoto(Kopete::Global::Properties::self()->photo()),
	propJid("jabberVCardJid", i18n("Jabber ID"), QString::null, true, false),
	propBirthday("jabberVCardBirthday", i18n("Birthday"), QString::null, true, false),
	propTimezone("jabberVCardTimezone", i18n("Timezone"), QString::null, true, false),
	propHomepage("jabberVCardHomepage", i18n("Homepage"), QString::null, true, false),
	propCompanyName("jabberVCardCompanyName", i18n("Company name"), QString::null, true, false),
	propCompanyDepartement("jabberVCardCompanyDepartement", i18n("Company Departement"), QString::null, true, false),
	propCompanyPosition("jabberVCardCompanyPosition", i18n("Company Position"), QString::null, true, false),
	propCompanyRole("jabberVCardCompanyRole", i18n("Company Role"), QString::null, true, false),
	propWorkStreet("jabberVCardWorkStreet", i18n("Work Street"), QString::null, true, false),
	propWorkExtAddr("jabberVCardWorkExtAddr", i18n("Work Extra Address"), QString::null, true, false),
	propWorkPOBox("jabberVCardWorkPOBox", i18n("Work PO Box"), QString::null, true, false),
	propWorkCity("jabberVCardWorkCity", i18n("Work City"), QString::null, true, false),
	propWorkPostalCode("jabberVCardWorkPostalCode", i18n("Work Postal Code"), QString::null, true, false),
	propWorkCountry("jabberVCardWorkCountry", i18n("Work Country"), QString::null, true, false),
	propWorkEmailAddress("jabberVCardWorkEmailAddress", i18n("Work Email Address"), QString::null, true, false),
	propHomeStreet("jabberVCardHomeStreet", i18n("Home Street"), QString::null, true, false),
	propHomeExtAddr("jabberVCardHomeExt", i18n("Home Extra Address"), QString::null, true, false),
	propHomePOBox("jabberVCardHomePOBox", i18n("Home PO Box"), QString::null, true, false),
	propHomeCity("jabberVCardHomeCity", i18n("Home City"), QString::null, true, false),
	propHomePostalCode("jabberVCardHomePostalCode", i18n("Home Postal Code"), QString::null, true, false),
	propHomeCountry("jabberVCardHomeCountry", i18n("Home Country"), QString::null, true, false),
	propPhoneFax("jabberVCardPhoneFax", i18n("Fax"), QString::null, true, false),
	propAbout("jabberVCardAbout", i18n("About"), QString::null, true, false)

{

	kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Loading ..." << endl;

	/* This is meant to be a singleton, so we will check if we have
	 * been loaded before. */
	if (protocolInstance)
	{
		kdDebug (JABBER_DEBUG_GLOBAL) << "[JabberProtocol] Warning: Protocol already " << "loaded, not initializing again." << endl;
		return;
	}

	protocolInstance = this;

	addAddressBookField ("messaging/xmpp", Kopete::Plugin::MakeIndexField);
	setCapabilities(Kopete::Protocol::FullRTF|Kopete::Protocol::CanSendOffline);

	// Init the Entity Capabilities manager.
	capsManager = new JabberCapabilitiesManager;
	capsManager->loadCachedInformation();
}

JabberProtocol::~JabberProtocol ()
{
	//disconnectAll();

	delete capsManager;
	capsManager = 0L;

	/* make sure that the next attempt to load Jabber
	 * re-initializes the protocol class. */
	protocolInstance = 0L;
}



AddContactPage *JabberProtocol::createAddContactWidget (QWidget * parent, Kopete::Account * i)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[Jabber Protocol] Create Add Contact  Widget\n" << endl;
	return new JabberAddContactPage (i, parent);
}

KopeteEditAccountWidget *JabberProtocol::createEditAccountWidget (Kopete::Account * account, QWidget * parent)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[Jabber Protocol] Edit Account Widget\n" << endl;
	JabberAccount *ja=dynamic_cast < JabberAccount * >(account);
	if(ja || !account)
		return new JabberEditAccountWidget (this,ja , parent);
	else
	{
		JabberTransport *transport = dynamic_cast < JabberTransport * >(account);
		if(!transport)
			return 0L;
		dlgJabberRegister *registerDialog = new dlgJabberRegister (transport->account(), transport->myself()->contactId());
		registerDialog->show (); 
		registerDialog->raise ();
		return 0l; //we make ourself our own dialog, not an editAccountWidget.
	}
}

Kopete::Account *JabberProtocol::createNewAccount (const QString & accountId)
{
	kdDebug (JABBER_DEBUG_GLOBAL) << "[Jabber Protocol] Create New Account. ID: " << accountId << "\n" << endl;
	if( Kopete::AccountManager::self()->findAccount( pluginId() , accountId ) )
		return 0L;  //the account may already exist if greated just above

	int slash=accountId.find('/');
	if(slash>=0)
	{
		QString realAccountId=accountId.left(slash);
		JabberAccount *realAccount=dynamic_cast<JabberAccount*>(Kopete::AccountManager::self()->findAccount( pluginId() , realAccountId ));
		if(!realAccount) //if it doesn't exist yet, create it
		{
			realAccount = new JabberAccount( this, realAccountId );
			if(!Kopete::AccountManager::self()->registerAccount(  realAccount ) )
				return 0L;
		}
		if(!realAccount)
			return 0L;
		return new JabberTransport( realAccount , accountId );
	}
	else
	{
		return new JabberAccount (this, accountId);
	}
}

Kopete::OnlineStatus JabberProtocol::resourceToKOS ( const XMPP::Resource &resource )
{

	// update to offline by default
	Kopete::OnlineStatus status = JabberKOSOffline;

	if ( !resource.status().isAvailable () )
	{
		// resource is offline
		status = JabberKOSOffline;
	}
	else
	{
		if (resource.status ().show ().isEmpty ())
		{
			if (resource.status ().isInvisible ())
			{
				status = JabberKOSInvisible;
			}
			else
			{
				status = JabberKOSOnline;
			}
		}
		else
		if (resource.status ().show () == "chat")
		{
			status = JabberKOSChatty;
		}
		else if (resource.status ().show () == "away")
		{
			status = JabberKOSAway;
		}
		else if (resource.status ().show () == "xa")
		{
			status = JabberKOSXA;
		}
		else if (resource.status ().show () == "dnd")
		{
			status = JabberKOSDND;
		}
		else if (resource.status ().show () == "online")
		{ // the ApaSMSAgent sms gateway report status as "online" even if it's not in the RFC 3921 § 2.2.2.1 
			// See Bug 129059
			status = JabberKOSOnline;
		}
		else if (resource.status ().show () == "connecting")
		{ // this is for kopete internals
			status = JabberKOSConnecting;
		}
		else
		{
			kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Unknown status <show>" << resource.status ().show () << "</show> for contact. One of your contact is probably using a broken client, ask him to report a bug" << endl;
		}
	}

	return status;

}

JabberCapabilitiesManager *JabberProtocol::capabilitiesManager()
{
	return capsManager;
}

JabberProtocol *JabberProtocol::protocol ()
{
	// return current instance
	return protocolInstance;
}

Kopete::Contact *JabberProtocol::deserializeContact (Kopete::MetaContact * metaContact,
										 const QMap < QString, QString > &serializedData, const QMap < QString, QString > & /* addressBookData */ )
{
//  kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Deserializing data for metacontact " << metaContact->displayName () << "\n" << endl;

	QString contactId = serializedData["contactId"];
	QString displayName = serializedData["displayName"];
	QString accountId = serializedData["accountId"];
	QString jid = serializedData["JID"];

	QDict < Kopete::Account > accounts = Kopete::AccountManager::self ()->accounts (this);
	Kopete::Account *account = accounts[accountId];
	
	if (!account)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "WARNING: Account for contact does not exist, skipping." << endl;
		return 0;
	}
	
	JabberTransport *transport = dynamic_cast<JabberTransport*>(account);
	if( transport )
		transport->account()->addContact ( jid.isEmpty() ? contactId : jid ,  metaContact);
	else
		account->addContact (contactId,  metaContact);
	return account->contacts()[contactId];
}

XMPP::Status JabberProtocol::kosToStatus( const Kopete::OnlineStatus & status , const QString & message )
{
	XMPP::Status xmppStatus ( "", message );

	if( status.status() == Kopete::OnlineStatus::Offline )
	{
		xmppStatus.setIsAvailable( false );
	}

	switch ( status.internalStatus () )
	{
		case JabberProtocol::JabberFreeForChat:
			xmppStatus.setShow ( "chat" );
			break;

		case JabberProtocol::JabberOnline:
			xmppStatus.setShow ( "" );
			break;

		case JabberProtocol::JabberAway:
			xmppStatus.setShow ( "away" );
			break;

		case JabberProtocol::JabberXA:
			xmppStatus.setShow ( "xa" );
			break;

		case JabberProtocol::JabberDND:
			xmppStatus.setShow ( "dnd" );
			break;

		case JabberProtocol::JabberInvisible:
			xmppStatus.setIsInvisible ( true );
			break;
	}
	return xmppStatus;
}

#include "jabberprotocol.moc"
