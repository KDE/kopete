/*
    Kopete Oscar Protocol
    logintask.cpp - Handles logging into to the AIM or ICQ service

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>
    Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

    Kopete (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

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

#include "connection.h"
#include "closeconnectiontask.h"
#include "oscarlogintask.h"
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
	m_loginTask = 0L;
	m_closeTask = 0L;
}

StageOneLoginTask::~StageOneLoginTask()
{
}

bool StageOneLoginTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Starting login";
		kDebug(OSCAR_RAW_DEBUG) << "Sending the FLAP version back";

		//send the flap version response
		FLAP f = { 0x01, 0 , 0 };
		Buffer *outbuf = new Buffer;
		outbuf->addDWord(0x00000001); //flap version
		f.length = outbuf->length();
		Transfer* ft = createTransfer( f, outbuf );
		send( ft );

		m_loginTask = new OscarLoginTask( client()->rootTask() );
		m_closeTask = new CloseConnectionTask( client()->rootTask() );
		connect( m_loginTask, SIGNAL(finished()), this, SLOT(loginTaskFinished()) );
		connect( m_closeTask, SIGNAL(finished()), this, SLOT(closeTaskFinished()) );
		m_loginTask->go( Task::AutoDelete );
		return true;
	}
	return false;
}

void StageOneLoginTask::closeTaskFinished()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	if ( m_closeTask->success() )
		setSuccess( m_closeTask->statusCode(), m_closeTask->statusString() );
	else
		setError( m_closeTask->statusCode(), m_closeTask->statusString() );
}

void StageOneLoginTask::loginTaskFinished()
{
	kDebug(OSCAR_RAW_DEBUG) ;
	m_cookie = m_loginTask->cookie();
	m_bosPort = m_loginTask->bosPort();
	m_bosServer = m_loginTask->bosHost();
	m_bosEncrypted = m_loginTask->bosEncrypted();
	m_bosSSLName = m_loginTask->bosSSLName();

	if ( !m_loginTask->success() )
	{
		disconnect( m_closeTask, SIGNAL(finished()), this, SLOT(closeTaskFinished()) );
		setError( m_loginTask->statusCode(), m_loginTask->statusString() );
	}
}

bool StageOneLoginTask::forMe( const Transfer* transfer ) const
{
	const FlapTransfer* ft = dynamic_cast<const FlapTransfer*> ( transfer );
	
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

bool StageOneLoginTask::bosEncrypted() const
{
	return m_bosEncrypted;
}

const QString& StageOneLoginTask::bosSSLName() const
{
	return m_bosSSLName;
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
	
	QObject::connect( m_versionTask, SIGNAL(finished()), this, SLOT(versionTaskFinished()) );
	QObject::connect( m_rateTask, SIGNAL(finished()), this, SLOT(rateTaskFinished()) );
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

bool StageTwoLoginTask::forMe( const Transfer* transfer ) const
{
	const FlapTransfer* ft = dynamic_cast<const FlapTransfer*>( transfer );
	
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
		outbuf->addTLV( 0x06, m_cookie );
		Transfer* ft = createTransfer( f, outbuf );
		kDebug(OSCAR_RAW_DEBUG) << "Sending the login cookie back";
		send( ft );
	}
	else
		setError( -1, QString() );
	return;
}

void StageTwoLoginTask::setCookie( const QByteArray& newCookie )
{
	m_cookie = newCookie;
}

const QByteArray& StageTwoLoginTask::cookie()
{
	return m_cookie;
}

void StageTwoLoginTask::versionTaskFinished()
{
	//start the rate info task
	m_rateTask->go( Task::AutoDelete );
}

void StageTwoLoginTask::rateTaskFinished()
{
	setSuccess( 0, QString() );
}

#include "logintask.moc"

//kate: tab-width 4; indent-mode csands;
