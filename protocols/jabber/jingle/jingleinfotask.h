/*
    jinglevoicesessiondialog.cpp - GUI for a voice session.

    Copyright (c) 2007      by Joshua Hodosh     <josh.hodosh@gmail.com>

    Kopete    (c) 2001-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef JINGLEINFOTASK_H
#define JINGLEINFOTASK_H

#include "xmpp_task.h"

/**
 * A struct representing a hostname and port.
 * May contain an IP address instead of unresolved hostname.
 */
struct PortAddress
{
	PortAddress(QString hn, quint16 p){ hostname=(hn);port=(p);}
	PortAddress(QHostAddress ad, quint16 p){ address=(ad); port=(p);}
	QString hostname;
	QHostAddress address;
	quint16 port;
};

class  JingleInfoTask : XMPP::Task
{
public:
	JingleInfoTask(XMPP::Task* parent);

	void updateInfo();
signals:
	void signalJingoInfoUpdate(QString relayToken, QList<QHostAddress> relayHosts, QList<PortAddress> stunHosts);

};

#endif 
