/***************************************************************************
                          connectionstatusplugin.cpp  -  description
                             -------------------
    begin                : 26th Oct 2002
    copyright            : (C) 2002 by Chris Howells, Duncan Mac-Vicar Prett
    email                : howells@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <kgenericfactory.h>
#include <qtimer.h>
#include <kprocess.h>

#include "connectionstatusplugin.h"
#include "kopeteidentitymanager.h"

K_EXPORT_COMPONENT_FACTORY(kopete_connectionstatus, KGenericFactory<ConnectionStatusPlugin>);

ConnectionStatusPlugin::ConnectionStatusPlugin(QObject *parent, const char *name,
	const QStringList& /* args */ )
: KopetePlugin(parent, name)
{
	kdDebug(14301) << "ConnectionStatusPlugin::ConnectionStatusPlugin()" << endl;
	qtTimer = new QTimer();
	connect(qtTimer, SIGNAL(timeout()), this,
		 SLOT(slotCheckStatus()) );
	qtTimer->start(60000);

	kpIfconfig = new KProcess;
	connect(kpIfconfig, SIGNAL(receivedStdout(KProcess *, char *, int)),
		this, SLOT(slotProcessStdout(KProcess *, char *, int)));

	m_boolPluginConnected = false;
}

ConnectionStatusPlugin::~ConnectionStatusPlugin()
{
	kdDebug(14301) << "ConnectionStatusPlugin::~ConnectionStatusPlugin()" << endl;
	delete qtTimer;
	delete kpIfconfig;
}

void ConnectionStatusPlugin::slotCheckStatus()
{
	/* Use KProcess to run netstat -r. We'll then parse the output of
	* netstat -r in slotProcessStdout() to see if it mentions the
	* default gateway. If so, we're connected, if not, we're offline */

	kdDebug(14301) << "ConnectionStatusPlugin::checkStatus()" << endl;
	*kpIfconfig << "netstat" << "-r";
	kpIfconfig->start(KProcess::DontCare, KProcess::Stdout);
}

void ConnectionStatusPlugin::slotProcessStdout(KProcess *, char *buffer, int buflen)
{
	// Look for a default gateway
	kdDebug(14301) << "ConnectionStatusPlugin::slotProcessStdout()" << endl;
	QString qsBuffer = QString::fromLatin1(buffer, buflen);
	//kdDebug(14301) << qsBuffer << endl;
	setConnectedStatus(qsBuffer.contains("default"));
}

void ConnectionStatusPlugin::setConnectedStatus(bool connected)
{
	/* We have to handle a few cases here. First is the machine is connected, and the plugin thinks
	* we're connected. Then we don't do anything. Next, we can have machine connected, but plugin thinks
	* we're disconnected. Then we call slotConnectAll(). Also, machine disconnected, plugin disconnected -- we
	* don't do anything. Finally, we can have the machine disconnected, and the plugin thinks we're
	* connected -- then we call slotDisconnectAll(). This mechanism is required so that we don't
	* keep calling slotConnectAll() or slotDisconnectAll() constantly.
	*/

	kdDebug(14301) << "ConnectionStatusPlugin::setConnectedStatus()" << endl;

	if (connected && !m_boolPluginConnected) // the machine is connected and plugin thinks we're disconnected
	{
		kdDebug(14301) << "Setting m_boolPluginConnected to true" << endl;
		m_boolPluginConnected = true;
		kdDebug(14301) << "ConnectionStatusPlugin::setConnectedStatus() -- we're connected" << endl;
		KopeteIdentityManager::manager()->connectAll();
	}
	else
	if (!connected && m_boolPluginConnected) // the machine isn't connected and plugin thinks we're connected
	{
		kdDebug(14301) << "Setting m_boolPluginConnected to false" << endl;
		m_boolPluginConnected = false;
		kdDebug(14301) << "ConnectionStatusPlugin::setConnectedStatus() -- we're offline" << endl;
		KopeteIdentityManager::manager()->disconnectAll();
	}
}

#include "connectionstatusplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

