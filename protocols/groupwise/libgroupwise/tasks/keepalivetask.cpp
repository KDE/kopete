//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <iostream.h>

#include <qtimer.h>

#include "client.h"
#include "request.h"
#include "requestfactory.h"
#include "keepalivetask.h"

#define GW_KEEPALIVE_INTERVAL 60000

KeepAliveTask::KeepAliveTask(Task* parent): RequestTask(parent)
{
	m_keepAliveTimer = new QTimer();
	connect( m_keepAliveTimer, SIGNAL( timeout() ), SLOT( slotSendKeepAlive ) );
	m_keepAliveTimer->start( GW_KEEPALIVE_INTERVAL );
}


KeepAliveTask::~KeepAliveTask()
{
	m_keepAliveTimer->stop();
	delete m_keepAliveTimer;
}

void KeepAliveTask::slotSendKeepAlive()
{
	QCString command = "ping";
	setTransfer( client()->requestFactory()->request( command ) );
	go();
}

void KeepAliveTask::onGo()
{
	cout << "KeepAliveTask::onGo() - sending keepalive/ping" << endl;
	send( static_cast<Request *>( transfer() ) );
}

#include "keepalivetask.moc"
