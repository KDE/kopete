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
#include <im.h>
#include "jabberprotocol.h"

class JabberResource:public QObject
{

Q_OBJECT

public:
	JabberResource (const XMPP::Jid &jid, const XMPP::Resource &resource);
	~JabberResource ();

	const XMPP::Jid &jid() const;
	const XMPP::Resource &resource() const;

	void setResource ( const XMPP::Resource &resource );

private:
	XMPP::Jid mJid;
	XMPP::Resource mResource;

};

#endif

// vim: set noet ts=4 sts=4 tw=4:
