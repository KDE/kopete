/*
    Kopete MSN Protocol

	Copyright (c) 2005 by Olivier Goffart            <ogoffart @kde.org>

	Note:  this is just YahooWebcamDialog with s/Yahoo/MSN/g  

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

//#include <qlabel.h>
#include <webcamwidget.h>
#include <kdialogbase.h>

#include "kopete_export.h"


class QPixmap;
class QWidget;
class MSNContact;

class KOPETE_EXPORT MSNWebcamDialog : public KDialogBase
{
Q_OBJECT
public:
	MSNWebcamDialog( const QString& contact, QWidget* parent = 0, const char* name = 0 );
	~MSNWebcamDialog();
	
public slots:
	void newImage( const QPixmap& image );
	void webcamClosed( int );

signals:
	void closingWebcamDialog();
	
private:
	Kopete::WebcamWidget m_imageContainer;
	
};

#endif
//kate: indent-mode csands; auto-insert-doxygen on;
