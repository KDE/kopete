/***************************************************************************
                          connectionstatusplugin.h -  description
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

#ifndef CONNECTIONSTATUSPLUGIN_H
#define CONNECTIONSTATUSPLUGIN_H

#include "kopeteplugin.h"

class QTimer;
class KProcess;

/**
 * @author Chris Howells
 */
class ConnectionStatusPlugin : public KopetePlugin
{
	Q_OBJECT

public:
	ConnectionStatusPlugin( QObject *parent, const char *name, const QStringList &args );
	~ConnectionStatusPlugin();
	virtual void init();
	virtual bool unload();

private slots:
	void slotCheckStatus();
	void slotProcessStdout(KProcess *, char *, int);

private:
	QTimer *qtTimer;
	void setConnectedStatus(bool);
	bool m_boolPluginConnected;
	QString m_qsInterface;
	KProcess *kpIfconfig;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

