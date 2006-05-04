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
	QObject::connect( this, SIGNAL( closeClicked() ), this, SIGNAL( closingWebcamDialog() ) );

	contactName = contactId;
	QWidget *page = plainPage();
	setMainWidget(page);

	QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );	
	m_imageContainer = new QLabel( page );
	m_imageContainer->setText( i18n( "No webcam image received" ) );
	m_imageContainer->setAlignment( Qt::AlignCenter );
	m_imageContainer->setMinimumSize(320,240);
	m_imageContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	topLayout->add( m_imageContainer );
	
	m_Viewer = new QLabel( page );
	m_Viewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_Viewer->hide();
	topLayout->add( m_Viewer );

	show();
	
	m_videoDevicePool = Kopete::AV::VideoDevicePool::self();
	m_videoDevicePool->open();
	m_videoDevicePool->setSize(320, 240);
	m_videoDevicePool->startCapturing();
}

TestbedWebcamDialog::~ TestbedWebcamDialog( )
{
	kdDebug(14210) << k_funcinfo << endl;

	m_videoDevicePool->stopCapturing(); 
	m_videoDevicePool->close();
}

void TestbedWebcamDialog::newImage( const QPixmap &image )
{
	m_imageContainer->clear();
	bitBlt(m_imageContainer, 0, 0, &image, 0, Qt::CopyROP);
	show();
}


#include "testbedwebcamdialog.moc"
