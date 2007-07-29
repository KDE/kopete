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
#ifndef JINGLENETWORK_H
#define JINGLENETWORK_H

#include <QtNetwork> 

#include "jingleinfotask.h"

class JingleNetworkManager : public QObject
{
	Q_OBJECT
public:
	JingleNetworkManager(PortAddress* stunAddress);

	void makeConnection();
	

	// Creates a network object for each network available on the machine.
	static void CreateNetworks(QList<Solid::NetworkInterface>& networks);

	static QList<JingleConnectionCandidate> CreateCandidates(QList<Solid::NetworkInterface> networks);

public slots:
	void slotJingleNetworkInfoUpdated(QString relayToken, QList<QHostAddress> relayHosts, QList<PortAddress> stunHosts);

protected slots:
	void slotSocketDisconnected();
	void slotSocketError(QAbstractSocket::SocketError socketError );


private:
	QAbstractSocket socket*;
	QMap<QString,Solid::NetworkInterface> networks_; 
	void connectWithCandiate(JingleConnectionCandidate);
	int currentCandidate;
	QList<JingleConnectionCandidate> candidates;
	int findBestCandidate();
};

#endif
