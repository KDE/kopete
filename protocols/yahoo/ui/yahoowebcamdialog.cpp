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

#include "yahoowebcamdialog.h"

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <kdebug.h>
#include <klocale.h>

#include <webcamwidget.h>

YahooWebcamDialog::YahooWebcamDialog( const QString &contactId, QWidget * parent )
: KDialog( parent )
{
	setCaption( i18n( "Webcam for %1", contactId ) );
	setButtons( KDialog::Close );
	setDefaultButton( KDialog::Close );
	showButtonSeparator( true );
	
	setInitialSize( QSize(320,290) );
	
	setEscapeButton( KDialog::Close );
	QObject::connect( this, SIGNAL(closeClicked()), this, SIGNAL(closingWebcamDialog()) );

	contactName = contactId;
	QWidget *page = new QWidget(this);
	setMainWidget(page);

	QVBoxLayout *topLayout = new QVBoxLayout( page );	
	topLayout->addSpacing( spacingHint() );
	m_imageContainer = new Kopete::WebcamWidget( page );
	m_imageContainer->setText( i18n( "No webcam image received" ) );
	m_imageContainer->setMinimumSize(320,240);
	m_imageContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	topLayout->addWidget( m_imageContainer );
	
	m_Viewer = new QLabel( page );
	m_Viewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_Viewer->hide();
	topLayout->addWidget( m_Viewer );

	show();
}

YahooWebcamDialog::~ YahooWebcamDialog( )
{

}

void YahooWebcamDialog::newImage( const QPixmap &image )
{
	m_imageContainer->updatePixmap( image );
}

void YahooWebcamDialog::webcamPaused()
{
	m_imageContainer->setText( QLatin1String("*** Webcam paused ***") );
}

void YahooWebcamDialog::webcamClosed( int reason  )
{
	kDebug(14180) << "webcam closed with reason?? " <<  reason;
	QString closeReason;
	switch ( reason )
	{
	case 1:
		closeReason = i18n( "%1 has stopped broadcasting", contactName ); break;
	case 2:
		closeReason = i18n( "%1 has cancelled viewing permission", contactName ); break;
	case 3:
		closeReason = i18n( "%1 has declined permission to view webcam", contactName ); break;
	case 4:
		closeReason = i18n( "%1 does not have his/her webcam online", contactName ); break;
	default:
		closeReason = i18n( "Unable to view %1's webcam for an unknown reason", contactName);
	}
	m_imageContainer->clear();

	m_imageContainer->setText( closeReason );
}

void YahooWebcamDialog::setViewer( const QStringList &viewer )
{
	QString s = i18np( "1 viewer", "%1 viewers", viewer.size() );
	if( !viewer.empty() )
	{
		QStringList::ConstIterator it = viewer.begin();
		const QStringList::ConstIterator itEnd = viewer.end();
		
		s += ": " + *it++;
		
		for ( ; it != itEnd; ++it ) {
			s += ", " + *it;
		}
	}
	m_Viewer->setText( s );
	m_Viewer->show();
}

// kate: indent-mode csands; tab-width 4;

#include "yahoowebcamdialog.moc"
