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

#include <kdebug.h>
#include "jabberresource.h"

JabberResource::JabberResource (const XMPP::Jid &jid, const XMPP::Resource &resource)
{

	mJid = jid;
	mResource = resource;

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

#include "jabberresource.moc"
