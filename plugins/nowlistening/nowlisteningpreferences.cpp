/*
    nowlisteningpreferences.cpp

    Kopete Now Listening To plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "nowlisteningprefs.h"
#include "nowlisteningpreferences.h"
#include "nowlisteningpreferences.moc"

NowListeningPreferences::NowListeningPreferences( const QString &pixmap, QObject *parent ) 
	: ConfigModule( i18n( "Now Listening" ), i18n( "Now Listening Plugin" ), pixmap, parent )
{
	(  new QVBoxLayout(  this ) )->setAutoAdd(  true );
	preferencesDialog = new NowListeningPrefsUI( this );

	KGlobal::config()->setGroup( "Now Listening Plugin" );
	preferencesDialog->m_freq->setValue(
			KGlobal::config()->readNumEntry( "PollFrequency", 90 )
			);
	preferencesDialog->m_message->setText( 
			KGlobal::config()->readEntry( "Message", 
				"Now Listening To: " ) 
			 );
}

NowListeningPreferences::~NowListeningPreferences( )
{
}

void NowListeningPreferences::save()
{
		KConfig *config=KGlobal::config();
		config->setGroup( "Now Listening Plugin" );
		config->writeEntry( "PollFrequency", 
				preferencesDialog->m_freq->value() );
		config->writeEntry( "Message", 
				preferencesDialog->m_message->text() );
		config->sync();
		emit saved();
}

/*
* Local variables:
* c-indentation-style: k&r
* c-basic-offset: 8
* indent-tabs-mode: t
* End:
*/
// vim: set noet ts=4 sts=4 sw=4:
//
