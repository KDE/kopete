 /*
  * jabberresource.cpp
  *
  * Copyright (c) 2005 by Michaël Larouche <michael.larouche@kdemail.net>
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

// Qt includes
#include <qtimer.h>

// KDE includes
#include <kdebug.h>

// libiris includes
#include <im.h>
#include <xmpp_tasks.h>

// Kopete includes
#include "jabberprotocol.h"
#include "jabberaccount.h"

class JabberResource::Private
{
public:
	Private( JabberAccount *t_account, const XMPP::Jid &t_jid, const XMPP::Resource &t_resource )
	 : account(t_account), jid(t_jid), resource(t_resource), supportedFeatures(0)
	{}

	JabberAccount *account;
	XMPP::Jid jid;
	XMPP::Resource resource;
	
	QString clientName, clientSystem;
	int supportedFeatures;
};

JabberResource::JabberResource ( JabberAccount *account, const XMPP::Jid &jid, const XMPP::Resource &resource )
	: d( new Private(account, jid, resource) )
{
	if ( account->isConnected () )
	{
		QTimer::singleShot ( account->client()->getPenaltyTime () * 1000, this, SLOT ( slotGetTimedClientVersion () ) );
		QTimer::singleShot ( account->client()->getPenaltyTime () * 1000, this, SLOT ( slotGetDiscoCapabilties () ) );
	}

}

JabberResource::~JabberResource ()
{
	delete d;
}

const XMPP::Jid &JabberResource::jid () const
{
	return d->jid;
}

const XMPP::Resource &JabberResource::resource () const
{
	return d->resource;
}

void JabberResource::setResource ( const XMPP::Resource &resource )
{
	d->resource = resource;
}

const QString &JabberResource::clientName () const
{
	return d->clientName;
}

const QString &JabberResource::clientSystem () const
{
	return d->clientSystem;
}

bool JabberResource::canHandleXHTML()
{
	return (d->supportedFeatures & JabberProtocol::Feature_XHTML_IM);
}

int JabberResource::supportedFeatures()
{
	return d->supportedFeatures;
}

void JabberResource::slotGetTimedClientVersion ()
{
	if ( d->account->isConnected () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Requesting client version for " << d->jid.full () << endl;

		// request client version
		XMPP::JT_ClientVersion *task = new XMPP::JT_ClientVersion ( d->account->client()->rootTask () );
		// signal to ourselves when the vCard data arrived
		QObject::connect ( task, SIGNAL ( finished () ), this, SLOT ( slotGotClientVersion () ) );
		task->get ( d->jid );
		task->go ( true );
	}
}

void JabberResource::slotGotClientVersion ()
{
	XMPP::JT_ClientVersion *clientVersion = (XMPP::JT_ClientVersion *) sender ();

	if ( clientVersion->success () )
	{
		d->clientName = clientVersion->name () + " " + clientVersion->version ();
		d->clientSystem = clientVersion->os ();
	}

	emit updated ( this );

}

void JabberResource:: slotGetDiscoCapabilties ()
{
	if ( d->account->isConnected () )
	{
		kdDebug ( JABBER_DEBUG_GLOBAL ) << k_funcinfo << "Requesting Client Features for " << d->jid.full () << endl;

		// Reset supported features.
		d->supportedFeatures = 0;

		XMPP:: JT_DiscoInfo *task = new XMPP::JT_DiscoInfo ( d->account->client()->rootTask () );
		// Retrive features when service discovery is done.
		QObject::connect ( task, SIGNAL ( finished () ), this, SLOT (slotGotDiscoCapabilities () ) );
		task->get ( d->jid);
		task->go ( true );
	}
}

void JabberResource::slotGotDiscoCapabilities ()
{
	XMPP::JT_DiscoInfo *DiscoInfos = (XMPP::JT_DiscoInfo *) sender ();

	if ( DiscoInfos->success () )
	{
		//If we don't want to patch libiris, we can also retrieve a QStringList that we should parse manually
		if (DiscoInfos->item().features().canXHTML())
		{
			d->supportedFeatures |= JabberProtocol::Feature_XHTML_IM;
		}
	}
}

#include "jabberresource.moc"
