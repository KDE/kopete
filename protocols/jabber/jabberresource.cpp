 /*
  * jabberresource.cpp
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

#include "jabberresource.h"

#include <kdebug.h>
#include "xmpp_tasks.h"
#include "jabberaccount.h"

JabberResource::JabberResource ( JabberAccount *account, const XMPP::Jid &jid, const XMPP::Resource &resource )
{

	mJid = jid;
	mResource = resource;
	mAccount = account;

	if ( account->isConnected () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Requesting client version for " << jid.full () << endl;

		// request client version
		XMPP::JT_ClientVersion *task = new XMPP::JT_ClientVersion ( account->client()->rootTask () );
		// signal to ourselves when the vCard data arrived
		QObject::connect ( task, SIGNAL ( finished () ), this, SLOT ( slotGotClientVersion () ) );
		task->get ( jid );
		task->go ( true );
	}

}

JabberResource::~JabberResource ()
{
}

const XMPP::Jid &JabberResource::jid () const
{

	return mJid;

}

const XMPP::Resource &JabberResource::resource () const
{

	return mResource;

}

void JabberResource::setResource ( const XMPP::Resource &resource )
{

	mResource = resource;

}

const QString &JabberResource::clientName () const
{

	return mClientName;

}

const QString &JabberResource::clientSystem () const
{

	return mClientSystem;

}

void JabberResource::slotGotClientVersion ()
{
	XMPP::JT_ClientVersion *clientVersion = (XMPP::JT_ClientVersion *) sender ();

	if ( clientVersion->success () )
	{
		mClientName = clientVersion->name () + " " + clientVersion->version ();
		mClientSystem = clientVersion->os ();
	}

	emit updated ( this );

}

#include "jabberresource.moc"
