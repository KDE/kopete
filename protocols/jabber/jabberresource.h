 /*
  * jabberresource.h
  *
  * Copyright (c) 2005-2006 by Michaël Larouche <larouche@kde.org>
  * Copyright (c) 2004 by Till Gerken <till@tantalo.net>
  *
  * Kopete    (c) 2001-2006 by the Kopete developers  <kopete-devel@kde.org>
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

class JabberAccount;

namespace XMPP
{
class Resource;
class Jid;
class Features;
}

class JabberResource : public QObject
{
Q_OBJECT

public:
	/**
	 * Create a new Jabber resource.
	 */
	JabberResource (JabberAccount *account, const XMPP::Jid &jid, const XMPP::Resource &resource);
	~JabberResource ();

	const XMPP::Jid &jid() const;
	const XMPP::Resource &resource() const;

	void setResource ( const XMPP::Resource &resource );

	/**
	 * Return the client name for this resource.
	 * @return the client name
	 */
	const QString &clientName () const;
	/**
	 * Return the client version for this resource.
	 * @return the client version
	 */
	const QString &clientVersion () const;
	/**
	 * Return the client system for this resource.
	 * @return the client system.
	 */
	const QString &clientSystem () const;

	/**
	 * Get the available features for this resource.
	 */
	XMPP::Features features() const;

signals:
	void updated ( JabberResource * );

private slots:
	void slotGetTimedClientVersion ();
	void slotGotClientVersion ();
	void slotGetDiscoCapabilties ();
	void slotGotDiscoCapabilities ();

private:
	class Private;
	Private * const d;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:
