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
	static QDict<Kopete::MimeTypeHandler> g_protocolHandlers;
}

class MimeTypeHandler::Private
{
public:
	Private( bool carf ) : canAcceptRemoteFiles( carf ) {}
	bool canAcceptRemoteFiles;
	QStringList mimeTypes;
	QStringList protocols;
};

MimeTypeHandler::MimeTypeHandler( bool canAcceptRemoteFiles )
 : d( new Private( canAcceptRemoteFiles ) )
{
}

MimeTypeHandler::~MimeTypeHandler()
{
	for( QStringList::iterator it = d->mimeTypes.begin(); it != d->mimeTypes.end(); ++it )
		g_mimeHandlers.remove( *it );

	for( QStringList::iterator it = d->protocols.begin(); it != d->protocols.end(); ++it )
		g_protocolHandlers.remove( *it );

	delete d;
}

bool MimeTypeHandler::registerAsMimeHandler( const QString &mimeType )
{
	if( g_mimeHandlers[ mimeType ] )
	{
		kdWarning(14010) << k_funcinfo << "Warning: Two mime type handlers attempting"
			" to handle " << mimeType << endl;
		return false;
	}

	g_mimeHandlers.insert( mimeType, this );
	d->mimeTypes.append( mimeType );
//	kdDebug(14010) << k_funcinfo << "Mime type " << mimeType << " registered" << endl;
	return true;
}

bool MimeTypeHandler::registerAsProtocolHandler( const QString &protocol )
{
	if( g_protocolHandlers[ protocol ] )
	{
		kdWarning(14010) << k_funcinfo << "Warning: Two protocol handlers attempting"
			" to handle " << protocol << endl;
		return false;
	}

	g_protocolHandlers.insert( protocol, this );
	d->protocols.append( protocol );
	kdDebug(14010) << k_funcinfo << "Mime type " << protocol << " registered" << endl;
	return true;
}

const QStringList MimeTypeHandler::mimeTypes() const
{
	return d->mimeTypes;
}

const QStringList MimeTypeHandler::protocols() const
{
	return d->protocols;
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

	MimeTypeHandler *mimeHandler = g_mimeHandlers[ type ];

	if( mimeHandler )
	{
		return dispatchToHandler( url, type, mimeHandler );
	}
	else
	{
		mimeHandler = g_protocolHandlers[ url.protocol() ];

		if( mimeHandler )
		{
			mimeHandler->handleURL( url );
			return true;
		}
		else
		{
			kdDebug(14010) << "No mime type handler can handle this URL: " << url.prettyURL() << endl;
			return false;
		}
	}
}

bool MimeTypeHandler::dispatchToHandler( const KURL &url, const QString &mimeType, MimeTypeHandler *handler )
{
	if( !handler->canAcceptRemoteFiles() )
	{
		QString file;
		if( !KIO::NetAccess::download( url, file, Kopete::UI::Global::mainWidget() ) )
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

		KURL dest;
		dest.setPath( file );

		if( !mimeType.isNull() )
			handler->handleURL( mimeType, dest );
		else
			handler->handleURL( dest );

		// for now, local-only handlers have to be synchronous
		KIO::NetAccess::removeTempFile( file );
	}
	else
	{
		if( !mimeType.isNull() )
			handler->handleURL( mimeType, url );
		else
			handler->handleURL( url );
	}

	return true;
}

void MimeTypeHandler::handleURL( const KURL &url ) const
{
	Q_UNUSED( url );
}

void MimeTypeHandler::handleURL( const QString &mimeType, const KURL &url ) const
{
	Q_UNUSED( mimeType );
	Q_UNUSED( url );
}


EmoticonMimeTypeHandler::EmoticonMimeTypeHandler()
 : MimeTypeHandler( false )
{
	registerAsMimeHandler( QString::fromLatin1("application/x-kopete-emoticons") );
	registerAsMimeHandler( QString::fromLatin1("application/x-tgz") );
	registerAsMimeHandler( QString::fromLatin1("application/x-tbz") );
}

void EmoticonMimeTypeHandler::handleURL( const QString &, const KURL &url ) const
{
	Global::installEmoticonTheme( url.path() );
}

} // END namespace Kopete

// vim: set noet ts=4 sts=4 sw=4:
