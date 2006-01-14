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

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qpixmap.h>
#include <qmovie.h>
#include <kaction.h>
#include <kpopupmenu.h>

#include "kopetecontact.h"
#include "kopetecontactproperty.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "addcontactpage.h"

#define JABBER_DEBUG_GLOBAL		14130
#define JABBER_DEBUG_PROTOCOL	14131

namespace XMPP
{
	class Resource;
	class Status;
}

class JabberContact;
class dlgJabberStatus;
class dlgJabberSendRaw;
class JabberCapabilitiesManager;

class JabberProtocol:public Kopete::Protocol
{
	Q_OBJECT

public:
	/**
	 * Object constructor and destructor
	 */
	 JabberProtocol (QObject * parent, const char *name, const QStringList &);
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

	const Kopete::ContactPropertyTmpl propLastSeen;
	const Kopete::ContactPropertyTmpl propAwayMessage;
	const Kopete::ContactPropertyTmpl propFirstName;
	const Kopete::ContactPropertyTmpl propLastName;
	const Kopete::ContactPropertyTmpl propFullName;
	const Kopete::ContactPropertyTmpl propEmailAddress;
	const Kopete::ContactPropertyTmpl propPrivatePhone;
	const Kopete::ContactPropertyTmpl propPrivateMobilePhone;
	const Kopete::ContactPropertyTmpl propWorkPhone;
	const Kopete::ContactPropertyTmpl propWorkMobilePhone;
	const Kopete::ContactPropertyTmpl propNickName;
	const Kopete::ContactPropertyTmpl propSubscriptionStatus;
	const Kopete::ContactPropertyTmpl propAuthorizationStatus;
	const Kopete::ContactPropertyTmpl propAvailableResources;
	const Kopete::ContactPropertyTmpl propVCardCacheTimeStamp;
	const Kopete::ContactPropertyTmpl propPhoto;
	// extra properties to match with vCard
	const Kopete::ContactPropertyTmpl propJid;
	const Kopete::ContactPropertyTmpl propBirthday;
	const Kopete::ContactPropertyTmpl propTimezone;
	const Kopete::ContactPropertyTmpl propHomepage;
	const Kopete::ContactPropertyTmpl propCompanyName;
	const Kopete::ContactPropertyTmpl propCompanyDepartement;
	const Kopete::ContactPropertyTmpl propCompanyPosition;
	const Kopete::ContactPropertyTmpl propCompanyRole;
	const Kopete::ContactPropertyTmpl propWorkStreet;
	const Kopete::ContactPropertyTmpl propWorkExtAddr;
	const Kopete::ContactPropertyTmpl propWorkPOBox;
	const Kopete::ContactPropertyTmpl propWorkCity;
	const Kopete::ContactPropertyTmpl propWorkPostalCode;
	const Kopete::ContactPropertyTmpl propWorkCountry;
	const Kopete::ContactPropertyTmpl propWorkEmailAddress;
	const Kopete::ContactPropertyTmpl propHomeStreet;
	const Kopete::ContactPropertyTmpl propHomeExtAddr;
	const Kopete::ContactPropertyTmpl propHomePOBox;
	const Kopete::ContactPropertyTmpl propHomeCity;
	const Kopete::ContactPropertyTmpl propHomePostalCode;
	const Kopete::ContactPropertyTmpl propHomeCountry;
	const Kopete::ContactPropertyTmpl propPhoneFax;
	const Kopete::ContactPropertyTmpl propAbout;

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
