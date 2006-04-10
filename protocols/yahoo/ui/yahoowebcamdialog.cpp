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

#include <q3frame.h>
#include <qobject.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3vbox.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <QPixmap>
#include <kdebug.h>
#include <klocale.h>

YahooWebcamDialog::YahooWebcamDialog( const QString &contactId, QWidget * parent, const char * name )
: KDialogBase( KDialogBase::Plain, i18n( "Webcam for %1", contactId ),
                   KDialogBase::Close, KDialogBase::Close, parent, name, false, true /*seperator*/ )
{
	setInitialSize( QSize(320,290) );
	
	setEscapeButton( KDialogBase::Close );
	QObject::connect( this, SIGNAL( closeClicked() ), this, SIGNAL( closingWebcamDialog() ) );

	contactName = contactId;
	QWidget *page = plainPage();
	setMainWidget(page);

	Q3VBoxLayout *topLayout = new Q3VBoxLayout( page, 0, spacingHint() );	
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
}

YahooWebcamDialog::~ YahooWebcamDialog( )
{

}

void YahooWebcamDialog::newImage( const QPixmap &image )
{
	m_imageContainer->clear();
	m_imageContainer->setPixmap( image );
#warning port bitBlt here correctly.
	//bitBlt(m_imageContainer, 0, 0, &image, 0, Qt::CopyROP);
	show();
}

void YahooWebcamDialog::webcamPaused()
{
	m_imageContainer->clear();
	
	m_imageContainer->setText( QString::fromLatin1("*** Webcam paused ***") );
	m_imageContainer->adjustSize();
	m_imageContainer->setAlignment( Qt::AlignCenter );
	adjustSize();
	show();
}

void YahooWebcamDialog::webcamClosed( int reason  )
{
	kDebug(14180) << k_funcinfo << "webcam closed with reason?? " <<  reason <<endl;
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
		closeReason = i18n( "Unable to view the webcam of %1 for an unknown reason", contactName);
	}
	m_imageContainer->clear();

	m_imageContainer->setText( closeReason );
	m_imageContainer->adjustSize();
	m_imageContainer->setAlignment( Qt::AlignCenter );
	adjustSize();
	show();
}

void YahooWebcamDialog::setViewer( const QStringList &viewer )
{
	QString s = i18n( "%1 viewer(s)", viewer.size() );
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
