/*
 oscarprotocol.h  -  Oscar Protocol Plugin

 Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
 Copyright (c) 2005 by Matt Rogers <mattr@kde.org>
 Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>

 Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */

#ifndef OSCARPROTOCOL_H
#define OSCARPROTOCOL_H

#include "kopeteprotocol.h"
#include "kopete_export.h"
#include "kopeteproperty.h"

class OscarStatusManager;

class OSCAR_EXPORT OscarProtocol : public Kopete::Protocol
{
	Q_OBJECT

public:
	OscarProtocol( const KComponentData &instance, QObject *parent, bool canAddMyself = false );
	virtual ~OscarProtocol();

	virtual Kopete::Contact *deserializeContact( Kopete::MetaContact *metaContact,
	                                             const QMap<QString, QString> &serializedData,
	                                             const QMap<QString, QString> &addressBookData );

	const Kopete::PropertyTmpl statusTitle;
	const Kopete::PropertyTmpl statusMessage;
	const Kopete::PropertyTmpl clientFeatures;
	const Kopete::PropertyTmpl buddyIconHash;
	const Kopete::PropertyTmpl contactEncoding;
	const Kopete::PropertyTmpl memberSince;
	const Kopete::PropertyTmpl client;
	const Kopete::PropertyTmpl protocolVersion;

	virtual OscarStatusManager *statusManager() const = 0;
};

#endif 
//kate: tab-width 4; indent-mode csands;
