/*
    channellist.cpp - IRC Channel Search Widget

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2004-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "channellist.h"

#include "kircclient.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <qvariant.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include <Q3HBoxLayout>
#include <QPixmap>
#include <Q3VBoxLayout>
#include <k3listview.h>
#include <qlayout.h>

#include <qtimer.h>
#include <qspinbox.h>
#include <Q3Header>

#include <kdebug.h>

class ChannelListItem : public K3ListViewItem
{
	public:
		ChannelListItem( K3ListView *parent, QString arg1, QString arg2, QString arg3 );
		virtual int compare( Q3ListViewItem *i, int col, bool ascending ) const;
		//virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );

	private:
		K3ListView *parentList;
};

ChannelListItem::ChannelListItem( K3ListView *parent, QString arg1, QString arg2, QString arg3 ) :
	K3ListViewItem( parent, parent->lastItem() ), parentList( parent )
{
	setText(0, arg1);
	setText(1, arg2);
	setText(2, arg3);
}

int ChannelListItem::compare( Q3ListViewItem *i, int col, bool ascending ) const
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
		return Q3ListViewItem::compare( i, col, ascending );
}

#if 0
void ChannelListItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
	QPixmap back( width, height() );
	QPainter paint( &back );
	//K3ListViewItem::paintCell( &paint, cg, column, width, align );
	// PASTED FROM KLISTVIEWITEM:
	// set the alternate cell background colour if necessary
	QColorGroup _cg = cg;
	if (isAlternate())
		if (listView()->viewport()->backgroundMode()==Qt::FixedColor)
			_cg.setColor(QColorGroup::Background, static_cast< K3ListView* >(listView())->alternateBackground());
		else
			_cg.setColor(QColorGroup::Base, static_cast< K3ListView* >(listView())->alternateBackground());
	// PASTED FROM QLISTVIEWITEM
	{
		QPainter *p = &paint;

		Q3ListView *lv = listView();
		if ( !lv )
			return;
		QFontMetrics fm( p->fontMetrics() );

		// any text we render is done by the Components, not by this class, so make sure we've nothing to write
		QString t;

		// removed text truncating code from Qt - we do that differently, further on

		int marg = lv->itemMargin();
		int r = marg;
	//	const QPixmap * icon = pixmap( column );

		const BackgroundMode bgmode = lv->viewport()->backgroundMode();
		const QColorGroup::ColorRole crole = QPalette::backgroundRoleFromMode( bgmode );

		if ( _cg.brush( crole ) != lv->colorGroup().brush( crole ) )
			p->fillRect( 0, 0, width, height(), _cg.brush( crole ) );
		else
		{
			// all copied from QListView::paintEmptyArea

			//lv->paintEmptyArea( p, QRect( 0, 0, width, height() ) );
			QStyleOption opt( lv->sortColumn(), 0 ); // ### hack; in 3.1, add a property in QListView and QHeader
			QStyle::SFlags how = QStyle::Style_Default;
			if ( lv->isEnabled() )
				how |= QStyle::Style_Enabled;

			lv->style().drawComplexControl( QStyle::CC_ListView,
						p, lv, QRect( 0, 0, width, height() ), lv->colorGroup(),
						how, QStyle::SC_ListView, QStyle::SC_None,
						opt );
		}

		if ( isSelected() &&
		(column == 0 || lv->allColumnsShowFocus()) ) {
			p->fillRect( r - marg, 0, width - r + marg, height(),
					_cg.brush( QColorGroup::Highlight ) );
	// removed text pen setting code from Qt
		}

		// removed icon drawing code from Qt

		// draw the tree gubbins
		if ( multiLinesEnabled() && column == 0 && isOpen() && childCount() ) {
			int textheight = fm.size( align, t ).height() + 2 * lv->itemMargin();
			textheight = qMax( textheight, QApplication::globalStrut().height() );
			if ( textheight % 2 > 0 )
				textheight++;
			if ( textheight < height() ) {
				int w = lv->treeStepSize() / 2;
				lv->style().drawComplexControl( QStyle::CC_ListView, p, lv,
								QRect( 0, textheight, w + 1, height() - textheight + 1 ), _cg,
								lv->isEnabled() ? QStyle::Style_Enabled : QStyle::Style_Default,
								QStyle::SC_ListViewExpand,
								(uint)QStyle::SC_All, QStyleOption( this ) );
			}
		}
	}
	// END OF PASTE

	//do you see a better way to tell the TextComponent we are selected ?  - Olivier 2004-09-02
	if ( isSelected() )
		_cg.setColor(QColorGroup::Text , _cg.highlightedText() );

	Q3SimpleRichText myrichtext( text(column), paint.font() );
	myrichtext.draw(  &paint, 0, 0, paint.window(), _cg );

	paint.end();
	p->drawPixmap( 0, 0, back );
}
#endif

ChannelList::ChannelList( QWidget* parent, KIRC::Client *client )
    : QWidget( parent ), m_client( client )
{
	ChannelListLayout = new Q3VBoxLayout( this, 11, 6, "ChannelListLayout");

	layout72_2 = new Q3HBoxLayout( 0, 0, 6, "layout72_2");

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

	mChannelList = new K3ListView( this );
	mChannelList->addColumn( i18n( "Channel" ) );
	mChannelList->addColumn( i18n( "Users" ) );
	//mChannelList->header()->setResizeEnabled( false, mChannelList->header()->count() - 1 );
	mChannelList->addColumn( i18n( "Topic" ) );
	mChannelList->setAllColumnsShowFocus( true );
	mChannelList->setShowSortIndicator( true );

	ChannelListLayout->addWidget( mChannelList );

	//clearWState( WState_Polished );

	textLabel1_2->setText( i18n( "Search for:" ) );
	textLabel1_2->setToolTip( i18n( "You may search for channels on the IRC server, using a text string entered here." ) );
	numUsers->setToolTip( i18n( "Channels returned must have at least this many members." ) );
	numUsers->setWhatsThis( i18n( "Channels returned must have at least this many members." ) );
	textLabel1_2->setWhatsThis( i18n( "You may search for channels on the IRC server, using a text string entered here.  For instance, you may type 'linux' to find channels that have something to do with Linux." ) );
	channelSearch->setToolTip( i18n( "You may search for channels on the IRC server for a text string entered here." ) );
	channelSearch->setWhatsThis( i18n( "You may search for channels on the IRC server, using a text string entered here.  For instance, you may type 'linux' to find channels that have something to do with Linux." ) );
	mSearchButton->setText( i18n( "S&earch" ) );
	mSearchButton->setToolTip( i18n( "Perform a channel search." ) );
	mSearchButton->setWhatsThis( i18n( "Perform a channel search.  Please be patient, as this can be slow, depending on the total number of channels on the server." ) );

	mChannelList->setToolTip( i18n( "Double click on a channel to select it." ) );
	mChannelList->header()->setLabel( 0, i18n( "Channel" ) );
	mChannelList->header()->setLabel( 1, i18n( "Users" ) );
	mChannelList->header()->setLabel( 2, i18n( "Topic" ) );

	// signals and slots connections
	connect( mChannelList, SIGNAL(doubleClicked(Q3ListViewItem*)),
		this, SLOT(slotItemDoubleClicked(Q3ListViewItem*)) );

	connect( mSearchButton, SIGNAL(clicked()), this, SLOT(search()) );

	connect( mChannelList, SIGNAL(selectionChanged(Q3ListViewItem*)), this,
		SLOT(slotItemSelected(Q3ListViewItem*)) );

	/*
	connect( m_engine, SIGNAL(incomingListedChan(QString,uint,QString)),
		this, SLOT(slotChannelListed(QString,uint,QString)) );

	connect( m_engine, SIGNAL(incomingEndOfList()), this, SLOT(slotListEnd()) );

	connect( m_engine, SIGNAL(connectedToServer()), this, SLOT(reset()) );
	connect( m_engine, SIGNAL(disconnected()), this, SLOT(slotDisconnected()) );
	*/

	show();
}

void ChannelList::slotItemDoubleClicked( Q3ListViewItem *i )
{
	emit channelDoubleClicked( i->text(0) );
}

void ChannelList::slotItemSelected( Q3ListViewItem *i )
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
	kDebug(14120) ;

	if( m_client->connectionState() == KIRC::Socket::Open || !channelCache.isEmpty() )
	{
		mChannelList->clear();
		mChannelList->setSorting( -1 );
		mSearchButton->setEnabled(false);
		mSearch = channelSearch->text();
		mSearching = true;
		mUsers = numUsers->value();

		if( channelCache.isEmpty() ) {
			// FIXME
			//m_client->list();
		}
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
		//new ChannelListItem( mChannelList, channel, QString::number(users), topic );
	}
}

void ChannelList::slotSearchCache()
{
	if( cacheIterator != channelCache.end() )
	{
		checkSearchResult( cacheIterator.key(), cacheIterator.data().first, cacheIterator.data().second );
		++cacheIterator;
		QTimer::singleShot( 0, this, SLOT(slotSearchCache()) );
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

