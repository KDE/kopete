/*
    Kopete Oscar Protocol
    logintask.cpp - Handles logging into to the AIM or ICQ service

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "logintask.h"

#include <qtimer.h>
#include <kdebug.h>
#include <klocale.h>

#include "aimlogintask.h"
#include "connection.h"
#include "closeconnectiontask.h"
#include "icqlogintask.h"
#include "oscarutils.h"
#include "rateinfotask.h"
#include "serverversionstask.h"
#include "transfer.h"



/**
 * Stage One Task Implementation
 */

StageOneLoginTask::StageOneLoginTask( Task* parent )
	: Task ( parent )
{
	m_aimTask = 0L;
	m_icqTask = 0L;
	m_closeTask = 0L;
}

StageOneLoginTask::~StageOneLoginTask()
{
	delete m_aimTask;
	delete m_icqTask;
	delete m_closeTask;
}

bool StageOneLoginTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		if ( client()->isIcq() )
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Starting ICQ login" << endl;
			m_icqTask = new IcqLoginTask( client()->rootTask() );
			m_closeTask = new CloseConnectionTask( client()->rootTask() );
			
			//connect finished signal
			connect( m_closeTask, SIGNAL( finished() ), this, SLOT( closeTaskFinished() ) );
			m_icqTask->go( true );
		}
		else
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Starting AIM login" << endl;
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending the FLAP version back" << endl;
			
			//send the flap version response
			FLAP f = { 0x01, 0 , 0 };
			Buffer *outbuf = new Buffer;
			outbuf->addDWord(0x00000001); //flap version
			f.length = outbuf->length();
			Transfer* ft = createTransfer( f, outbuf );
			send( ft );
			
			m_aimTask = new AimLoginTask( client()->rootTask() );
			connect( m_aimTask, SIGNAL( finished() ), this, SLOT( aimTaskFinished() ) );
			m_aimTask->go( true );
		}
		return true;
	}
	return false;
}

void StageOneLoginTask::closeTaskFinished()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_cookie = m_closeTask->cookie();
	m_bosPort = m_closeTask->bosPort();
	m_bosServer = m_closeTask->bosHost();
	m_closeTask->safeDelete();
	setSuccess( m_closeTask->statusCode(), m_closeTask->statusString() );
}

void StageOneLoginTask::aimTaskFinished()
{
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	m_cookie = m_aimTask->cookie();
	m_bosPort = m_aimTask->bosPort();
	m_bosServer = m_aimTask->bosHost();
	
	setSuccess( m_aimTask->statusCode(), m_aimTask->statusString() );
}

bool StageOneLoginTask::forMe( Transfer* transfer ) const
{
	FlapTransfer* ft = dynamic_cast<FlapTransfer*> ( transfer );
	
	if (!ft)
		return false;
	
	return ( ft && ft->flapChannel() == 1 );
}

const QByteArray& StageOneLoginTask::loginCookie() const
{
	return m_cookie;
}

const QString& StageOneLoginTask::bosServer() const
{
	return m_bosServer;
}

const QString& StageOneLoginTask::bosPort() const
{
	return m_bosPort;
}


/**
 * Stage Two Task Implementation
 */
StageTwoLoginTask::StageTwoLoginTask( Task* parent )
	: Task( parent )
{
	//Create our tasks
	Task* rootTask = client()->rootTask();
	m_versionTask = new ServerVersionsTask( rootTask );
	m_rateTask = new RateInfoTask( rootTask );
	
	QObject::connect( m_versionTask, SIGNAL( finished() ), this, SLOT( versionTaskFinished() ) );
	QObject::connect( m_rateTask, SIGNAL( finished() ), this, SLOT( rateTaskFinished() ) );
}

StageTwoLoginTask::~StageTwoLoginTask()
{
	delete m_versionTask;
}

bool StageTwoLoginTask::take( Transfer* transfer )
{
	bool yes = forMe( transfer );
	return yes;
}

bool StageTwoLoginTask::forMe( Transfer* transfer ) const
{
	FlapTransfer* ft = dynamic_cast<FlapTransfer*>( transfer );
	
	if (!ft)
		return false;
	
	int channel = ft->flapChannel();
	if ( channel == 1 )
		return true;
	else
		return false;
}

void StageTwoLoginTask::onGo()
{
	if ( !m_cookie.isEmpty() )
	{
		//send the flap back
		FLAP f = { 0x01, 0, 0 };
		Buffer* outbuf = new Buffer();
		outbuf->addDWord( 0x00000001 );
		outbuf->addTLV( 0x06, m_cookie.size(), m_cookie.data() );
		Transfer* ft = createTransfer( f, outbuf );
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending the login cookie back" << endl;
		send( ft );
	}
	else
		setError( -1, QString::null );
	return;
}

void StageTwoLoginTask::setCookie( const QByteArray& newCookie )
{
	m_cookie.duplicate( newCookie );
}

const QByteArray& StageTwoLoginTask::cookie()
{
	return m_cookie;
}

void StageTwoLoginTask::versionTaskFinished()
{
	//start the rate info task
	m_rateTask->go(true);
}

void StageTwoLoginTask::rateTaskFinished()
{
	setSuccess( 0, QString::null );
}

#include "logintask.moc"

//kate: tab-width 4; indent-mode csands;
