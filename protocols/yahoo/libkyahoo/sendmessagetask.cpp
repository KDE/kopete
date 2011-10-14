/*
    Kopete Yahoo Protocol
    Send a message

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "sendmessagetask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <kdebug.h>
#include <klocale.h>

using namespace KYahoo;

SendMessageTask::SendMessageTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

SendMessageTask::~SendMessageTask()
{
}

void SendMessageTask::onGo()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	if( m_text.isEmpty() )
	{
		kDebug(YAHOO_RAW_DEBUG) << "Text to send is empty.";
		client()->notifyError( i18n( "An error occurred while sending the message" ), i18n( "The message is empty." ), Client::Debug );
		return;
	}	
	int pos=0;
	
	// split messages that are longer than 800 chars. they get dropped otherwise
	while( pos < m_text.length() )
	{
		YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceMessage, Yahoo::StatusOffline);
		t->setId( client()->sessionID() );
		t->setParam( 1, client()->userId().toLocal8Bit() );
		t->setParam( 5, m_target.toLocal8Bit() );
		t->setParam( 14, m_text.mid( pos, 700).toUtf8() );
		t->setParam( 63, ";0" );
		t->setParam( 64, "0"  );	
		t->setParam( 97, 1 );	// UTF-8
		t->setParam( 206, client()->pictureFlag() );	
		send( t );

		pos += 700;
	}
	
	setSuccess();
}

void SendMessageTask::setTarget( const QString &to )
{
	m_target = to;
}

void SendMessageTask::setText( const QString &text )
{
	m_text = text;
}

void SendMessageTask::setPicureFlag( int flag )
{
	m_pictureFlag = flag;
}
