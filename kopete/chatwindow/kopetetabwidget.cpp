/*
    kopetetabwidget.cpp - KDE Tab Widget

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qpainter.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include "kopetetabwidget.h"

class KopeteTabBarPrivate {
	public:
		QMap<int,QColor> tabColorMap;
};

KopeteTabWidget::KopeteTabWidget( QWidget *parent ) : QTabWidget( parent )
{
	setTabBar( static_cast<QTabBar*>( new KopeteTabBar( this ) ) );
}

void KopeteTabWidget::setLabelTextColor( QWidget *w, const QColor &color ) const
{
	int tabIndex = indexOf( w );
	static_cast<KopeteTabBar*>( tabBar() )->setLabelTextColor( tabIndex, color );
}

KopeteTabBar::KopeteTabBar( KopeteTabWidget *parent ) : QTabBar( parent )
{
	d = new KopeteTabBarPrivate;
	connect( this, SIGNAL( contextMenu( QWidget *, const QPoint & ) ),
		parent, SIGNAL( contextMenu( QWidget *, const QPoint & ) ) );
}

KopeteTabBar::~KopeteTabBar()
{
	delete d;
}

void KopeteTabBar::contextMenuEvent( QContextMenuEvent *e )
{
	QTab *tab = selectTab( e->pos() );
	if( tab )
	{
		KopeteTabWidget *m_tabWidget = static_cast<KopeteTabWidget*>( parent() );
		QWidget *targetTab = m_tabWidget->page( indexOf( tab->identifier() ) );
		emit( contextMenu( targetTab, mapToGlobal( e->pos() ) ) );
	}
}

void KopeteTabBar::setLabelTextColor( int tabIndex, const QColor &color )
{
	d->tabColorMap[ tabAt( tabIndex )->identifier() ] = color;
	update();
}

void KopeteTabBar::removeTab( QTab *t )
{
	d->tabColorMap.remove( t->identifier() );
	QTabBar::removeTab( t );
}

void KopeteTabBar::paintLabel ( QPainter * p, const QRect & br, QTab * t, bool has_focus ) const
{
	QRect r = br;
	bool selected = ( currentTab() == t->identifier() );
	if ( t->iconSet() )
	{
		// the tab has an iconset, draw it in the right mode
		QIconSet::Mode mode = ( t->isEnabled() ) ? QIconSet::Normal : QIconSet::Disabled;
		if ( mode == QIconSet::Normal && has_focus )
			mode = QIconSet::Active;
		QPixmap pixmap = t->iconSet()->pixmap( QIconSet::Small, mode );
		int pixw = pixmap.width();
		int pixh = pixmap.height();
		r.setLeft( r.left() + pixw + 4 );
		r.setRight( r.right() + 2 );
		// ### the pixmap shift should probably not be hardcoded..
		p->drawPixmap( br.left() + 2 + ((selected == TRUE) ? 0 : 2),
			br.center().y()-pixh/2 + ((selected == TRUE) ? 0 : 2),
			pixmap );
	}

	if( d->tabColorMap.contains( t->identifier() ) )
		p->setPen( d->tabColorMap[ t->identifier() ] );
	else
		p->setPen( KGlobalSettings::textColor() );

	p->drawText( r, AlignCenter, t->text() );
}

#include "kopetetabwidget.moc"
