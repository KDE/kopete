/*
    wlmchatsessioninkarea.cpp - Kopete Wlm Protocol

    Copyright (c) 2009      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "wlmchatsessioninkarea.h"

#include <QPaintDevice>
#include <QPainter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPolygon>
#include <QPushButton>
#include <QBitmap>
#include <QColorDialog>
#include <QSlider>

#include <KLocale>

WlmChatSessionInkArea::WlmChatSessionInkArea(QWidget * parent)
: QWidget(parent), 
  m_pen(Qt::black, 3), 
  m_buffer(350, 100), 
  mousePressed( FALSE )
{
    setFixedSize(350,100);
    m_buffer.fill(Qt::white);
}

void WlmChatSessionInkArea::slotChangePenSize(int size)
{
    m_pen.setWidth(size);
}

void WlmChatSessionInkArea::slotClear()
{
    m_polyline.clear();
    m_buffer.fill(Qt::white);
    update();
}

void WlmChatSessionInkArea::slotColor()
{
    m_pen.setColor(QColorDialog::getColor(m_pen.color(), this, i18n("Select the pen's color")));
    emit raiseInkWindow();
}

void WlmChatSessionInkArea::slotSend()
{
    QRect r = QRegion(QBitmap::fromImage(m_buffer.toImage())).boundingRect(); 
    QPixmap buffer_tmp = m_buffer.copy(r);
    emit sendInk(buffer_tmp);
    slotClear();
    if ( isVisible() && parentWidget() &&
        parentWidget()->inherits("QMenu") )
    {
        parentWidget()->close();
    }
}

void WlmChatSessionInkArea::paintEvent(QPaintEvent *)
{
    QPainter painter(&m_buffer);
    painter.setPen( m_pen );
    painter.drawPolyline(m_polyline);

    QPainter paint(this);
    paint.drawPixmap(0, 0, 350, 100, m_buffer);
}

void WlmChatSessionInkArea::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    m_polyline << e->pos();
    update();
}

void WlmChatSessionInkArea::mouseReleaseEvent( QMouseEvent * )
{
    mousePressed = FALSE;
    m_polyline.clear();
    update();
}

void WlmChatSessionInkArea::mouseMoveEvent( QMouseEvent *e )
{
    if ( mousePressed ) {
	m_polyline << e->pos();
    	update();
    }
}

WlmChatSessionInkArea::~WlmChatSessionInkArea()
{
}

#include "wlmchatsessioninkarea.moc"
