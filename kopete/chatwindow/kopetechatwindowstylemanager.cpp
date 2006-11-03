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
#include <kstaticdeleter.h>
#include <kconfig.h>
#include <kglobal.h>

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

		QMap<QString, ChatWindowStyle*>::Iterator styleIt, styleItEnd = stylePool.end();
		for(styleIt = stylePool.begin(); styleIt != styleItEnd; ++styleIt)
		{
			delete styleIt.data();
		}
	}

	KDirLister *styleDirLister;
	StyleList availableStyles;
	
	// key = style path, value = ChatWindowStyle instance
	QMap<QString, ChatWindowStyle*> stylePool;

	QValueStack<KURL> styleDirs;
};

static KStaticDeleter<ChatWindowStyleManager> ChatWindowStyleManagerstaticDeleter;

ChatWindowStyleManager *ChatWindowStyleManager::s_self = 0;

ChatWindowStyleManager *ChatWindowStyleManager::self()
{
	if( !s_self )
	{
		ChatWindowStyleManagerstaticDeleter.setObject( s_self, new ChatWindowStyleManager() );
	}
	
	return s_self;
}

ChatWindowStyleManager::ChatWindowStyleManager(QObject *parent, const char *name)
	: QObject(parent, name), d(new Private())
{
	kdDebug(14000) << k_funcinfo << endl;
	loadStyles();
}

ChatWindowStyleManager::~ChatWindowStyleManager()
{	
	kdDebug(14000) << k_funcinfo << endl;
	delete d;
}

void ChatWindowStyleManager::loadStyles()
{
	QStringList chatStyles = KGlobal::dirs()->findDirs( "appdata", QString::fromUtf8( "styles" ) );
        QString localStyleDir( locateLocal( "appdata", QString::fromUtf8("styles/"),true) );
        if( !chatStyles.contains(localStyleDir))
                chatStyles<<localStyleDir;

	QStringList::const_iterator it;
	for(it = chatStyles.constBegin(); it != chatStyles.constEnd(); ++it)
	{
		kdDebug(14000) << k_funcinfo << *it << endl;
		d->styleDirs.push( KURL(*it) );
	}
	
	d->styleDirLister = new KDirLister(this);
	d->styleDirLister->setDirOnlyMode(true);

	connect(d->styleDirLister, SIGNAL(newItems(const KFileItemList &)), this, SLOT(slotNewStyles(const KFileItemList &)));
	connect(d->styleDirLister, SIGNAL(completed()), this, SLOT(slotDirectoryFinished()));

	if( !d->styleDirs.isEmpty() )
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
				if( currentDir->entry(QString::fromUtf8("Contents/Resources/Incoming/Content.html")) )
				{
// 					kdDebug() << k_funcinfo << "Contents/Resources/Incoming/Content.html found" << endl;
					validResult += 1;
				}
				if( currentDir->entry(QString::fromUtf8("Contents/Resources/Outgoing/Content.html")) )
				{
// 					kdDebug() << k_funcinfo << "Contents/Resources/Outgoing/Content.html found" << endl;
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

bool ChatWindowStyleManager::removeStyle(const QString &stylePath)
{
	// Find for the current style in avaiableStyles map.
        KURL urlStyle(stylePath);
	QString styleName=urlStyle.fileName();
	StyleList::Iterator foundStyle = d->availableStyles.find(styleName);
	// QMap iterator return end() if it found no item.
	if(foundStyle != d->availableStyles.end())
	{
		d->availableStyles.remove(foundStyle);
		
		// Remove and delete style from pool if needed.
		if( d->stylePool.contains(stylePath) )
		{
			ChatWindowStyle *deletedStyle = d->stylePool[stylePath];
			d->stylePool.remove(stylePath);
			delete deletedStyle;
		}
	
		// Do the actual deletion of the directory style.
		return KIO::NetAccess::del( urlStyle, 0 );
	}
	else
	{
		return false;
	}
}

ChatWindowStyle *ChatWindowStyleManager::getStyleFromPool(const QString &stylePath)
{
	if( d->stylePool.contains(stylePath) )
	{
		// NOTE: This is a hidden config switch for style developers
		// Check in the config if the cache is disabled.
		// if the cache is disabled, reload the style everytime it's getted.
		KConfig *config = KGlobal::config();
		config->setGroup("KopeteStyleDebug");
		bool disableCache = config->readBoolEntry("disableStyleCache", false);
		if(disableCache)
		{
			d->stylePool[stylePath]->reload();
		}

		return d->stylePool[stylePath];
	}
	else
	{
		// Build a chat window style and list its variants, then add it to the pool.
		ChatWindowStyle *style = new ChatWindowStyle(stylePath, ChatWindowStyle::StyleBuildNormal);
		d->stylePool.insert(stylePath, style);

		return style;
	}
	
	return 0;
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
			kdDebug(14000) << k_funcinfo << "Listing: " << item->url().fileName() << endl;
			// If the style path is already in the pool, that's mean the style was updated on disk
			// Reload the style
			if( d->stylePool.contains(item->url().path()) )
			{
				kdDebug(14000) << k_funcinfo << "Updating style: " << item->url().path() << endl;

				d->stylePool[item->url().path()]->reload();

				// Add to avaialble if required.
				if( !d->availableStyles.contains(item->url().fileName()) )
					d->availableStyles.insert(item->url().fileName(), item->url().path());
			}
			else
			{
				// TODO: Use name from Info.plist
				d->availableStyles.insert(item->url().fileName(), item->url().path());
			}
		}
		++it;
	}
}

void ChatWindowStyleManager::slotDirectoryFinished()
{
	// Start another scanning if the directories stack is not empty
	if( !d->styleDirs.isEmpty() )
	{
		kdDebug(14000) << k_funcinfo << "Starting another directory." << endl;
		d->styleDirLister->openURL(d->styleDirs.pop(), true);
	}
	else
	{
		emit loadStylesFinished();
	}
}

#include "kopetechatwindowstylemanager.moc"
