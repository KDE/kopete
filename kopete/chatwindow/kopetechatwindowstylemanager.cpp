 /*
    kopetechatwindowstylemanager.cpp - Manager all chat window styles

    Copyright (c) 2005      by Michaël Larouche     <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetechatwindowstylemanager.h"

// Qt includes
#include <qvaluestack.h>

// KDE includes
#include <kstandarddirs.h>
#include <kdirlister.h>
#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>
#include <karchive.h>
#include <kzip.h>
#include <ktar.h>
#include <kmimetype.h>
#include <kio/netaccess.h>

#include "kopetechatwindowstyle.h"

class ChatWindowStyleManager::Private
{
public:
	Private()
	 : styleDirLister(0)
	{}

	~Private()
	{
		if(styleDirLister)
		{
			styleDirLister->deleteLater();
		}

		StyleList::Iterator styleIt, styleItEnd = availableStyles.end();
		for(styleIt = availableStyles.begin(); styleIt != styleItEnd; ++styleIt)
		{
			delete styleIt.data();
		}
	}

	KDirLister *styleDirLister;
	StyleList availableStyles;

	QValueStack<KURL> styleDirs;
};

ChatWindowStyleManager *ChatWindowStyleManager::s_self = 0;

ChatWindowStyleManager *ChatWindowStyleManager::self()
{
	if( !s_self )
		s_self = new ChatWindowStyleManager;
	
	return s_self;
}

ChatWindowStyleManager::ChatWindowStyleManager(QObject *parent, const char *name)
	: QObject(parent, name), d(new Private())
{
	kdDebug(14000) << k_funcinfo << endl;
}

ChatWindowStyleManager::~ChatWindowStyleManager()
{	
	kdDebug(14000) << k_funcinfo << endl;
	delete d;
}

void ChatWindowStyleManager::loadStyles()
{
	// Clear current Dir Lister.
	if(d->styleDirLister)
	{
		d->styleDirLister->deleteLater();
		d->styleDirLister = 0L;
	}
	
	// Clear current availableStyles
	// FIXME: It crash :(
// 	StyleList::Iterator styleIt, styleItEnd = d->availableStyles.end();
// 	for(styleIt = d->availableStyles.begin(); styleIt != styleItEnd; ++styleIt)
// 	{
// 		delete styleIt.data();
// 		styleIt.data() = 0L;
// 	}
// 	d->availableStyles.clear();

	QStringList chatStyles = KGlobal::dirs()->findDirs( "appdata", QString::fromUtf8( "styles" ) );
	QStringList::const_iterator it;
	for(it = chatStyles.constBegin(); it != chatStyles.constEnd(); ++it)
	{
		kdDebug(14010) << k_funcinfo << *it << endl;
		d->styleDirs.push( KURL(*it) );
	}
	
	d->styleDirLister = new KDirLister;
	d->styleDirLister->setDirOnlyMode(true);

	connect(d->styleDirLister, SIGNAL(newItems(const KFileItemList &)), this, SLOT(slotNewStyles(const KFileItemList &)));
	connect(d->styleDirLister, SIGNAL(completed()), this, SLOT(slotDirectoryFinished()));

	d->styleDirLister->openURL(d->styleDirs.pop(), true);
}

ChatWindowStyleManager::StyleList ChatWindowStyleManager::getAvailableStyles()
{
	return d->availableStyles;
}

int ChatWindowStyleManager::installStyle(const QString &styleBundlePath)
{
	QString localStyleDir( locateLocal( "appdata", QString::fromUtf8("styles/") ) );
	
	KArchiveEntry *currentEntry = 0L;
	KArchiveDirectory* currentDir = 0L;
	KArchive *archive = 0L;

	if( localStyleDir.isEmpty() )
	{
		return StyleNoDirectoryValid;
	}

	// Find mimetype for current bundle. ZIP and KTar need separate constructor
	QString currentBundleMimeType = KMimeType::findByPath(styleBundlePath, 0, false)->name();
	if(currentBundleMimeType == "application/x-zip")
	{
		archive = new KZip(styleBundlePath);
	}
	else if( currentBundleMimeType == "application/x-tgz" || currentBundleMimeType == "application/x-tbz" || currentBundleMimeType == "application/x-gzip" || currentBundleMimeType == "application/x-bzip2" )
	{
		archive = new KTar(styleBundlePath);
	}
	else
	{
		return StyleCannotOpen;
	}

	if ( !archive->open(IO_ReadOnly) )
	{
		delete archive;

		return StyleCannotOpen;
	}

	const KArchiveDirectory* rootDir = archive->directory();

	// Ok where we go to check if the archive is valid.
	// Each time we found a correspondance to a theme bundle, we add a point to validResult.
	// A valid style bundle must have:
	// -a Contents, Contents/Resources, Co/Res/Incoming, Co/Res/Outgoing dirs
	// main.css, Footer.html, Header.html, Status.html files in Contents/Ressources.
	// So for a style bundle to be valid, it must have a result greather than 8, because we test for 8 required entry.
	int validResult = 0;
	QStringList entries = rootDir->entries();
	// Will be reused later.
	QStringList::Iterator entriesIt, entriesItEnd = entries.end();
	for(entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt)
	{
		currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
// 		kdDebug() << k_funcinfo << "Current entry name: " << currentEntry->name() << endl;
		if (currentEntry->isDirectory())
		{
			currentDir = dynamic_cast<KArchiveDirectory*>( currentEntry );
			if (currentDir)
			{
				if( currentDir->entry(QString::fromUtf8("Contents")) )
				{
// 					kdDebug() << k_funcinfo << "Contents found" << endl;
					validResult += 1;
				}
				if( currentDir->entry(QString::fromUtf8("Contents/Resources")) )
				{
// 					kdDebug() << k_funcinfo << "Contents/Resources found" << endl;
					validResult += 1;
				}
				if( currentDir->entry(QString::fromUtf8("Contents/Resources/Incoming")) )
				{
// 					kdDebug() << k_funcinfo << "Contents/Resources/Incoming found" << endl;
					validResult += 1;
				}
				if( currentDir->entry(QString::fromUtf8("Contents/Resources/Outgoing")) )
				{
// 					kdDebug() << k_funcinfo << "Contents/Resources/Outgoing found" << endl;
					validResult += 1;
				}
				if( currentDir->entry(QString::fromUtf8("Contents/Resources/main.css")) )
				{
// 					kdDebug() << k_funcinfo << "Contents/Resources/main.css found" << endl;
					validResult += 1;
				}
				if( currentDir->entry(QString::fromUtf8("Contents/Resources/Footer.html")) )
				{
// 					kdDebug() << k_funcinfo << "Contents/Resources/Footer.html found" << endl;
					validResult += 1;
				}
				if( currentDir->entry(QString::fromUtf8("Contents/Resources/Status.html")) )
				{
// 					kdDebug() << k_funcinfo << "Contents/Resources/Status.html found" << endl;
					validResult += 1;
				}
				if( currentDir->entry(QString::fromUtf8("Contents/Resources/Header.html")) )
				{
// 					kdDebug() << k_funcinfo << "Contents/Resources/Header.html found" << endl;
					validResult += 1;
				}
			}
		}
	}
// 	kdDebug() << k_funcinfo << "Valid result: " << QString::number(validResult) << endl;
	// The archive is a valid style bundle.
	if(validResult >= 8)
	{
		bool installOk = false;
		for(entriesIt = entries.begin(); entriesIt != entries.end(); ++entriesIt)
		{
			currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*entriesIt));
			if(currentEntry && currentEntry->isDirectory())
			{
				// Ignore this MacOS X "garbage" directory in zip.
				if(currentEntry->name() == QString::fromUtf8("__MACOSX"))
				{
					continue;
				}
				else
				{
					currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);
					if(currentDir)
					{
						currentDir->copyTo(localStyleDir + currentDir->name());
						installOk = true;
					}
				}
			}
		}

		archive->close();
		delete archive;

		if(installOk)
			return StyleInstallOk;
		else
			return StyleUnknow;
	}
	else
	{
		archive->close();
		delete archive;

		return StyleNotValid;
	}

	if(archive)
	{
		archive->close();
		delete archive;
	}

	return StyleUnknow;
}

bool ChatWindowStyleManager::removeStyle(const QString &styleName)
{
	ChatWindowStyle *deletedStyle = d->availableStyles[styleName];
	if(deletedStyle)
	{
		QString stylePath = deletedStyle->getStylePath();
		// Remove style from Available Styles
		d->availableStyles.remove(styleName);

		// Do the actual deletion of the directory style.
		return KIO::NetAccess::del( KURL(stylePath), 0 );
	}
	return false;
}

void ChatWindowStyleManager::slotNewStyles(const KFileItemList &dirList)
{
	KFileItem *item;
	QPtrListIterator<KFileItem> it( dirList );
	while( (item = it.current()) != 0 ) 
	{
		// Ignore data dir(from deprecated XSLT themes)
		if( !item->url().fileName().contains(QString::fromUtf8("data")) )
		{
			kdDebug(14010) << k_funcinfo << "Listing: " << item->url().fileName() << endl;
			// Build a chat window style and list its variants.
			ChatWindowStyle *newStyle = new ChatWindowStyle(item->url().path(), ChatWindowStyle::StyleBuildNormal);
			// TODO: Use name from Info.plist
			d->availableStyles.insert(item->url().fileName(), newStyle);
		}
		++it;
	}
}

void ChatWindowStyleManager::slotDirectoryFinished()
{
	// Start another scanning if the directories stack is not empty
	if( !d->styleDirs.isEmpty() )
	{
		kdDebug(14010) << k_funcinfo << "Starting another directory." << endl;
		d->styleDirLister->openURL(d->styleDirs.pop(), true);
	}
	else
	{
		emit loadStylesFinished();
	}
}

#include "kopetechatwindowstylemanager.moc"
