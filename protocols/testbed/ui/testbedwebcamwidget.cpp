/*
    Kopete Testbed Protocol

    Copyright (c) 2006 by Gustavo Pichorim Boiko   <gustavo.boiko@kdemail.net>
    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "testbedwebcamwidget.h"

TestbedWebcamWidget::TestbedWebcamWidget( QWidget* parent, const char* name )
: QWidget( parent, name )
{
	// do nothing for now
}

TestbedWebcamWidget::~TestbedWebcamWidget()
{
	// don't do anything either
}

void TestbedWebcamWidget::updatePixmap(const QPixmap& pixmap)
{
	mPixmap = pixmap;

	QPaintEvent *ev = new QPaintEvent( rect(), true );
	paintEvent( ev );
	delete ev;
}

void TestbedWebcamWidget::paintEvent( QPaintEvent* event )
{
	QMemArray<QRect> rects = event->region().rects();

	for (int i = 0; i < rects.count(); ++i )
	{
		bitBlt(this, rects[i].topLeft(), &mPixmap, rects[i], Qt::CopyROP, true);
	}
}
