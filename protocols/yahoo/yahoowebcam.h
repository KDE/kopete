/*
    yahoowebcam.h - Send webcam images

    Copyright (c) 2005 by André Duffec <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOWEBCAM_H
#define YAHOOWEBCAM_H

#include <qobject.h>

class YahooAccount;
class YahooWebcamDialog;
class QTimer;
class KTempFile;

class YahooWebcam : public QObject
{
	Q_OBJECT
public:
	YahooWebcam( YahooAccount *account );
	~YahooWebcam();
public slots:
	void startTransmission();
	void stopTransmission();
	void sendImage();
	void webcamDialogClosing();
	void addViewer( const QString & );
	void removeViewer( const QString & );
signals:
	void webcamClosing();
private:
	YahooAccount *theAccount;
	YahooWebcamDialog *theDialog;
	QTimer *m_timer;
	QStringList m_viewer;
	KTempFile *origImg;
	KTempFile *convertedImg;
};

#endif
