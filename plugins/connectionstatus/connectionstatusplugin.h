/***************************************************************************
                          connectionstatusplugin.h
                             -------------------
    begin                : 26th Oct 2002
    copyright            : (C) 2002-2003 Chris Howells
    email                : howells@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#ifndef CONNECTIONSTATUSPLUGIN_H
#define CONNECTIONSTATUSPLUGIN_H

#include "kopeteplugin.h"

class QTimer;
class KProcess;

/**
 * @author Chris Howells <howells@kde.org>
 */
class ConnectionStatusPlugin : public KopetePlugin
{
	Q_OBJECT

public:

	ConnectionStatusPlugin(QObject *parent, const char *name, const QStringList &args);
	~ConnectionStatusPlugin();

private slots:

	void slotCheckStatus();
	void slotProcessStdout(KProcess *, char *, int);

private:

	void setConnectedStatus(bool);
	bool m_boolPluginConnected;
	QString m_qsInterface;
	KProcess *kpIfconfig;
	QTimer *qtTimer;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

