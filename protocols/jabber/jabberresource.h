 /*
  * jabberresource.h
  *
  * Copyright (c) 2004 by Till Gerken <till@tantalo.net>
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

#ifndef JABBERRESOURCE_H
#define JABBERRESOURCE_H

/**
 * Container class for a contact's resource
 */

#include <qobject.h>
#include <qstring.h>
#include <im.h>
#include "jabberprotocol.h"

class JabberAccount;

class JabberResource:public QObject
{

Q_OBJECT

public:
	JabberResource (JabberAccount *account, const XMPP::Jid &jid, const XMPP::Resource &resource);
	~JabberResource ();

	const XMPP::Jid &jid() const;
	const XMPP::Resource &resource() const;

	void setResource ( const XMPP::Resource &resource );

	const QString &clientName () const;
	const QString &clientSystem () const;

signals:
	void updated ( JabberResource * );

private slots:
	void slotGetTimedClientVersion ();
	void slotGotClientVersion ();

private:
	XMPP::Jid mJid;
	XMPP::Resource mResource;
	JabberAccount *mAccount;
	QString mClientName, mClientSystem;

};

#endif

// vim: set noet ts=4 sts=4 tw=4:
