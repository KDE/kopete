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


#include <kdialog.h>


class QPixmap;
class QLabel;
class QWidget;

namespace Kopete
{
	class WebcamWidget;
}

class YahooWebcamDialog : public KDialog
{
Q_OBJECT
public:
	explicit YahooWebcamDialog( const QString &, QWidget* parent = 0 );
	~YahooWebcamDialog();
	
	void setViewer( const QStringList & );
public slots:
	void newImage( const QPixmap &image );
	void webcamClosed( int );
	void webcamPaused();
signals:
	void closingWebcamDialog();
	
private:
	Kopete::WebcamWidget *m_imageContainer;
	QLabel *m_Viewer;
	QString contactName;
	
};

#endif
//kate: indent-mode csands; auto-insert-doxygen on;
