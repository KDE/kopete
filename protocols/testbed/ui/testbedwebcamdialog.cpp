/*
    Kopete Testbed Protocol

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

#include "testbedwebcamdialog.h"
#include <webcamwidget.h>
#include "avdevice/videodevicepool.h"

#include <QObject>
#include <QWidget>
#include <QPixmap>

#include <Q3VBoxLayout>
#include <q3vbox.h>

#include <kdebug.h>
#include <klocale.h>

TestbedWebcamDialog::TestbedWebcamDialog( const QString &contactId, QWidget * parent )
: KDialog( parent )
{
	setCaption( i18n( "Webcam for %1", contactId ) );
	setButtons( KDialog::Close );
	setDefaultButton( KDialog::Close );
	showButtonSeparator( true );
	setWindowFlags( Qt::WDestructiveClose );

	setInitialSize( QSize(320,290) );
	
	setEscapeButton( KDialog::Close );
//	QObject::connect( this, SIGNAL( closeClicked() ), this, SIGNAL( closingWebcamDialog() ) );

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	Q3VBoxLayout *topLayout = new Q3VBoxLayout( page, 0, spacingHint() );	
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
#endif

	mPixmap=QPixmap::fromImage(mImage);
	if (!mPixmap.isNull())
		mImageContainer->updatePixmap(mPixmap);
	connect(&qtimer, SIGNAL(timeout()), this, SLOT(slotUpdateImage()) );
	qtimer.setSingleShot(false);
	qtimer.start(0);
}

TestbedWebcamDialog::~ TestbedWebcamDialog( )
{
#ifndef Q_OS_WIN
	mVideoDevicePool->stopCapturing();
	mVideoDevicePool->close();
#endif
}

void TestbedWebcamDialog::slotUpdateImage()
{
#ifndef Q_OS_WIN
	mVideoDevicePool->getFrame();
	kDebug() << "Getting image";
	mVideoDevicePool->getImage(&mImage);
	mImageContainer->updatePixmap( QPixmap( mImage ) );
#endif
}


#include "testbedwebcamdialog.moc"
