/*
    pluginconfig.cpp  -  Kopete Plugin Module

    Copyright (c) 2001-2002 Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002      The Kopete developers  <kopete-devel@kde.org>

    Based on Noatun plugin selection code
    Copyright (c) 2000-2001 Charles Samuels        <charles@kde.org>
    Copyright (c) 2000-2001 Neil Stevens           <neil@qualityassistant.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <ktabctl.h>

#include "pluginconfig.h"

PluginListItem::PluginListItem(const bool _exclusive, bool _checked, const KopeteLibraryInfo &_info, QListView *_parent)
	: QCheckListItem(_parent, _info.name, CheckBox)
	, mInfo(_info)
	, silentStateChange(false)
	, exclusive(_exclusive)
{
	setChecked(_checked);
	if( _checked )
		static_cast<PluginListView *>( listView() )->m_count++;
}

void PluginListItem::setChecked(bool b)
{
	silentStateChange = true;
	setOn(b);
	silentStateChange = false;
}

void PluginListItem::stateChange(bool b)
{
	if(!silentStateChange)
		static_cast<PluginListView *>(listView())->stateChanged(this, b);
}

void PluginListItem::paintCell(QPainter *p, const QColorGroup &cg, int a, int b, int c)
{
	/* WTF? Is this even needed? Breaks compilation, anyhoo. -DS */
	// if(exclusive) myType = RadioButton;
	QCheckListItem::paintCell(p, cg, a, b, c);
	// if(exclusive) myType = CheckBox;
}

PluginListView::PluginListView( QWidget *parent, const char *name )
: KListView( parent, name )
{
	m_count = 0;

	addColumn( i18n( "Name" ) );
	addColumn( i18n( "Description" ) );
	addColumn( i18n( "Author" ) );
	addColumn( i18n( "License" ) );
}

void PluginListView::clear()
{
	m_count = 0;
	KListView::clear();
}

void PluginListView::stateChanged( PluginListItem *item, bool b )
{
	if( b )
	{
		m_count++;
		emit stateChange( item, b );
	}
	else
	{
		if( !m_count )
		{
			item->setChecked( true );
		}
		else
		{
			m_count--;
			emit stateChange( item, b );
		}
	}
}

PluginConfig::PluginConfig( QObject *_parent )
: ConfigModule( i18n( "PluginConfig" ), i18n( "Choose Your PluginConfig" ),
	"input_devices_settings", _parent )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	KTabCtl *tabControl = new KTabCtl( this );

	QFrame *protocolTab = new QFrame( tabControl );
	( new QVBoxLayout( protocolTab, KDialog::marginHint(),
		KDialog::spacingHint() ) )->setAutoAdd( true );
	( void ) new QLabel( i18n( "<b>Choose protocol plugins to use:</b>" ),
		protocolTab );

	protocolList = new PluginListView( protocolTab );
	connect( protocolList, SIGNAL( stateChange( PluginListItem *, bool ) ),
		this, SLOT( stateChange( PluginListItem *, bool ) ) );
	tabControl->addTab(protocolTab, i18n("Protocols"));

	QFrame *otherTab = new QFrame(tabControl);
	( new QVBoxLayout( otherTab, KDialog::marginHint(),
		KDialog::spacingHint() ) )->setAutoAdd( true );
	( void ) new QLabel( i18n( "<b>Choose plugins to use</b>" ), otherTab );

	otherList = new PluginListView( otherTab );
	connect(otherList, SIGNAL(stateChange(PluginListItem *, bool)), this, SLOT(stateChange(PluginListItem *, bool)));
	tabControl->addTab(otherTab, i18n("Other PluginConfig"));
}

void PluginConfig::reopen()
{
	//playlistList->clear();
	//interfaceList->clear();
	protocolList->clear();
	otherList->clear();

	QValueList<KopeteLibraryInfo> available = LibraryLoader::pluginLoader()->available();
	QValueList<KopeteLibraryInfo> loaded = LibraryLoader::pluginLoader()->loaded();

	for(QValueList<KopeteLibraryInfo>::Iterator i = available.begin(); i != available.end(); ++i)
	{
		PluginListView *parent;
		bool exclusive = false;

		if((*i).type == "protocol")
		{
			parent = protocolList;
		}
		else
		{
			parent = otherList;
		}
		/*
		else if((*i).type == "playlist")
		{
			parent = playlistList;
			exclusive = true;
		}
		else if((*i).type == "sm" || (*i).type=="hidden")
			parent = 0;
		else
			parent = otherList;
     	*/
		if(parent)
		{
			PluginListItem *item = new PluginListItem(exclusive, loaded.contains(*i), *i, parent);
			item->setText(0, (*i).name);
			item->setText(1, (*i).comment);
			item->setText(2, (*i).author);
			item->setText(3, (*i).license);
		}
	}
}

void PluginConfig::stateChange(PluginListItem *item, bool b)
{
	if(b)
		addPlugin(item->info());
	else
		removePlugin(item->info());
}

void PluginConfig::addPlugin(const KopeteLibraryInfo &info)
{
	// Load any that this one depends upon
	for(QStringList::ConstIterator i = info.require.begin(); i != info.require.end(); ++i)
	{
		KopeteLibraryInfo requiredInfo = LibraryLoader::pluginLoader()->getInfo(*i);
		PluginListItem *item = findItem(requiredInfo);
		if(item) item->setOn(true);
	}

	if(mDeleted.contains(info.specfile))
		mDeleted.remove(info.specfile);
	else if(!mAdded.contains(info.specfile))
		mAdded.append(info.specfile);
}

void PluginConfig::removePlugin(const KopeteLibraryInfo &info)
{
	LibraryLoader *loader = LibraryLoader::pluginLoader();

	// Here are the ones loaded
	QValueList<KopeteLibraryInfo> loaded = loader->loaded();
	
	// Add the ones marked for loading
	for(QStringList::ConstIterator i = mAdded.begin(); i != mAdded.end(); ++i)
		loaded.append( loader->getInfo( *i ) );

	// Subtract the ones marked for removal
	for(QStringList::ConstIterator i = mDeleted.begin(); i != mDeleted.end(); ++i)
		loaded.remove( loader->getInfo( *i ) );

	// If any depend on this plugin, mark them for removal (or remove them from mAdded)
	for(QValueList<KopeteLibraryInfo>::Iterator i = loaded.begin(); i != loaded.end(); ++i)
	{
		for(QStringList::ConstIterator j = (*i).require.begin(); j != (*i).require.end(); ++j)
		{
			if(*j == info.specfile)
			{
				PluginListItem *item = findItem(*i);
				if(item) item->setOn(false);
			}
		}
	}

	if (mAdded.contains(info.specfile))
		mAdded.remove(info.specfile);
	else if(!mDeleted.contains(info.specfile))
		mDeleted.append(info.specfile);
}

PluginListItem *PluginConfig::findItem(const KopeteLibraryInfo &info) const
{
	for(QListViewItem *cur = protocolList->firstChild(); cur != 0; cur = cur->itemBelow())
	{
		PluginListItem *item = dynamic_cast<PluginListItem *>(cur);
		if(item && item->info() == info)
			return item;
	}

	// If our only interface has a dependency removed, that's a double dose of trouble
	// We may as well have this here for completeness, though
	/*
	for(QListViewItem *cur = interfaceList->firstChild(); cur != 0; cur = cur->itemBelow())
	{
		PluginListItem *item = dynamic_cast<PluginListItem *>(cur);
		if(item && item->info() == info)
			return item;
	}
   */
	// If a playlist is added or removed due to a dependency, we're doom-diddly-oomed
	// We may as well have this here for completeness, though
	 /*
   for(QListViewItem *cur = playlistList->firstChild(); cur != 0; cur = cur->itemBelow())
	{
		PluginListItem *item = dynamic_cast<PluginListItem *>(cur);
		if(item && item->info() == info)
			return item;
	}
	*/

	return 0;
}

void PluginConfig::save()
{
	LibraryLoader *loader = LibraryLoader::pluginLoader();

	// Load the plugins the user added
	for(QStringList::Iterator i = mAdded.begin(); i != mAdded.end(); ++i)
		loader->add( *i );

	// Remove the plugins the user removed
	for (QStringList::Iterator i = mDeleted.begin(); i != mDeleted.end(); ++i)
		loader->remove( *i );

	// Round up the ones that weren't loaded right now, for saving in the configuration
	QStringList specList(mAdded);

	QValueList<KopeteLibraryInfo> loaded = loader->loaded();
	for(QValueList<KopeteLibraryInfo>::Iterator i = loaded.begin(); i != loaded.end(); ++i)
		if(!specList.contains((*i).specfile) && loader->isLoaded( ( *i ).specfile ) )
				specList += (*i).specfile;

	// Now we actually save
	loader->setModules( specList );

	mDeleted.clear();
	mAdded.clear();
}

#include "pluginconfig.moc"

// vim: set noet ts=4 sts=4 sw=4:

