/*
    kopeteglobal.cpp - Kopete Globals

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

EmoticonHandler::EmoticonHandler()
 : MimeTypeHandler( false )
{
	registerAsHandler( QString::fromLatin1("application/x-kopete-emoticons") );
	registerAsHandler( QString::fromLatin1("application/x-tgz") );
	registerAsHandler( QString::fromLatin1("application/x-tbz") );
}

void EmoticonHandler::handleURL( const QString &, const KURL &url ) const
{
	QString archiveName = url.path();
	QStringList foundThemes;
	KArchiveEntry *currentEntry = 0L;
	KArchiveDirectory* currentDir = 0L;
	KProgressDialog *progressDlg = 0L;
	KTar *archive = 0L;

	QString localThemesDir(locateLocal("data",
		QString::fromLatin1("kopete/pics/emoticons/")));

	if(localThemesDir.isEmpty())
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error, i18n("Could not find suitable place " \
			"to install emoticon themes into."));
		return;
	}

	progressDlg = new KProgressDialog(0 , "emoticonInstProgress",
	 	i18n("Installing Emoticon Themes..."), QString::null, true);
	progressDlg->progressBar()->setTotalSteps(foundThemes.count());
	progressDlg->show();
	kapp->processEvents();

	archive = new KTar(archiveName);
	if ( !archive->open(IO_ReadOnly) )
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error,
			i18n("Could not open \"%1\" for unpacking.").arg(archiveName));
		delete archive;
		delete progressDlg;
		return;
	}

	const KArchiveDirectory* rootDir = archive->directory();

	// iterate all the dirs looking for an emoticons.xml file
	QStringList entries = rootDir->entries();
	for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
	{
		currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*it));
		if (currentEntry->isDirectory())
		{
			currentDir = dynamic_cast<KArchiveDirectory*>( currentEntry );
			if (currentDir && (currentDir->entry(QString::fromLatin1("emoticons.xml")) != NULL))
				foundThemes.append(currentDir->name());
		}
	}

	if (foundThemes.isEmpty())
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error, i18n("<qt>The file \"%1\" is not a valid" \
				" emoticon theme archive!</qt>").arg(archiveName));
		archive->close();
		delete archive;
		delete progressDlg;
		return;
	}

	for (QStringList::ConstIterator it = foundThemes.begin(); it != foundThemes.end(); ++it)
	{
		progressDlg->setLabel(
			i18n("<qt>Installing <strong>%1</strong> emoticon theme</qt>")
			.arg(*it));
		progressDlg->resize(progressDlg->sizeHint());
		kapp->processEvents();

		if (progressDlg->wasCancelled())
			break;

		currentEntry = const_cast<KArchiveEntry *>(rootDir->entry(*it));
		if (currentEntry == 0)
		{
			kdDebug(14010) << k_funcinfo << "couldn't get next archive entry" << endl;
			continue;
		}

		if(currentEntry->isDirectory())
		{
			currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
			if (currentDir == 0)
			{
				kdDebug(14010) << k_funcinfo <<
					"couldn't cast archive entry to KArchiveDirectory" << endl;
				continue;
			}
			currentDir->copyTo(localThemesDir + *it);
			progressDlg->progressBar()->advance(1);
		}
	}

	archive->close();
	delete archive;

	// check if all steps were done, if there are skipped ones then we didn't
	// succeed copying all dirs from the tarball
	if (progressDlg->progressBar()->totalSteps() !=
		progressDlg->progressBar()->progress())
	{
		KMessageBox::queuedMessageBox(Kopete::UI::Global::mainWidget(),
			KMessageBox::Error,
			i18n("<qt>A problem occurred during the installation process. "
			"However, most of the emoticon themes in the archive have been " \
			"installed.</qt>"));
	}

	delete progressDlg;

	KIO::NetAccess::removeTempFile( archiveName );
}

bool Global::handleURL( const KURL &url )
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
				sorryText = i18n( "Unable to find the file %1!" );
			}
			else
			{
				sorryText = i18n( "<qt>Unable to download the requested file.<br>"
				                  "Please check that address %1 is correct.</qt>" );
			}

			KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
			                    sorryText.arg( url.prettyURL() ) );
			return false;
		}

		KURL dest; dest.setPath( file );
		handler->handleURL( type, dest );
	}
	else
	{
		handler->handleURL( type, url );
	}

	return true;
}

} // END namespace Kopete

// vim: set noet ts=4 sts=4 sw=4:
