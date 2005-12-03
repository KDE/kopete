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

void ChatWindowStyleManager::installStyle()
{
	
}

void ChatWindowStyleManager::removeStyle()
{
	
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
