/*
    kopetemimetypehandler.cpp - Kopete mime type handlers

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

#include "kopetemimetypehandler.h"
#include "kopeteglobal.h"
#include "kopeteuiglobal.h"

#include <qwidget.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <ktar.h>

namespace Kopete
{

namespace
{
	static QDict<Kopete::MimeTypeHandler> g_mimeHandlers;
}

class MimeTypeHandler::Private
{
public:
	Private( bool carf ) : canAcceptRemoteFiles( carf ) {}
	bool canAcceptRemoteFiles;
	QStringList mimeTypes;
};

MimeTypeHandler::MimeTypeHandler( bool canAcceptRemoteFiles )
 : d( new Private( canAcceptRemoteFiles ) )
{
}

MimeTypeHandler::~MimeTypeHandler()
{
	for( QStringList::iterator it = d->mimeTypes.begin(); it != d->mimeTypes.end(); ++it )
		g_mimeHandlers.remove( *it );

	delete d;
}

bool MimeTypeHandler::registerAsHandler( const QString &mimeType )
{
	if( g_mimeHandlers[ mimeType ] )
	{
		kdWarning(14010) << k_funcinfo << "Warning: Two mime type handlers attempting"
			" to handle " << mimeType << endl;
		return false;
	}

	g_mimeHandlers.insert( mimeType, this );
	d->mimeTypes.append( mimeType );
	kdDebug(14010) << k_funcinfo << "Mime type " << mimeType << " registered" << endl;
	return true;
}

const QStringList MimeTypeHandler::mimeTypes() const
{
	return d->mimeTypes;
}

bool MimeTypeHandler::canAcceptRemoteFiles() const
{
	return d->canAcceptRemoteFiles;
}

bool MimeTypeHandler::dispatchURL( const KURL &url )
{
	if( url.isEmpty() )
		return false;

	QString type = KMimeType::findByURL( url )->name();

	MimeTypeHandler *handler = g_mimeHandlers[ type ];
	if( !handler )
	{
		kdDebug(14010) << "No mime type handler found for " << url.prettyURL() << " of type " << type << endl;
		return false;
	}

	if( !handler->canAcceptRemoteFiles() )
	{
		QString file;
		#if KDE_IS_VERSION( 3, 1, 90 )
		if( !KIO::NetAccess::download( url, file, Kopete::UI::Global::mainWidget() ) )
		#else
		if( !KIO::NetAccess::download( url, file ) )
		#endif
		{
			QString sorryText;
			if ( url.isLocalFile() )
			{
				sorryText = i18n( "Unable to find the file %1." );
			}
			else
			{
				sorryText = i18n( "<qt>Unable to download the requested file;<br>"
				                  "please check that address %1 is correct.</qt>" );
			}

			KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
			                    sorryText.arg( url.prettyURL() ) );
			return false;
		}

		KURL dest; dest.setPath( file );
		handler->handleURL( type, dest );
		// for now, local-only handlers have to be synchronous
		KIO::NetAccess::removeTempFile( file );
	}
	else
	{
		handler->handleURL( type, url );
	}

	return true;
}

EmoticonMimeTypeHandler::EmoticonMimeTypeHandler()
 : MimeTypeHandler( false )
{
	registerAsHandler( QString::fromLatin1("application/x-kopete-emoticons") );
	registerAsHandler( QString::fromLatin1("application/x-tgz") );
	registerAsHandler( QString::fromLatin1("application/x-tbz") );
}

void EmoticonMimeTypeHandler::handleURL( const QString &, const KURL &url ) const
{
	Global::installEmoticonTheme( url.path() );
}

} // END namespace Kopete

// vim: set noet ts=4 sts=4 sw=4:
