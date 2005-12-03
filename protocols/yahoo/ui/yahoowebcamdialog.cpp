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
#include <kdebug.h>
#include <klocale.h>
#include "yahoocontact.h"



YahooWebcamDialog::YahooWebcamDialog( YahooContact* contact, QWidget * parent, const char * name )
	: KDialogBase( KDialogBase::Plain, i18n( "Webcam for %1" ).arg( contact->nickName() ),
                   KDialogBase::Close, KDialogBase::Close, parent, name, false, true /*seperator*/ ),
	m_imageContainer( this )
{
	setInitialSize( QSize(320,290), false );
	
	setEscapeButton( KDialogBase::Close );
	/*
	QObject::connect( contact, SIGNAL( signalReceivedWebcamImage( const QPixmap&  ) ),
	                  this, SLOT( newImage( const QPixmap& ) ) );
	*/
	QObject::connect( this, SIGNAL( closeClicked() ), this, SIGNAL( closingWebcamDialog() ) );
	/*
	QObject::connect( contact, SIGNAL( webcamClosed( int ) ), this, SLOT( webcamClosed( int ) ) );
	*/
	contactName = contact->contactId();
	
	QFrame* page = plainPage();
	if ( page )
	{
		kdDebug(14180) << k_funcinfo << "Adding webcam image container" << endl;
		m_imageContainer.setText( i18n( "No webcam image received" ) );
		m_imageContainer.setAlignment( Qt::AlignCenter );
		m_imageContainer.setMinimumSize(320,240);
	}
	show();
}

YahooWebcamDialog::~ YahooWebcamDialog( )
{

}

void YahooWebcamDialog::newImage( const QPixmap & image )
{
	m_imageContainer.clear();
	m_imageContainer.setPixmap( image );
	show();
}

void YahooWebcamDialog::webcamPaused()
{
	m_imageContainer.clear();
	
	m_imageContainer.setText( QString::fromLatin1("*** Webcam paused ***") );
	m_imageContainer.adjustSize();
	m_imageContainer.setAlignment( Qt::AlignCenter );
	adjustSize();
	show();
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
	m_imageContainer.clear();

	m_imageContainer.setText( closeReason );
	m_imageContainer.adjustSize();
	m_imageContainer.setAlignment( Qt::AlignCenter );
	adjustSize();
	show();
}

// kate: indent-mode csands; tab-width 4;

#include "yahoowebcamdialog.moc"
