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

// Solid includes
#include <solid/networkinterface.h>
#include <solid/networkmanager.h>
#include <solid/devicemanager.h>
#include <solid/device.h>
#include <solid/capability.h>
#include <solid/control/networking.h> //socket stuff

// Jingle includes
#include "jinglenetwork.h"

// KDE includes
#include <kdebug.h>

JingleNetworkManager(PortAddress GoogleStunAddress)
{

	QList<Solid::NetworkInterface> nets;
	CreateNetworks(&nets);
	candidates = CreateCandidates(nets);

}

void makeConnection()
{
	currentCandidate = findBestCandidate();
	connectWithCandidate(candidates[currentCandidate]);
}

void connectWithCandidate(JingleConnectionCandidate candidate)
{
	socket = &(candidate.getSocket());
	connect(socket,SIGNAL(disconnected()),this,SLOT(slotSocketDisconnected()));
	connect(socket,SIGNAL(error ( QAbstractSocket::SocketError socketError )), this, SLOT(slotSocketError(QAbstractSocket::SocketError socketError )));
	socket->connectToHost(



}

void JingleNetworkManager::CreateNetworks(QList<Solid::NetworkInterface>& networks)
{
	Solid::NetworkManager &netmanager = Solid::NetworkManager::self();
	if(netmanager.isNetworkingEnabled() )
	{
		kDebug() << "Networking is enabled.  Feel free to go online!"
				<< endl;
	}
	else
	{
		kDebug() << "Network not available." << endl;
	}

	//Solid::DeviceManager &manager = Solid::DeviceManager::self();
	//QList<Solid::Device> netlist = manager.findDevicesFromQuery(Solid::Capability::NetworkHw);
	
	//NOTE does this include loopback?
	//QList<Solid::NetworkInterface>
	networks = netmanager.networkInterfaces();

	//I'm going at this from the wrong end.


	//networks =  QNetworkInterface::allInterfaces ();

// 	QList<QNetworkInterface>::iterator it = networks.begin();
// 	//for(i=networks.begin(); i != networks.end(); ++i){
// 	while(it != networks.end()){
// 		QNetworkInterface::InterfaceFlags flags = it->flags();
// 		if (!(flags & QNetworkInterface::IsUp)){
// 		//NOTE does the QNetworkInterface need a delete call?
// 		it=networks.erase(it);
// 		}else{
// 			++it;
// 		}
// 	}

}


QList<JingleConnectionCandidate> CreateCandidates(QList<Solid::NetworkInterface> networks)
{
	QList<JingleConnectionCandidate> candidates;
	//create local candidates
	for(int i=0;i<networks.size();i++){
//TODO actually rank connections
		candidates.add(JingleConnectionCandidate(QTcpSocket(),networks[i],1.0,JingleConnectionCandidate::LOCAL_TYPE);
		candidates.add(JingleConnectionCandidate(QUdpSocket(),networks[i],0.5,JingleConnectionCandidate::LOCAL_TYPE);
	}
	//TODO create STUN and relay candidates

}

void signalSocketError(QAbstractSocket::SocketError socketError)
{
//TODO special case for DatagramTooLargeError?
	disconnect(socket,SIGNAL(disconnected()),this,SLOT(slotSocketDisconnected()));
	disconnect(socket,SIGNAL(error ( QAbstractSocket::SocketError socketError )), this, SLOT(slotSocketError(QAbstractSocket::SocketError socketError )));

	candidates[currentCandidate].setQuality(0.0);
	currentCandidate = findBestCandidate();
	connectWithCandidate(candidates[currentCandidate]);

}

void signalSocketDisconnected()
{
	disconnect(socket,SIGNAL(disconnected()),this,SLOT(slotSocketDisconnected()));
	disconnect(socket,SIGNAL(error ( QAbstractSocket::SocketError socketError )), this, SLOT(slotSocketError(QAbstractSocket::SocketError socketError )));

	candidates[currentCandidate].setQuality(candidates[currentCandidate].getQuality - 0.5);
	currentCandidate = findBestCandidate();
	connectWithCandidate(candidates[currentCandidate]);
}

#include "jinglenetwork.moc"
