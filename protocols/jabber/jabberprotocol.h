 /*
  * jabberprotocol.h  -  Base class for the Kopete Jabber protocol
  *
  * Copyright (c) 2002-2003 by Till Gerken <till@tantalo.net>
  * Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
  *
  * Kopete    (c) by the Kopete developers  <kopete-devel@kde.org>
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

#ifndef JABBERPROTOCOL_H
#define JABBERPROTOCOL_H


#include <QMap>

#include "kopeteprotocol.h"
#include "kopetemimetypehandler.h"
#include "kopeteproperty.h"
#include "kopeteonlinestatus.h"

class AddContactPage;
namespace Kopete {
	class Contact;
	class MetaContact;
}

#define JABBER_DEBUG_GLOBAL		14130
#define JABBER_DEBUG_PROTOCOL	14131

namespace XMPP
{
	class Resource;
	class Status;
}

class JabberCapabilitiesManager;

class JabberProtocol:public Kopete::Protocol , private Kopete::MimeTypeHandler
{
	Q_OBJECT

public:
	/**
	 * Object constructor and destructor
	 */
	 JabberProtocol (QObject * parent, const QVariantList &);
	 ~JabberProtocol ();

	/**
	 * Creates the "add contact" dialog specific to this protocol
	 */
	virtual AddContactPage *createAddContactWidget (QWidget * parent, Kopete::Account * i);
	virtual KopeteEditAccountWidget *createEditAccountWidget (Kopete::Account * account, QWidget * parent);
	virtual Kopete::Account *createNewAccount (const QString & accountId);

	/**
	 * Deserialize contact data
	 */
	virtual Kopete::Contact *deserializeContact (Kopete::MetaContact * metaContact,
									 const QMap < QString, QString > &serializedData, const QMap < QString, QString > &addressBookData);

	enum OnlineStatus { JabberOnline, JabberFreeForChat, JabberAway, JabberXA, JabberDND,
						JabberOffline, JabberInvisible, JabberConnecting };

	const Kopete::OnlineStatus JabberKOSChatty;
	const Kopete::OnlineStatus JabberKOSOnline;
	const Kopete::OnlineStatus JabberKOSAway;
	const Kopete::OnlineStatus JabberKOSXA;
	const Kopete::OnlineStatus JabberKOSDND;
	const Kopete::OnlineStatus JabberKOSOffline;
	const Kopete::OnlineStatus JabberKOSInvisible;
	const Kopete::OnlineStatus JabberKOSConnecting;

	const Kopete::PropertyTmpl propLastSeen;
	const Kopete::PropertyTmpl propFirstName;
	const Kopete::PropertyTmpl propLastName;
	const Kopete::PropertyTmpl propFullName;
	const Kopete::PropertyTmpl propEmailAddress;
	const Kopete::PropertyTmpl propPrivatePhone;
	const Kopete::PropertyTmpl propPrivateMobilePhone;
	const Kopete::PropertyTmpl propWorkPhone;
	const Kopete::PropertyTmpl propWorkMobilePhone;
	const Kopete::PropertyTmpl propNickName;
	const Kopete::PropertyTmpl propSubscriptionStatus;
	const Kopete::PropertyTmpl propAuthorizationStatus;
	const Kopete::PropertyTmpl propAvailableResources;
	const Kopete::PropertyTmpl propVCardCacheTimeStamp;
	const Kopete::PropertyTmpl propPhoto;
	// extra properties to match with vCard
	const Kopete::PropertyTmpl propJid;
	const Kopete::PropertyTmpl propBirthday;
	const Kopete::PropertyTmpl propTimezone;
	const Kopete::PropertyTmpl propHomepage;
	const Kopete::PropertyTmpl propCompanyName;
	const Kopete::PropertyTmpl propCompanyDepartement;
	const Kopete::PropertyTmpl propCompanyPosition;
	const Kopete::PropertyTmpl propCompanyRole;
	const Kopete::PropertyTmpl propWorkStreet;
	const Kopete::PropertyTmpl propWorkExtAddr;
	const Kopete::PropertyTmpl propWorkPOBox;
	const Kopete::PropertyTmpl propWorkCity;
	const Kopete::PropertyTmpl propWorkPostalCode;
	const Kopete::PropertyTmpl propWorkCountry;
	const Kopete::PropertyTmpl propWorkEmailAddress;
	const Kopete::PropertyTmpl propHomeStreet;
	const Kopete::PropertyTmpl propHomeExtAddr;
	const Kopete::PropertyTmpl propHomePOBox;
	const Kopete::PropertyTmpl propHomeCity;
	const Kopete::PropertyTmpl propHomePostalCode;
	const Kopete::PropertyTmpl propHomeCountry;
	const Kopete::PropertyTmpl propPhoneFax;
	const Kopete::PropertyTmpl propAbout;

	/**
	 * This returns our protocol instance
	 */
	static JabberProtocol *protocol ();

	/**
	 * Return whether the protocol supports offline messages.
	 */
	bool canSendOffline() const { return true; }

	/**
	 * Convert an XMPP::Resource status to a Kopete::OnlineStatus
	 */
	Kopete::OnlineStatus resourceToKOS ( const XMPP::Resource &resource );
	
	/**
	 * Convert an online status to a  XMPP::Status
	 */
	XMPP::Status kosToStatus( const Kopete::OnlineStatus & status, const QString& message=QString() );

	/**
	 * Return the Entity Capabilities(JEP-0115) manager instance.
	 */
	JabberCapabilitiesManager *capabilitiesManager();
	
	/**
	 * inherited from Kopete::MimeTypeHandler
	 */
	virtual void handleURL(const QString&, const KUrl & kurl) const;
	using Kopete::MimeTypeHandler::handleURL;

private:
	/*
	 * Singleton instance of our protocol class
	 */
	static JabberProtocol *protocolInstance;

	/**
	 * Unique Instance of the Entity Capabilities(JEP-0115) manager for Kopete Jabber plugin.
	 */
	JabberCapabilitiesManager *capsManager;
	
};

#endif
