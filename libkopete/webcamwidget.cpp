/*
    webcamwidget.h - A simple widget for displaying webcam frames

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

#include "webcamwidget.h"

#include <qcolor.h>
#include <qpainter.h>

#include <kdebug.h>
namespace Kopete
{

WebcamWidget::WebcamWidget( QWidget* parent, const char* name )
: QWidget( parent, name )
{
	clear();
}

WebcamWidget::~WebcamWidget()
{
	// don't do anything either
}

void WebcamWidget::updatePixmap(const QPixmap& pixmap)
{
	mPixmap = pixmap;
	mText = "";

	QPaintEvent *ev = new QPaintEvent( rect(), true );
	paintEvent( ev );
	delete ev;
}

void WebcamWidget::clear()
{
	mText = "";
	if (!mPixmap.isNull())
		mPixmap.resize(0,0);
	
	QPaintEvent *ev = new QPaintEvent( rect(), true );
	paintEvent( ev );
	delete ev;
}

void WebcamWidget::setText(const QString& text)
{
	mText = text;

	// for now redraw everything
	QPaintEvent *ev = new QPaintEvent( rect(), true );
        paintEvent( ev );
        delete ev;
}

void WebcamWidget::paintEvent( QPaintEvent* event )
{
	QMemArray<QRect> rects = event->region().rects();

	if (!mPixmap.isNull())
	{
		for (unsigned int i = 0; i < rects.count(); ++i)
		{
			bitBlt(this, rects[i].topLeft(), &mPixmap, rects[i], Qt::CopyROP, true);
		}
	}
	else
	{
		for (unsigned int i = 0; i < rects.count(); ++i)
		{
			QColor bgColor = paletteBackgroundColor();
			QPainter p(this);
			p.fillRect(rects[i], bgColor);
		}
	}

	// TODO: draw the text
	QPainter p(this);
	QRect r = p.boundingRect(rect(), Qt::AlignCenter | Qt::WordBreak, mText );
	if ( !mText.isEmpty() && event->rect().intersects(r))
	{
		p.setPen(Qt::black);
		QRect rec = rect();
		rec.moveTopLeft(QPoint(1,1));
		p.drawText(rec, Qt::AlignCenter | Qt::WordBreak, mText, -1); 

		rec.moveTopLeft(QPoint(-1,-1));
		p.setPen(Qt::white);
		p.drawText(rec, Qt::AlignCenter | Qt::WordBreak, mText, -1); 
	}
}

} // end namespace Kopete

#include "webcamwidget.moc"
