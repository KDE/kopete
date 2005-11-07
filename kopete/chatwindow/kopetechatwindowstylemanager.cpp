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
#include <qmap.h>

// KDE includes
#include <kstandarddirs.h>
#include <kdirlister.h>
#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>

class ChatWindowStyleManager::Private
{
public:
	Private()
	 : styleDirLister(0)
	{}

	KDirLister *styleDirLister;
	// key=style name, value=path
	QMap<QString, QString> styleNamePathMap;

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
	
}

ChatWindowStyleManager::~ChatWindowStyleManager()
{
	delete d;
}

void ChatWindowStyleManager::load()
{
	// TODO: Init latest current chat window style.
}

void ChatWindowStyleManager::loadStyles()
{
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
			d->styleNamePathMap.insert(item->url().fileName(), item->url().path());
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
		// TODO: Remove this debug code.
		// TODO: Call building of each chat window style instead.
		QMap<QString, QString>::const_iterator it;
		for(it = d->styleNamePathMap.constBegin(); it != d->styleNamePathMap.constEnd(); ++it)
		{
			kdDebug(14010) << k_funcinfo << "Theme name: " << it.key() << " path: " << it.data() << endl;
		}
	}
}

#include "kopetechatwindowstylemanager.moc"
