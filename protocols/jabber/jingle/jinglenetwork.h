
#ifndef JINGLENETWORK_H
#define JINGLENETWORK_H

#include <QtNetwork> 

#include "jingleinfotask.h"

class JingleNetworkManager
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
