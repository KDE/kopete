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
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	theAccount = account;
	theDialog = 0L;
	origImg = new KTempFile();
	convertedImg = new KTempFile();
	m_img = new QImage();

	m_sendTimer = new QTimer( this );
	connect( m_sendTimer, SIGNAL(timeout()), this, SLOT(sendImage()) );	

	m_updateTimer = new QTimer( this );
	connect( m_updateTimer, SIGNAL(timeout()), this, SLOT(updateImage()) );	

	theDialog = new YahooWebcamDialog( "YahooWebcam" );
	connect( theDialog, SIGNAL(closingWebcamDialog()), this, SLOT(webcamDialogClosing()) );

	m_devicePool = Kopete::AV::VideoDevicePool::self();
	m_devicePool->open();
	m_devicePool->setSize(320, 240);
	m_devicePool->startCapturing();
	m_updateTimer->start( 250 );
}

YahooWebcam::~YahooWebcam()
{
	QFile::remove( origImg->name() );
	QFile::remove( convertedImg->name() );
	delete origImg;
	delete convertedImg;
	delete m_img;
}

void YahooWebcam::stopTransmission()
{
	m_sendTimer->stop();
}

void YahooWebcam::startTransmission()
{
	m_sendTimer->start( 1000 );
}

void YahooWebcam::webcamDialogClosing()
{
	m_sendTimer->stop();
	theDialog->delayedDestruct();
	emit webcamClosing();
	m_devicePool->stopCapturing(); 
	m_devicePool->close();
}

void YahooWebcam::updateImage()
{
	m_devicePool->getFrame();
	m_devicePool->getImage(m_img);
	theDialog->newImage( *m_img );
}

void YahooWebcam::sendImage()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	m_devicePool->getFrame();
	m_devicePool->getImage(m_img);
	
	origImg->close();
	convertedImg->close();
	
	m_img->save( origImg->name(), "JPEG");
	
	KProcess p;
	p << "jasper";
	p << "--input" << origImg->name() << "--output" << convertedImg->name() << "--output-format" << "jpc" << "-O" <<"cblkwidth=64\ncblkheight=64\nnumrlvls=4\nrate=0.0165\nprcheight=128\nprcwidth=2048\nmode=real";
	
	
	p.start( KProcess::Block );
	if( p.exitStatus() != 0 )
	{
		kdDebug(YAHOO_GEN_DEBUG) << " jasper exited with status " << p.exitStatus() << endl;
	}
	else
	{
		QFile file( convertedImg->name() );
		if( file.open( IO_ReadOnly ) )
		{
			QByteArray ar = file.readAll();
			theAccount->yahooSession()->sendWebcamImage( ar );
		}
		else
			kdDebug(YAHOO_GEN_DEBUG) << "Error opening the converted webcam image." << endl;
	}
}

void YahooWebcam::addViewer( const QString &viewer )
{
	m_viewer.push_back( viewer );
	if( theDialog )
		theDialog->setViewer( m_viewer );
}

void YahooWebcam::removeViewer( const QString &viewer )
{
	m_viewer.remove( viewer );
	if( theDialog )
		theDialog->setViewer( m_viewer );
}

#include "yahoowebcam.moc"
