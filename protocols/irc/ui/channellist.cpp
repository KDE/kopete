/*
    channellist.cpp - IRC Channel Search Widget

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <klocale.h>
#include <kmessagebox.h>

#include <qvariant.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include "kirc.h"
#include "channellist.h"

class ChannelListItem : public QListViewItem
{
	public:
		ChannelListItem( QListView *parent, QString arg1, QString arg2, QString arg3 );
		virtual int compare( QListViewItem *i, int col, bool ascending ) const;
};

ChannelListItem::ChannelListItem( QListView *parent, QString arg1, QString arg2, QString arg3 ) :
	QListViewItem( parent, parent->lastItem() )
{
	setText(0, arg1);
	setText(1, arg2);
	setText(2, arg3);
}

int ChannelListItem::compare( QListViewItem *i, int col, bool ascending ) const
{
	if( col == 1 )
	{
		if( text(1).toUInt() < i->text(1).toUInt() )
			return -1;
		else if ( text(1).toUInt() == i->text(1).toUInt() )
			return 0;
		else
			return 1;
	}
	else
		return QListViewItem::compare( i, col, ascending );
}

ChannelList::ChannelList( QWidget* parent, KIRC *engine )
    : QWidget( parent ), m_engine( engine )
{
	ChannelListLayout = new QVBoxLayout( this, 11, 6, "ChannelListLayout");

	layout72_2 = new QHBoxLayout( 0, 0, 6, "layout72_2");

	textLabel1_2 = new QLabel( this, "textLabel1_2" );
	layout72_2->addWidget( textLabel1_2 );

	channelSearch = new QLineEdit( this, "channelSearch" );
	layout72_2->addWidget( channelSearch );

	numUsers = new QSpinBox( 0, 32767, 1, this, "num_users" );
	numUsers->setSuffix( i18n(" members") );
	layout72_2->addWidget( numUsers );

	mSearchButton = new QPushButton( this, "mSearchButton" );
	layout72_2->addWidget( mSearchButton );
	ChannelListLayout->addLayout( layout72_2 );

	mChannelList = new QListView( this, "mChannelList" );
	mChannelList->addColumn( i18n( "Channel" ) );
	mChannelList->addColumn( i18n( "Users" ) );
	mChannelList->header()->setResizeEnabled( FALSE, mChannelList->header()->count() - 1 );
	mChannelList->addColumn( i18n( "Topic" ) );
	mChannelList->setAllColumnsShowFocus( TRUE );
	mChannelList->setShowSortIndicator( TRUE );
	ChannelListLayout->addWidget( mChannelList );

	clearWState( WState_Polished );

	textLabel1_2->setText( i18n( "Search for:" ) );
	QToolTip::add( textLabel1_2, i18n( "You may search for channels on the IRC server for a text string entered here." ) );
	QToolTip::add( numUsers, i18n( "Channels returned must have at least this many members." ) );
	QWhatsThis::add( numUsers, i18n( "Channels returned must have at least this many members." ) );
	QWhatsThis::add( textLabel1_2, i18n( "You may search for channels on the IRC server for a text string entered here.  For instance, you may type 'linux' to find channels that have something to do with linux." ) );
	QToolTip::add( channelSearch, i18n( "You may search for channels on the IRC server for a text string entered here." ) );
	QWhatsThis::add( channelSearch, i18n( "You may search for channels on the IRC server for a text string entered here.  For instance, you may type 'linux' to find channels that have something to do with linux." ) );
	mSearchButton->setText( i18n( "S&earch" ) );
	QToolTip::add( mSearchButton, i18n( "Perform a channel search." ) );
	QWhatsThis::add( mSearchButton, i18n( "Perform a channel search.  Please be patient, as this can be slow depending on the number of channels on the server." ) );
	QToolTip::add( mChannelList, i18n( "Double click on a channel to select it." ) );
	mChannelList->header()->setLabel( 0, i18n( "Channel" ) );
	mChannelList->header()->setLabel( 1, i18n( "Users" ) );
	mChannelList->header()->setLabel( 2, i18n( "Topic" ) );

	// signals and slots connections
	connect( mChannelList, SIGNAL( doubleClicked(QListViewItem*) ),
		this, SLOT( slotItemDoubleClicked(QListViewItem*) ) );

	connect( mSearchButton, SIGNAL( clicked() ), this, SLOT( search() ) );

	connect( mChannelList, SIGNAL( selectionChanged( QListViewItem*) ), this,
		SLOT( slotItemSelected( QListViewItem *) ) );

	connect( m_engine, SIGNAL( incomingListedChan( const QString &, uint, const QString & ) ),
		this, SLOT( slotChannelListed( const QString &, uint, const QString & ) ) );

	connect( m_engine, SIGNAL( incomingEndOfList() ), this, SLOT( slotListEnd() ) );

	connect( m_engine, SIGNAL( connectedToServer() ), this, SLOT( reset() ) );
	connect( m_engine, SIGNAL( disconnected() ), this, SLOT( slotDisconnected() ) );

	show();
}

void ChannelList::slotItemDoubleClicked( QListViewItem *i )
{
	emit channelDoubleClicked( i->text(0) );
}

void ChannelList::slotItemSelected( QListViewItem *i )
{
	emit channelSelected( i->text(0) );
}

void ChannelList::reset()
{
	channelCache.clear();
	clear();
}

void ChannelList::clear()
{
	mChannelList->clear();
	channelSearch->clear();
	channelSearch->setFocus();
}

void ChannelList::search()
{
	if( m_engine->isConnected() || !channelCache.isEmpty() )
	{
		mChannelList->clear();
		mChannelList->setSorting( -1 );
		mSearchButton->setEnabled(false);
		mSearch = channelSearch->text();
		mSearching = true;
		mUsers = numUsers->value();

		if( channelCache.isEmpty() )
			m_engine->list();
		else
		{
			cacheIterator = channelCache.begin();
			slotSearchCache();
		}
	}
	else
	{
		KMessageBox::queuedMessageBox(
			this, KMessageBox::Error,
			i18n("You must be connected to the IRC server to perform a channel listing."),
			i18n("Not Connected"), 0
		);
	}
}

void ChannelList::slotChannelListed( const QString &channel, uint users, const QString &topic )
{
	checkSearchResult( channel, users, topic );
	channelCache.insert( channel, QPair< uint, QString >( users, topic ) );
}

void ChannelList::checkSearchResult( const QString &channel, uint users, const QString &topic )
{
	if( ( mUsers == 0 || mUsers <= users ) &&
		( mSearch.isEmpty() || channel.contains( mSearch, false ) || topic.contains( mSearch, false ) )
	)
	{
		new ChannelListItem( mChannelList, channel, QString::number(users), topic );
	}
}

void ChannelList::slotSearchCache()
{
	if( cacheIterator != channelCache.end() )
	{
		checkSearchResult( cacheIterator.key(), cacheIterator.data().first, cacheIterator.data().second );
		++cacheIterator;
		QTimer::singleShot( 0, this, SLOT( slotSearchCache() ) );
	}
	else
	{
		slotListEnd();
	}
}

void ChannelList::slotDisconnected()
{
	KMessageBox::queuedMessageBox(
		this, KMessageBox::Error, i18n("You have been disconnected from the IRC server."),
		i18n("Disconnected"), 0
	);

	slotListEnd();
}

void ChannelList::slotListEnd()
{
	mChannelList->setSorting(0, true);
	mSearchButton->setEnabled(true);
	mSearching = false;
}

#include "channellist.moc"
