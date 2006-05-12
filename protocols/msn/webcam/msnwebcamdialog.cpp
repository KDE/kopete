/*
    Kopete MSN Protocol
	Copyright (c) 2005 by Olivier Goffart            <ogoffart @kde.org>
      
	Note:  this is just YahooWebcamDialog with s/Yahoo/MSN/g  

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

#include "msnwebcamdialog.h"

#include <qframe.h>
#include <qobject.h>
#include <qwidget.h>
#include <kdebug.h>
#include <klocale.h>



MSNWebcamDialog::MSNWebcamDialog( const QString& contact, QWidget * parent, const char * name )
	: KDialogBase( KDialogBase::Plain, i18n( "Webcam for %1" ).arg( contact ),
                   KDialogBase::Close, KDialogBase::Close, parent, name, false, true /*seperator*/ ),
	m_imageContainer( this )
{
	setInitialSize( QSize(320,290), true );
	
	setEscapeButton( KDialogBase::Close );
	/*
	QObject::connect( contact, SIGNAL( signalReceivedWebcamImage( const QPixmap&  ) ),
	                  this, SLOT( newImage( const QPixmap& ) ) );
	*/
	QObject::connect( this, SIGNAL( closeClicked() ), this, SIGNAL( closingWebcamDialog() ) );
	/*
	QObject::connect( contact, SIGNAL( webcamClosed( int ) ), this, SLOT( webcamClosed( int ) ) );
	*/
	QFrame* page = plainPage();
	if ( page )
	{
		kdDebug(14180) << k_funcinfo << "Adding webcam image container" << endl;
		//m_imageContainer.setText( i18n( "No webcam image received" ) );
		//m_imageContainer.setAlignment( Qt::AlignCenter );
		m_imageContainer.setMinimumSize(320,240);
	}
	show();
}

MSNWebcamDialog::~ MSNWebcamDialog( )
{

}

void MSNWebcamDialog::newImage( const QPixmap & image )
{
	kdDebug(14180) << k_funcinfo << "New image received" << endl;
	//	kdDebug(14180) << image << endl;
	//m_imageContainer.clear();
	m_imageContainer.updatePixmap( image );
	//show();
}

void MSNWebcamDialog::webcamClosed( int reason  )
{
	kdDebug(14180) << k_funcinfo << "webcam closed with reason?? " <<  reason <<endl;
	//m_imageContainer.clear();
	//m_imageContainer.setText( i18n( "Webcam closed with reason %1" ).arg( QString::number( reason ) ) );
	//m_imageContainer.setAlignment( Qt::AlignCenter );
	//show();
}

// kate: indent-mode csands; tab-width 4;

#include "msnwebcamdialog.moc"
