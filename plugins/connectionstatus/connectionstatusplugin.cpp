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
#include <k3process.h>

#include "kopeteaccountmanager.h"

typedef KGenericFactory<ConnectionStatusPlugin> ConnectionStatusPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_connectionstatus, ConnectionStatusPluginFactory( "kopete_connectionstatus" )  )

ConnectionStatusPlugin::ConnectionStatusPlugin( QObject *parent, const QStringList& /* args */ )
: Kopete::Plugin( ConnectionStatusPluginFactory::componentData(), parent )
{
	kDebug( 14301 ) ;

	m_process = 0L;

	m_timer = new QTimer();
	connect( m_timer, SIGNAL( timeout() ), this, SLOT( slotCheckStatus() ) );
	m_timer->start( 60000 );

	m_pluginConnected = false;
}

ConnectionStatusPlugin::~ConnectionStatusPlugin()
{
	kDebug( 14301 ) ;
	delete m_timer;
	delete m_process;
}

void ConnectionStatusPlugin::slotCheckStatus()
{
	kDebug( 14301 ) ;

	if ( m_process )
	{
		kWarning( 14301 ) << "Previous netstat process is still running!" << endl
			<< "Not starting new netstat. Perhaps your system is under heavy load?" << endl;

		return;
	}

	m_buffer.clear();
	
	// Use K3Process to run netstat -rn. We'll then parse the output of
	// netstat -rn in slotProcessStdout() to see if it mentions the
	// default gateway. If so, we're connected, if not, we're offline
	m_process = new K3Process;
	*m_process << "netstat" << "-r";

	connect( m_process, SIGNAL( receivedStdout( K3Process *, char *, int ) ), this, SLOT( slotProcessStdout( K3Process *, char *, int ) ) );
	connect( m_process, SIGNAL( processExited( K3Process * ) ), this, SLOT( slotProcessExited( K3Process * ) ) );

	if ( !m_process->start( K3Process::NotifyOnExit, K3Process::Stdout ) )
	{
		kWarning( 14301 ) << "Unable to start netstat process!";

		delete m_process;
		m_process = 0L;
	}
}

void ConnectionStatusPlugin::slotProcessExited( K3Process *process )
{
	kDebug( 14301 ) << m_buffer;

	if ( process == m_process )
	{
		setConnectedStatus( m_buffer.contains( "default" ) );
		m_buffer.clear();
		delete m_process;
		m_process = 0L;
	}
}

void ConnectionStatusPlugin::slotProcessStdout( K3Process *, char *buffer, int buflen )
{
	// Look for a default gateway
	//kDebug( 14301 ) ;
	m_buffer += QString::fromLatin1( buffer, buflen );
	//kDebug( 14301 ) << qsBuffer;
}

void ConnectionStatusPlugin::setConnectedStatus( bool connected )
{
	//kDebug( 14301 ) ;

	// We have to handle a few cases here. First is the machine is connected, and the plugin thinks
	// we're connected. Then we don't do anything. Next, we can have machine connected, but plugin thinks
	// we're disconnected. Also, machine disconnected, plugin disconnected -- we
	// don't do anything. Finally, we can have the machine disconnected, and the plugin thinks we're
	// connected. This mechanism is required so that we don't keep calling the connect/disconnect functions
	// constantly.

	if ( connected && !m_pluginConnected )
	{
		// The machine is connected and plugin thinks we're disconnected
		kDebug( 14301 ) << "Setting m_pluginConnected to true";
		m_pluginConnected = true;
		Kopete::AccountManager::self()->connectAll();
		kDebug( 14301 ) << "We're connected";
	}
	else if ( !connected && m_pluginConnected )
	{
		// The machine isn't connected and plugin thinks we're connected
		kDebug( 14301 ) << "Setting m_pluginConnected to false";
		m_pluginConnected = false;
		Kopete::AccountManager::self()->disconnectAll();
		kDebug( 14301 ) << "We're offline";
	}
}

#include "connectionstatusplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

