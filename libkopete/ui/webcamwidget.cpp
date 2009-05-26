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

#include <QColor>
#include <QPainter>
#include <QPaintEvent>

#include <kdebug.h>
namespace Kopete
{

WebcamWidget::WebcamWidget(QWidget* parent)
: QWidget(parent)
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

	update();
}

void WebcamWidget::clear()
{
	mText = "";
	if (!mPixmap.isNull())
		mPixmap = QPixmap(0,0);
	
	update();
}

void WebcamWidget::setText(const QString& text)
{
	mText = text;

	// for now redraw everything
	update();
}

void WebcamWidget::paintEvent( QPaintEvent* event )
{
	QVector<QRect> rects = event->region().rects();

	QPainter p(this);
	if (!mPixmap.isNull())
	{
		for (int i = 0; i < rects.count(); ++i)
		{
			p.drawPixmap(rects[i], mPixmap, rects[i]);
		}
	}
	else
	{
		for (int i = 0; i < rects.count(); ++i)
		{
			QColor bgColor = palette().color(QPalette::Background);
			p.fillRect(rects[i], bgColor);
		}
	}

	// TODO: draw the text
	QRect r = p.boundingRect(rect(), Qt::AlignCenter | Qt::TextWordWrap, mText );
	if ( !mText.isEmpty() && event->rect().intersects(r))
	{
		p.setPen(Qt::black);
		QRect rec = rect();
		rec.moveTopLeft(QPoint(1,1));
		p.drawText(rec, Qt::AlignCenter | Qt::TextWordWrap, mText); 

		rec.moveTopLeft(QPoint(-1,-1));
		p.setPen(Qt::white);
		p.drawText(rec, Qt::AlignCenter | Qt::TextWordWrap, mText); 
	}
	p.end();
}

} // end namespace Kopete

#include "webcamwidget.moc"
