/***************************************************************************
                          kopetecontact.cpp  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kopetecontact.h"
#include "kopete.h"

#include <qtimer.h>
#include <klistview.h>
#include <kdebug.h>

KopeteContact::KopeteContact(QListViewItem *parent)
	: QObject(),
	KListViewItem(parent)
{
	blinkTimer = new QTimer();
	numBlinks=0;
}

KopeteContact::KopeteContact(QListView *parent)
	: QObject(),
	KListViewItem(parent)
{
	blinkTimer = new QTimer();
	numBlinks=0;
}

KopeteContact::~KopeteContact()
{
}

void KopeteContact::setHidden( bool hideme )
{
//	kdDebug() << "KopeteContact::setHidden for user " << text(0) << endl;
	setVisible( !hideme );
}

void KopeteContact::triggerNotify()
{
	numBlinks=0;
	connect ( blinkTimer, SIGNAL(timeout()), this, SLOT(slotNewItemNotify()) );
	blinkTimer->start(1000, false);
	kdDebug() << "notify on" << endl;
}

void KopeteContact::slotNotify(void)
{
	numBlinks++;
	repaint();
	if ( numBlinks == 5 )
	{
		kdDebug() << "notify off" << endl;
		blinkTimer->stop();
		blinkTimer->disconnect();
		numBlinks = 0;
	}
}

void KopeteContact::paintCell (QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
	QColorGroup myColor = QColorGroup (cg);

	if (numBlinks != 0)
	{
		kdDebug() << "numBlinks=" << numBlinks << endl;
		switch ( numBlinks )
		{
			case 1:
			case 3:
			case 5:
				myColor.setColor( QColorGroup::Base,myColor.highlight() );
				break;
		}
	}

	KListViewItem::paintCell (p, myColor, column, width, alignment);
}


bool KopeteContact::isHidden()
{
	return (!isVisible());
}

void KopeteContact::setName( const QString &name )
{
	mName = name;
	setText ( 0, name );
}

QString KopeteContact::name() const
{
	return mName;
}

#include "kopetecontact.moc"
