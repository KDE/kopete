/*
    kopetemetacontact_p.h - Kopete Meta Contact Private

    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2005      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEMETACONTACT_P_H
#define KOPETEMETACONTACT_P_H

#include "kopetemetacontact.h"

#include "kopetepicture.h"

namespace Kopete {

class  MetaContact::Private
{ public:
	Private() :
		photoSource(MetaContact::SourceCustom), displayNameSource(MetaContact::SourceCustom),
		displayNameSourceContact(0L),  photoSourceContact(0L), temporary(false),
		onlineStatus(Kopete::OnlineStatus::Offline), photoSyncedWithKABC(false)
	{}

	QList<Contact *> contacts;
	~Private()
	{}

	QUuid metaContactId;
	// property sources
	PropertySource photoSource;
	PropertySource displayNameSource;

	// when source is contact
	Contact *displayNameSourceContact;
	Contact *photoSourceContact;

	// used when source is kabc
	QString kabcId;

	// used when source is custom
	QString displayName;
	KUrl photoUrl;

	QList<Group *> groups;
	QMap<QString, QMap<QString, QString> > addressBook;
	bool temporary;

	OnlineStatus::StatusType onlineStatus;
	bool photoSyncedWithKABC;
	OnlineStatus notifyOnlineStatus;

	// Used to set contact source at load.
	QString nameSourcePID, nameSourceAID, nameSourceCID;
	QString photoSourcePID, photoSourceAID, photoSourceCID;

	// The photo cache. Reduce disk access and CPU usage.
	Picture customPicture, contactPicture, kabcPicture;

	Kopete::StatusMessage statusMessage;
};

} //END namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:
