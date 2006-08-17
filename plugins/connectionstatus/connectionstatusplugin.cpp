/*
    connectionstatusplugin.cpp

    Copyright (c) 2002-2003 by Chris Howells         <howells@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "connectionstatusplugin.h"

#include <qtimer.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kprocess.h>

#include "kopeteaccountmanager.h"

typedef KGenericFactory<ConnectionStatusPlugin> ConnectionStatusPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_connectionstatus, ConnectionStatusPluginFactory( "kopete_connectionstatus" )  )

ConnectionStatusPlugin::ConnectionStatusPlugin( QObject *parent, const char *name, const QStringList& /* args */ )
: Kopete::Plugin( ConnectionStatusPluginFactory::instance(), parent, name )
{
	kdDebug( 14301 ) << k_funcinfo << endl;

	m_process = 0L;

	m_timer = new QTimer();
	connect( m_timer, SIGNAL( timeout() ), this, SLOT( slotCheckStatus() ) );
	m_timer->start( 60000 );

	m_pluginConnected = false;
}

ConnectionStatusPlugin::~ConnectionStatusPlugin()
{
	kdDebug( 14301 ) << k_funcinfo << endl;
	delete m_timer;
	delete m_process;
}

void ConnectionStatusPlugin::slotCheckStatus()
{
	kdDebug( 14301 ) << k_funcinfo << endl;

	if ( m_process )
	{
		kdWarning( 14301 ) << k_funcinfo << "Previous netstat process is still running!" << endl
			<< "Not starting new netstat. Perhaps your system is under heavy load?" << endl;

		return;
	}

	m_buffer = QString::null;
	
	// Use KProcess to run netstat -rn. We'll then parse the output of
	// netstat -rn in slotProcessStdout() to see if it mentions the
	// default gateway. If so, we're connected, if not, we're offline
	m_process = new KProcess;
	*m_process << "netstat" << "-r";

	connect( m_process, SIGNAL( receivedStdout( KProcess *, char *, int ) ), this, SLOT( slotProcessStdout( KProcess *, char *, int ) ) );
	connect( m_process, SIGNAL( processExited( KProcess * ) ), this, SLOT( slotProcessExited( KProcess * ) ) );

	if ( !m_process->start( KProcess::NotifyOnExit, KProcess::Stdout ) )
	{
		kdWarning( 14301 ) << k_funcinfo << "Unable to start netstat process!" << endl;

		delete m_process;
		m_process = 0L;
	}
}

void ConnectionStatusPlugin::slotProcessExited( KProcess *process )
{
	kdDebug( 14301 ) << m_buffer << endl;

	if ( process == m_process )
	{
		setConnectedStatus( m_buffer.contains( "default" ) );
		m_buffer = QString::null;
		delete m_process;
		m_process = 0L;
	}
}

void ConnectionStatusPlugin::slotProcessStdout( KProcess *, char *buffer, int buflen )
{
	// Look for a default gateway
	//kdDebug( 14301 ) << k_funcinfo << endl;
	m_buffer += QString::fromLatin1( buffer, buflen );
	//kdDebug( 14301 ) << qsBuffer << endl;
}

void ConnectionStatusPlugin::setConnectedStatus( bool connected )
{
	//kdDebug( 14301 ) << k_funcinfo << endl;

	// We have to handle a few cases here. First is the machine is connected, and the plugin thinks
	// we're connected. Then we don't do anything. Next, we can have machine connected, but plugin thinks
	// we're disconnected. Also, machine disconnected, plugin disconnected -- we
	// don't do anything. Finally, we can have the machine disconnected, and the plugin thinks we're
	// connected. This mechanism is required so that we don't keep calling the connect/disconnect functions
	// constantly.

	if ( connected && !m_pluginConnected )
	{
		// The machine is connected and plugin thinks we're disconnected
		kdDebug( 14301 ) << k_funcinfo << "Setting m_pluginConnected to true" << endl;
		m_pluginConnected = true;
		Kopete::AccountManager::self()->connectAll();
		kdDebug( 14301 ) << k_funcinfo << "We're connected" << endl;
	}
	else if ( !connected && m_pluginConnected )
	{
		// The machine isn't connected and plugin thinks we're connected
		kdDebug( 14301 ) << k_funcinfo << "Setting m_pluginConnected to false" << endl;
		m_pluginConnected = false;
		Kopete::AccountManager::self()->disconnectAll();
		kdDebug( 14301 ) << k_funcinfo << "We're offline" << endl;
	}
}

#include "connectionstatusplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

