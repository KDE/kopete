/*
    yahoowebcam.cpp - Send webcam images

    Copyright (c) 2005 by André Duffec <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "yahoowebcam.h"

#include "yahoo_protocol_debug.h"
#include <qtimer.h>

#include "client.h"
#include "yahooaccount.h"
#include "yahoowebcamdialog.h"
#ifndef VIDEOSUPPORT_DISABLED
#include "avdevice/videodevicepool.h"
#endif

#include "webcamimgformat.h"

YahooWebcam::YahooWebcam( YahooAccount *account ) : QObject( 0 )
{
	setObjectName( QStringLiteral("yahoo_webcam") );
	qCDebug(YAHOO_PROTOCOL_LOG) ;
	theAccount = account;
	theDialog = 0L;
	m_img = new QImage();

	m_sendTimer = new QTimer( this );
	connect( m_sendTimer, SIGNAL(timeout()), this, SLOT(sendImage()) );	

	m_updateTimer = new QTimer( this );
	connect( m_updateTimer, SIGNAL(timeout()), this, SLOT(updateImage()) );	

	theDialog = new YahooWebcamDialog( QStringLiteral("YahooWebcam") );
	connect( theDialog, SIGNAL(closingWebcamDialog()), this, SLOT(webcamDialogClosing()) );
#ifndef VIDEOSUPPORT_DISABLED
	m_devicePool = Kopete::AV::VideoDevicePool::self();
	m_devicePool->open();
	m_devicePool->setImageSize(320, 240);
	m_devicePool->startCapturing();
	m_updateTimer->start( 250 );
#endif
}

YahooWebcam::~YahooWebcam()
{
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
#ifndef VIDEOSUPPORT_DISABLED
	m_devicePool->stopCapturing(); 
	m_devicePool->close();
#endif
}

void YahooWebcam::updateImage()
{
#ifndef VIDEOSUPPORT_DISABLED
	if (EXIT_SUCCESS == m_devicePool->getFrame())
	{
		m_devicePool->getImage(m_img);
		theDialog->newImage( QPixmap::fromImage(*m_img) );
	}
#endif
}

void YahooWebcam::sendImage()
{
	qCDebug(YAHOO_PROTOCOL_LOG) ;

#ifndef VIDEOSUPPORT_DISABLED
	if (EXIT_SUCCESS == m_devicePool->getFrame())
		m_devicePool->getImage(m_img);
	/* NOTE: not sure if we can skip sending an image without running into trouble...
	         => send last image
	 */
#endif

	QByteArray result;
	if (WebcamImgFormat::instance())
	{
		if (WebcamImgFormat::instance()->forYahoo(result, m_img))
		{
			qCDebug(YAHOO_PROTOCOL_LOG) << "Image successfully converted";
			theAccount->yahooSession()->sendWebcamImage(result);
		} else
			qCDebug(YAHOO_PROTOCOL_LOG) << "Failed to convert outgoing Yahoo webcam image";
	} else
		qCDebug(YAHOO_PROTOCOL_LOG) << "Failed to initialize WebcamImgFormat helper";
}

void YahooWebcam::addViewer( const QString &viewer )
{
	m_viewer.push_back( viewer );
	if( theDialog )
		theDialog->setViewer( m_viewer );
}

void YahooWebcam::removeViewer( const QString &viewer )
{
	m_viewer.removeAll( viewer );
	if( theDialog )
		theDialog->setViewer( m_viewer );
}

