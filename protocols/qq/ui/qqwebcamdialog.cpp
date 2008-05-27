/*
    Kopete QQ Protocol

    Copyright (c) 2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>
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

#include "qqwebcamdialog.h"
#include <webcamwidget.h>
#include "avdevice/videodevicepool.h"

#include <qobject.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QPixmap>
#include <QVBoxLayout>
#include <kdebug.h>
#include <klocale.h>

QQWebcamDialog::QQWebcamDialog( const QString &contactId, QWidget * parent )
: KDialog( parent )
{
	setCaption( i18n( "Webcam for %1", contactId ) );
	//setButtons( KDialog::Close );
	setDefaultButton( KDialog::Close );
	// showButtonSeparator( true );
	setWindowFlags( Qt::WDestructiveClose );

	setInitialSize( QSize(320,290) );
	
	setEscapeButton( KDialog::Close );
//	QObject::connect( this, SIGNAL( closeClicked() ), this, SIGNAL( closingWebcamDialog() ) );

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	QVBoxLayout *topLayout = new QVBoxLayout( page );	
	mImageContainer = new Kopete::WebcamWidget( page );
	mImageContainer->setMinimumSize(320,240);
	mImageContainer->setText( i18n( "No webcam image received" ) );
	mImageContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	topLayout->addWidget( mImageContainer );
	
	show();

#ifndef Q_OS_WIN
	mVideoDevicePool = Kopete::AV::VideoDevicePool::self();
	mVideoDevicePool->open();
	mVideoDevicePool->setSize(320, 240);
	mVideoDevicePool->startCapturing();
	mVideoDevicePool->getFrame();
	mVideoDevicePool->getImage(&mImage);
	kDebug() << "Just captured 1st frame";

	mPixmap = QPixmap::fromImage(mImage);
	if (!mPixmap.isNull())
		mImageContainer->updatePixmap(mPixmap);
#endif
	connect(&qtimer, SIGNAL(timeout()), this, SLOT(slotUpdateImage()) );
	qtimer.start(0);
}

QQWebcamDialog::~ QQWebcamDialog( )
{
#ifndef Q_OS_WIN
	mVideoDevicePool->stopCapturing();
	mVideoDevicePool->close();
#endif
}

void QQWebcamDialog::slotUpdateImage()
{
#ifndef Q_OS_WIN
	mVideoDevicePool->getFrame();
	kDebug() << "Getting image";
	mVideoDevicePool->getImage(&mImage);
	kDebug() << "BitBlitting image";
	mImageContainer->updatePixmap( QPixmap::fromImage( mImage ) );
#endif
}


#include "qqwebcamdialog.moc"
