 /*
  * jabberprotocol.h  -  Base class for the Kopete Jabber protocol
  *
  * Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
  * Copyright (c) 2002 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>
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

#include "client.h"
#include "types.h"
#include "jid.h"

#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "addcontactpage.h"
#include "jabbermap.h"

#define JABBER_DEBUG_GLOBAL		14130
#define JABBER_DEBUG_PROTOCOL	14131

class JabberContact;
class dlgJabberStatus;
class dlgJabberSendRaw;

using namespace Jabber;

class JabberProtocol:public KopeteProtocol
{
	Q_OBJECT

public:

	/*********************************************************************
	 *
	 * KopeteProtocol reimplementation start
	 *
	 ********************************************************************/

	/**
	 * Object constructor and destructor
	 */
	 JabberProtocol (QObject * parent, const char *name, const QStringList &);
	 ~JabberProtocol ();

	/**
	 * Creates the "add contact" dialog specific to this protocol
	 */
	virtual AddContactPage *createAddContactWidget (QWidget * parent, KopeteAccount * i);
	virtual EditAccountWidget *createEditAccountWidget (KopeteAccount * account, QWidget * parent);
	virtual KopeteAccount *createNewAccount (const QString & accountId);

	/**
	 * Deserialize contact data
	 */
	virtual void deserializeContact (KopeteMetaContact * metaContact,
									 const QMap < QString, QString > &serializedData, const QMap < QString, QString > &addressBookData);

	/*********************************************************************
	 *
	 * KopeteProtocol reimplementation end
	 *
	 ********************************************************************/

	const KopeteOnlineStatus JabberOnline;
	const KopeteOnlineStatus JabberChatty;
	const KopeteOnlineStatus JabberAway;
	const KopeteOnlineStatus JabberXA;
	const KopeteOnlineStatus JabberDND;
	const KopeteOnlineStatus JabberOffline;
	const KopeteOnlineStatus JabberInvisible;
	const KopeteOnlineStatus JabberConnecting;

	/**
	 * This returns our protocol instance
	 */
	static JabberProtocol *protocol ();

private:
	/*
	 * Singleton instance of our protocol class
	 */
	static JabberProtocol *protocolInstance;

};

#endif
