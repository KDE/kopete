/*
    connectionstatusplugin.h

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

#ifndef CONNECTIONSTATUSPLUGIN_H
#define CONNECTIONSTATUSPLUGIN_H

#include "kopeteplugin.h"

class QTimer;
class K3Process;

/**
 * @author Chris Howells <howells@kde.org>
 */
class ConnectionStatusPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	ConnectionStatusPlugin( QObject *parent, const QStringList &args );
	~ConnectionStatusPlugin();

private slots:
	void slotCheckStatus();
	void slotProcessStdout( K3Process *process, char *buffer, int len );

	/**
	 * Notify when the netstat process has exited
	 */
	void slotProcessExited( K3Process *process );

private:
	void setConnectedStatus( bool newStatus );

	bool m_pluginConnected;
	K3Process *m_process;
	QTimer *m_timer;
	QString m_buffer;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

