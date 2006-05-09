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
#include "avdevice/videodevicepool.h"

#include <qframe.h>
#include <qobject.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <kdebug.h>
#include <klocale.h>

TestbedWebcamDialog::TestbedWebcamDialog( const QString &contactId, QWidget * parent, const char * name )
: KDialogBase( KDialogBase::Plain, Qt::WDestructiveClose, parent, name, false, i18n( "Webcam for %1" ).arg( contactId ),
                   KDialogBase::Close, KDialogBase::Close, true /*seperator*/ )
{
	setInitialSize( QSize(320,290), false );
	
	setEscapeButton( KDialogBase::Close );
//	QObject::connect( this, SIGNAL( closeClicked() ), this, SIGNAL( closingWebcamDialog() ) );

	QWidget *page = plainPage();
	setMainWidget(page);

	QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );	
	mImageContainer = new QLabel( page );
	mImageContainer->setText( i18n( "No webcam image received" ) );
	mImageContainer->setAlignment( Qt::AlignCenter );
	mImageContainer->setMinimumSize(320,240);
	mImageContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	topLayout->add( mImageContainer );
	
	mViewer = new QLabel( page );
	mViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	mViewer->hide();
	topLayout->add( mViewer );

	show();
	
	mVideoDevicePool = Kopete::AV::VideoDevicePool::self();
	mVideoDevicePool->open();
	mVideoDevicePool->setSize(320, 240);
	mVideoDevicePool->startCapturing();
	mVideoDevicePool->getFrame();
	mVideoDevicePool->getImage(&mImage);
kdDebug() << "Just captured 1st frame" << endl;

	mPixmap=QPixmap(320,240,-1, QPixmap::DefaultOptim);
	if (mPixmap.convertFromImage(mImage,0) == true)
		mImageContainer->setPixmap(mPixmap);
	connect(&qtimer, SIGNAL(timeout()), this, SLOT(slotUpdateImage()) );
	qtimer.start(10000,FALSE);
}

TestbedWebcamDialog::~ TestbedWebcamDialog( )
{
	mVideoDevicePool->stopCapturing();
	mVideoDevicePool->close();
}

void TestbedWebcamDialog::slotUpdateImage()
{
	mVideoDevicePool->getFrame();
kdDebug() << "Getting image" << endl;
	mVideoDevicePool->getImage(&mImage);
kdDebug() << "BitBlitting image" << endl;
	mImageContainer->clear();
	bitBlt(mImageContainer, 0, 0, &mImage, 0, Qt::CopyROP);
kdDebug() << "Showing image" << endl;

	mImageContainer->clear();
//	bitBlt(m_imageContainer, 0, 0, &image, 0, Qt::CopyROP);
	show();
/* QPixmap pm;
 // grab the image here into pm
 newImage( pm );*/
// QTimer::singleShot( 0, this, SLOT(generateNewImage()) );
}


#include "testbedwebcamdialog.moc"
