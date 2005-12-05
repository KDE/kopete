/*
    yahoowebcam.cpp - Send webcam images

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

#include <kdebug.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <qtimer.h>

#include "client.h"
#include "yahoowebcam.h"
#include "yahooaccount.h"
#include "yahoowebcamdialog.h"
#include "avdevice/videodevicepool.h"


YahooWebcam::YahooWebcam( YahooAccount *account ) : QObject( 0, "yahoo_webcam" )
{
	kdDebug(14180) << k_funcinfo << endl;
	theAccount = account;
	theDialog = 0L;
	m_timestamp = 1;

	m_timer = new QTimer();
	connect( m_timer, SIGNAL(timeout()), this, SLOT(sendImage()) );	

	Kopete::AV::VideoDevicePool *videoDevice = Kopete::AV::VideoDevicePool::self();
	videoDevice->scanDevices();
	videoDevice->open(0);
	videoDevice->setSize(320, 240);
	videoDevice->selectInput(0);
	videoDevice->startCapturing();
}

YahooWebcam::~YahooWebcam()
{
	delete m_timer;
}

void YahooWebcam::stopTransmission()
{
	m_timer->stop();
}

void YahooWebcam::startTransmission()
{
	m_timer->start( 1000 );
}

void YahooWebcam::webcamDialogClosing()
{
	m_timer->stop();
	theDialog->delayedDestruct();
	emit webcamClosing();
	Kopete::AV::VideoDevicePool *videoDevice = Kopete::AV::VideoDevicePool::self(); 
	videoDevice->stopCapturing(); 
	videoDevice->close();
}

void YahooWebcam::sendImage()
{
	kdDebug(14180) << k_funcinfo << endl;

	Kopete::AV::VideoDevicePool *videoDevice = Kopete::AV::VideoDevicePool::self();
	videoDevice->getFrame();
	QImage img;
	videoDevice->getImage(&img);
	
	if( !theDialog )
	{
		theDialog = new YahooWebcamDialog( "YahooWebcam" );
		connect( theDialog, SIGNAL(closingWebcamDialog()), this, SLOT(webcamDialogClosing()) );
	}
	
	KTempFile origImg;
	KTempFile convertedImg;
	origImg.close();
	convertedImg.close();
	
	img.save( origImg.name(), "JPEG");
	
	theDialog->newImage( img );
	KProcess p;
	p << "jasper";
	p << "--input" << origImg.name() << "--output" << convertedImg.name() << "--output-format" << "jpc" << "-O" << "rate=0.01" ;
	
	p.start( KProcess::Block );
	if( p.exitStatus() != 0 )
	{
		kdDebug(14181) << " jasper exited with status " << p.exitStatus() << endl;
	}
	else
	{
		QFile file( convertedImg.name() );
		if( file.open( IO_ReadOnly ) )
		{
			QByteArray ar = file.readAll();
			theAccount->yahooSession()->sendWebcamImage( ar, ar.size(), m_timestamp++ );
		}
		else
			kdDebug(14181) << "Error opening the converted webcam image." << endl;
	}
}

#include "yahoowebcam.moc"
