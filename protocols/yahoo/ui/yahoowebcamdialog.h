/*
    Kopete Yahoo Protocol

    Copyright (c) 2005 by Matt Rogers                 <mattr@kde.org>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOWEBCAMDIALOG_H_
#define YAHOOWEBCAMDIALOG_H_

#include <qlabel.h>
#include <kdialogbase.h>


class QPixmap;
class QWidget;
class YahooContact;

class YahooWebcamDialog : public KDialogBase
{
Q_OBJECT
public:
	YahooWebcamDialog( YahooContact*, QWidget* parent = 0, const char* name = 0 );
	~YahooWebcamDialog();
	
public slots:
	void newImage( const QPixmap& image );
	void webcamClosed( int );

signals:
	void closingWebcamDialog();
	
private:
	QLabel m_imageContainer;
	
};

#endif
//kate: indent-mode csands; auto-insert-doxygen on;
