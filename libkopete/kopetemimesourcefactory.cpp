/*
    kopetemimesourcefactory.cpp - Kopete mime source factory

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetemimesourcefactory.h"

#include "kopeteaccountmanager.h"
#include "kopetecontactlist.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"

#include <kiconloader.h>

#include <qdragobject.h>
#include <qstring.h>
#include <qstringlist.h>

namespace Kopete
{

namespace
{
	static QDict<Kopete::MimeSourceFactory> g_mimeHandlers;
}

class MimeSourceFactory::Private
{
public:
	Private() : lastMimeSource( 0 ) {}
	~Private() { delete lastMimeSource; }
	mutable QMimeSource *lastMimeSource;
};

MimeSourceFactory::MimeSourceFactory()
 : d( new Private )
{
}

MimeSourceFactory::~MimeSourceFactory()
{
	delete d;
}

const QMimeSource *MimeSourceFactory::data( const QString &abs_name ) const
{
	// extract and decode arguments
	QStringList parts = QStringList::split( QChar(':'), abs_name );
	for ( QStringList::Iterator it = parts.begin(); it != parts.end(); ++it )
		*it = KURL::decode_string( *it );

	QPixmap img;

	if ( parts[0] == QString::fromLatin1("kopete-contact-icon") )
	{
		if ( parts.size() < 4 ) return 0;
		KopeteAccount *account = KopeteAccountManager::manager()->findAccount( parts[1], parts[2] );
		if ( !account ) return 0;
		KopeteContact *contact = account->contacts()[ parts[3] ];
		if ( !contact ) return 0;
		img = contact->onlineStatus().iconFor( contact );
	}

	if ( parts[0] == QString::fromLatin1("kopete-account-icon") )
	{
		if ( parts.size() < 3 ) return 0;
		KopeteAccount *account = KopeteAccountManager::manager()->findAccount( parts[1], parts[2] );
		if ( !account ) return 0;
		img = account->myself()->onlineStatus().iconFor( account->myself() );
	}

	if ( parts[0] == QString::fromLatin1("kopete-metacontact-icon") )
	{
		if ( parts.size() < 2 ) return 0;
		KopeteMetaContact *mc = KopeteContactList::contactList()->metaContact( parts[1] );
		img = SmallIcon( mc->statusIcon() );
	}

	delete d->lastMimeSource;
	d->lastMimeSource = new QImageDrag( img.convertToImage() );
	return d->lastMimeSource;
}

} // END namespace Kopete

// vim: set noet ts=4 sts=4 sw=4:
