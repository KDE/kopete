/*
  oscarprotocol.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>

  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#include "icqprotocol.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>

#include "oscarpreferences.h"

K_EXPORT_COMPONENT_FACTORY( kopete_icq, KGenericFactory<ICQProtocol> );

ICQProtocol* ICQProtocol::protocolStatic_ = 0L;

ICQProtocol::ICQProtocol(QObject *parent, const char *name, const QStringList &l)
	: OscarProtocol(parent,name,l)
{
	if (protocolStatic_)
		kdDebug(14150) << k_funcinfo << "Oscar plugin already initialized" << endl;
	else
	{
		protocolStatic_ = this;
		// Create the config widget, this does it's magic I think
		new OscarPreferences("icq_protocol", this);
	}
}

// Called when we want to return the active instance of the protocol
ICQProtocol *ICQProtocol::protocol(void)
{
	return protocolStatic_;
}


#include "icqprotocol.moc"
// vim: set noet ts=4 sts=4 sw=4:
