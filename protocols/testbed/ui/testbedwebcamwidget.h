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

#ifndef TESTBEDWEBCAMWIDGET_H 
#define TESTBEDWEBCAMWIDGET_H 

#include <qwidget.h>
#include <qpixmap.h>

class TestbedWebcamWidget : public QWidget
{
Q_OBJECT
public:
	TestbedWebcamWidget( QWidget* parent = 0, const char* name = 0 );
	~TestbedWebcamWidget();
	void updatePixmap(const QPixmap& pixmap);
protected slots:
	void paintEvent( QPaintEvent* event );
	QPixmap mPixmap;
};

#endif
