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
		bool exclusive = false;
		PluginListView *parent = ( *i ).type == "Kopete/Protocol" ? protocolList : otherList;

		PluginListItem *item = new PluginListItem( exclusive, loaded.contains( *i ), *i, parent );
		item->setText( 0, ( *i ).name );
		item->setText( 1, ( *i ).comment );
		item->setText( 2, ( *i ).author );
		item->setText( 3, ( *i ).license );
	}
}

void PluginConfig::stateChange(PluginListItem *item, bool b)
{
	if(b)
		addPlugin( item->info().specfile );
	else
		removePlugin( item->info().specfile );
}

void PluginConfig::addPlugin( const QString &specFile )
{
	if( mDeleted.contains( specFile ) )
		mDeleted.remove( specFile );
	else if( !mAdded.contains( specFile ) )
		mAdded.append( specFile );
}

void PluginConfig::removePlugin( const QString &specFile )
{
	if( mAdded.contains( specFile ) )
		mAdded.remove( specFile );
	else if( !mDeleted.contains( specFile ) )
		mDeleted.append( specFile );
}

void PluginConfig::save()
{
	LibraryLoader *loader = LibraryLoader::pluginLoader();

	// Load the plugins the user added
	for(QStringList::Iterator i = mAdded.begin(); i != mAdded.end(); ++i)
		loader->loadPlugin( *i );

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

