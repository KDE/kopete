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

#include "aimprotocol.h"


#include <kgenericfactory.h>
#include <klocale.h>
#include <kdebug.h>

#include "oscarpreferences.h"

K_EXPORT_COMPONENT_FACTORY( kopete_aim, KGenericFactory<AIMProtocol> );


AIMProtocol* AIMProtocol::protocolStatic_ = 0L;

AIMProtocol::AIMProtocol(QObject *parent, const char *name, const QStringList &l)
	: OscarProtocol(parent,name,l)
{
	if (protocolStatic_)
		kdDebug(14150) << k_funcinfo << "Oscar plugin already initialized" << endl;
	else
	{
		protocolStatic_ = this;
		// Create the config widget, this does it's magic I think
		new OscarPreferences("oscar_protocol", this);
	}
}

// Called when we want to return the active instance of the protocol
AIMProtocol *AIMProtocol::protocol(void)
{
	return protocolStatic_;
}

#include "aimprotocol.moc"
// vim: set noet ts=4 sts=4 sw=4:
