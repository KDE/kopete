/*
    kopeteballoon.cpp  -  Nice Balloon

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002      by the Kopete developers  <kopete-devel@kde.org>

    Portions of this code based on KSim Applet code
    Copyright (c) 2000-2002 by Malte Starostik        <malte@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qpointarray.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kapplication.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kopeteballoon.h"
#include "systemtray.h"

KopeteBalloon::KopeteBalloon( const QString &text, const QString &pix )
: QWidget( 0L, "KopeteBalloon", WStyle_StaysOnTop | WStyle_Customize |
	WStyle_NoBorder | WStyle_Tool | WX11BypassWM )
{
	resize( 139, 104 );

	setCaption( trUtf8( "" ) );
	QVBoxLayout *BalloonLayout = new QVBoxLayout( this, 24, 6, "BalloonLayout");

	QHBoxLayout *Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1");

	m_image = new QLabel( this, "m_image" );
	m_image->setScaledContents( FALSE );
	Layout1->addWidget( m_image );

	m_caption = new QLabel( this, "m_caption" );
	m_caption->setText( trUtf8( "" ) );
	Layout1->addWidget( m_caption );
	BalloonLayout->addLayout( Layout1 );

	QHBoxLayout *Layout2 = new QHBoxLayout( 0, 0, 6, "Layout2");
	QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	Layout2->addItem( spacer );

	QPushButton *viewButton = new QPushButton( this, "m_reject" );
	viewButton->setText( i18n( "View" ) );

	QPushButton *ignoreButton = new QPushButton( this );
	ignoreButton->setText( i18n( "Ignore" ) );

	Layout2->addWidget( viewButton );
	Layout2->addWidget( ignoreButton );

	QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	Layout2->addItem( spacer_2 );
	BalloonLayout->addLayout( Layout2 );

	setPalette(QToolTip::palette());
	setAutoMask(true);

	m_image->setPixmap(locate("data", pix));
	m_caption->setText(text);

	connect( viewButton, SIGNAL( clicked() ), SIGNAL( signalButtonClicked() ) );
	connect( ignoreButton, SIGNAL( clicked() ), SIGNAL( signalIgnoreButtonClicked() ) );
	connect( viewButton, SIGNAL( clicked() ), SLOT( deleteLater() ) );
	connect( ignoreButton, SIGNAL( clicked() ), SLOT( deleteLater() ) );
}

void KopeteBalloon::setAnchor( const QPoint &anchor)
{
	m_anchor = anchor;
	updateMask();
}

KopeteBalloon::~KopeteBalloon()
{
}

void KopeteBalloon::updateMask()
{
	QRegion mask(10, 10, width() - 20, height() - 20);

	QPoint corners[8] = {
		QPoint(width() - 50, 10),
		QPoint(10, 10),
		QPoint(10, height() - 50),
		QPoint(width() - 50, height() - 50),
		QPoint(width() - 10, 10),
		QPoint(10, 10),
		QPoint(10, height() - 10),
		QPoint(width() - 10, height() - 10)
	};

	for (int i = 0; i < 4; ++i)
	{
		QPointArray corner;
		corner.makeArc(corners[i].x(), corners[i].y(), 40, 40, i * 16 * 90, 16 * 90);
		corner.resize(corner.size() + 1);
		corner.setPoint(corner.size() - 1, corners[i + 4]);
		mask -= corner;
	}

	bool bottom, right;
	QDesktopWidget* tmp = QApplication::desktop();
	QRect deskRect;
	// get screen-geometry for screen our anchor is on
	// (geometry can differ from screen to screen!
	deskRect = tmp->screenGeometry( tmp->screenNumber(m_anchor) );

	bottom = m_anchor.y() + height() > deskRect.height() - 48;
	right = m_anchor.x() + width() > deskRect.width() - 48;

	QPointArray arrow(4);
	arrow.setPoint(0, QPoint(right ? width() : 0, bottom ? height() : 0));
	arrow.setPoint(1, QPoint(right ? width() - 10 : 10, bottom ? height() - 30 : 30));
	arrow.setPoint(2, QPoint(right ? width() - 30 : 30, bottom ? height() - 10 : 10));
	arrow.setPoint(3, arrow[0]);
	mask += arrow;
	setMask(mask);

	move(right ? m_anchor.x() - width() : m_anchor.x(),
	     bottom ? m_anchor.y() - height() : m_anchor.y());
}

#include "kopeteballoon.moc"

// vim: set noet ts=4 sts=4 sw=4:

