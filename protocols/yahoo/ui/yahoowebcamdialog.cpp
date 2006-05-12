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

#include <qframe.h>
#include <qobject.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <kdebug.h>
#include <klocale.h>

#include <webcamwidget.h>

YahooWebcamDialog::YahooWebcamDialog( const QString &contactId, QWidget * parent, const char * name )
: KDialogBase( KDialogBase::Plain, i18n( "Webcam for %1" ).arg( contactId ),
                   KDialogBase::Close, KDialogBase::Close, parent, name, false, true /*seperator*/ )
{
	setInitialSize( QSize(320,290), false );
	
	setEscapeButton( KDialogBase::Close );
	QObject::connect( this, SIGNAL( closeClicked() ), this, SIGNAL( closingWebcamDialog() ) );

	contactName = contactId;
	QWidget *page = plainPage();
	setMainWidget(page);

	QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );	
	m_imageContainer = new Kopete::WebcamWidget( page );
	m_imageContainer->setText( i18n( "No webcam image received" ) );
	m_imageContainer->setMinimumSize(320,240);
	m_imageContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	topLayout->add( m_imageContainer );
	
	m_Viewer = new QLabel( page );
	m_Viewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_Viewer->hide();
	topLayout->add( m_Viewer );

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
	m_imageContainer->setText( QString::fromLatin1("*** Webcam paused ***") );
}

void YahooWebcamDialog::webcamClosed( int reason  )
{
	kdDebug(14180) << k_funcinfo << "webcam closed with reason?? " <<  reason <<endl;
	QString closeReason;
	switch ( reason )
	{
	case 1:
		closeReason = i18n( "%1 has stopped broadcasting" ).arg( contactName ); break;
	case 2:
		closeReason = i18n( "%1 has cancelled viewing permission" ).arg( contactName ); break;
	case 3:
		closeReason = i18n( "%1 has declined permission to view webcam" ).arg( contactName ); break;
	case 4:
		closeReason = i18n( "%1 does not have his/her webcam online" ).arg( contactName ); break;
	default:
		closeReason = i18n( "Unable to view the webcam of %1 for an unknown reason" ).arg( contactName);
	}
	m_imageContainer->clear();

	m_imageContainer->setText( closeReason );
}

void YahooWebcamDialog::setViewer( const QStringList &viewer )
{
	QString s = i18n( "%1 viewer(s)" ).arg( viewer.size() );
	if( viewer.size() )
	{
		s += ": ";
		for ( QStringList::ConstIterator it = viewer.begin(); it != viewer.end(); ++it ) {
			if( it != viewer.begin() )
				s += ", ";
			s += *it;
		}
	}
	m_Viewer->setText( s );
	m_Viewer->show();
}

// kate: indent-mode csands; tab-width 4;

#include "yahoowebcamdialog.moc"
