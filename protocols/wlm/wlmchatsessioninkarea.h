/*
    wlmchatsessioninkarea.h - Kopete Wlm Protocol

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
#ifndef WLMCHATSESSIONINKAREA_H
#define WLMCHATSESSIONINKAREA_H

#include <QWidget>
#include <QMouseEvent>
#include <QPolygon>
#include <QPen>

class WlmChatSessionInkArea : public QWidget
{
    Q_OBJECT
    public:
        WlmChatSessionInkArea(QWidget * parent = 0);
        virtual ~WlmChatSessionInkArea();
    private:
        void paintEvent(QPaintEvent * );
    protected:
        void mousePressEvent( QMouseEvent *e );
        void mouseReleaseEvent( QMouseEvent *e );
        void mouseMoveEvent( QMouseEvent *e );
        QPen m_pen;
        QPolygon m_polyline;
        QPixmap m_buffer;

        bool mousePressed;
signals:
        void sendInk( const QPixmap &);
        void raiseInkWindow();
        void closeWindow();
public slots:
        void slotChangePenSize(int size);
        void slotClear();
        void slotSend();
        void slotColor();
};
#endif

