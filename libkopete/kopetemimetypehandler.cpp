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

#include <kdebug.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kemoticons.h>
#include <kopeteemoticons.h>

namespace Kopete
{

namespace
{
	static QHash<QString, Kopete::MimeTypeHandler*> g_mimeHandlers;
	static QHash<QString, Kopete::MimeTypeHandler*> g_protocolHandlers;
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
		kWarning(14010) << "Warning: Two mime type handlers attempting"
			" to handle " << mimeType << endl;
		return false;
	}

	g_mimeHandlers.insert( mimeType, this );
	d->mimeTypes.append( mimeType );
//	kDebug(14010) << "Mime type " << mimeType << " registered";
	return true;
}

bool MimeTypeHandler::registerAsProtocolHandler( const QString &protocol )
{
	if( g_protocolHandlers[ protocol ] )
	{
		kWarning(14010) << "Warning: Two protocol handlers attempting"
			" to handle " << protocol << endl;
		return false;
	}

	g_protocolHandlers.insert( protocol, this );
	d->protocols.append( protocol );
	kDebug(14010) << "Mime type " << protocol << " registered";
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

bool MimeTypeHandler::dispatchURL( const KUrl &url )
{
	if( url.isEmpty() )
		return false;

	QString type = KMimeType::findByUrl( url )->name();

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
			mimeHandler->handleURL( QString(), url );
			return true;
		}
		else
		{
			kDebug(14010) << "No mime type handler can handle this URL: " << url.prettyUrl();
			return false;
		}
	}
}

bool MimeTypeHandler::dispatchToHandler( const KUrl &url, const QString &mimeType, MimeTypeHandler *handler )
{
	if( !handler->canAcceptRemoteFiles() )
	{
		QString file;
		if( !KIO::NetAccess::download( url, file, Kopete::UI::Global::mainWidget() ) )
		{
			QString sorryText;
			if ( url.isLocalFile() )
			{
				sorryText = i18n( "Unable to find the file %1.", url.prettyUrl() );
			}
			else
			{
				sorryText = i18n( "<qt>Unable to download the requested file;<br />"
				                  "please check that address %1 is correct.</qt>",
				                  url.prettyUrl() );
			}

			KMessageBox::sorry( Kopete::UI::Global::mainWidget(), sorryText );
			return false;
		}

		KUrl dest;
		dest.setPath( file );

		handler->handleURL( mimeType, dest );

		// for now, local-only handlers have to be synchronous
		KIO::NetAccess::removeTempFile( file );
	}
	else
	{
		handler->handleURL( mimeType, url );
	}

	return true;
}

void MimeTypeHandler::handleURL( const QString &mimeType, const KUrl &url ) const
{
	Q_UNUSED( mimeType );
	Q_UNUSED( url );
}

void MimeTypeHandler::handleURL( const KUrl &url ) const
{
	handleURL( QString(), url );
}


EmoticonMimeTypeHandler::EmoticonMimeTypeHandler()
 : MimeTypeHandler( false )
{
	registerAsMimeHandler( QString::fromLatin1("application/x-kopete-emoticons") );
	registerAsMimeHandler( QString::fromLatin1("application/x-compressed-tar") );
	registerAsMimeHandler( QString::fromLatin1("application/x-bzip-compressed-tar") );
}

void EmoticonMimeTypeHandler::handleURL( const QString &, const KUrl &url ) const
{
  Emoticons::self()->installTheme( url.toLocalFile() );
}

void EmoticonMimeTypeHandler::handleURL( const KUrl &url ) const
{
	handleURL( QString(), url );
}


} // END namespace Kopete

// vim: set noet ts=4 sts=4 sw=4:
