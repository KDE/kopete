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

class JabberContact;
class dlgJabberStatus;
class dlgJabberSendRaw;

class JabberProtocol:public KopeteProtocol
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
	virtual AddContactPage *createAddContactWidget (QWidget * parent, KopeteAccount * i);
	virtual KopeteEditAccountWidget *createEditAccountWidget (KopeteAccount * account, QWidget * parent);
	virtual KopeteAccount *createNewAccount (const QString & accountId);

	/**
	 * Deserialize contact data
	 */
	virtual KopeteContact *deserializeContact (KopeteMetaContact * metaContact,
									 const QMap < QString, QString > &serializedData, const QMap < QString, QString > &addressBookData);

	enum OnlineStatus { JabberOnline, JabberChatty, JabberAway, JabberXA, JabberDND,
						JabberOffline, JabberInvisible, JabberConnecting };

	const KopeteOnlineStatus JabberKOSChatty;
	const KopeteOnlineStatus JabberKOSOnline;
	const KopeteOnlineStatus JabberKOSAway;
	const KopeteOnlineStatus JabberKOSXA;
	const KopeteOnlineStatus JabberKOSDND;
	const KopeteOnlineStatus JabberKOSOffline;
	const KopeteOnlineStatus JabberKOSInvisible;
	const KopeteOnlineStatus JabberKOSConnecting;

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
	const Kopete::ContactPropertyTmpl propVCardCacheTimeStamp;

	/**
	 * This returns our protocol instance
	 */
	static JabberProtocol *protocol ();

	/**
	 * Return whether the protocol supports offline messages.
	 */
	bool canSendOffline() const { return true; }

private:
	/*
	 * Singleton instance of our protocol class
	 */
	static JabberProtocol *protocolInstance;

};

#endif
