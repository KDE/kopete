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
	setEscapeButton( KDialogBase::Close );

	QObject::connect( contact, SIGNAL( receivedWebcamImage( const QPixmap&  ) ),
	                  this, SLOT( newImage( const QPixmap& ) ) );
	QObject::connect( contact, SIGNAL( webcamClosed( int ) ), this, SLOT( webcamClosed( int ) ) );
	
	QFrame* page = plainPage();
	if ( page )
	{
		kdDebug(14180) << k_funcinfo << "Adding webcam image container" << endl;
		m_imageContainer.setText( i18n( "No webcam image received" ) );
		m_imageContainer.setAlignment( Qt::AlignCenter );
	}
}

YahooWebcamDialog::~ YahooWebcamDialog( )
{
}

void YahooWebcamDialog::newImage( const QPixmap & image )
{
	m_imageContainer.setPixmap( image );
}

void YahooWebcamDialog::webcamClosed( int reason  )
{
	kdDebug(14180) << k_funcinfo << "webcam closed with reason " <<  reason <<endl;
	m_imageContainer.setText( i18n( "Webcam closed with reason %1" ).arg( QString::number( reason ) ) );
	m_imageContainer.setAlignment( Qt::AlignCenter );
}

// kate: indent-mode csands; tab-width 4;
