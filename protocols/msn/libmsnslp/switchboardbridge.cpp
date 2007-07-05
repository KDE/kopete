/*
    switchboardbridge.cpp - Peer To Peer Switchboard Bridge

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "switchboardbridge.h"
#include "packet.h"
#include "msnchatsession.h"
#include <qmap.h>
#include <qpair.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <kdebug.h>

namespace PeerToPeer
{

class SwitchboardBridge::SwitchboardBridgePrivate
{
	public :
		SwitchboardBridgePrivate() : switchboard(0l) {}

		Q_UINT32 bridgeId;
		QMap<Q_INT32, Q_UINT32> cookies;
		QValueList< QPair<Q_UINT32, QByteArray> > pendingChunks;
		QMap<QString, QVariant> properties;
		MSNChatSession *switchboard;
};

SwitchboardBridge::SwitchboardBridge(const Q_UINT32 bridgeId, QObject* parent) : TransportBridge(parent), d(new SwitchboardBridgePrivate())
{
	d->bridgeId = bridgeId;
	d->properties.insert("mtu", 1202);
	d->properties.insert("throttle", 5);
}

SwitchboardBridge::~SwitchboardBridge()
{
	delete d;
}

const QMap<QString, QVariant> & SwitchboardBridge::getProperties() const
{
	return d->properties;
}

Q_UINT32 SwitchboardBridge::id() const
{
	return d->bridgeId;
}

void SwitchboardBridge::connectTo(MSNChatSession *switchboard)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	d->switchboard = switchboard;
	// Connect the signal/slot
	QObject::connect(switchboard, SIGNAL(dataReceived(const QByteArray&)), this,
	SLOT(onDataReceived(const QByteArray&)));
	QObject::connect(switchboard, SIGNAL(onSend(const Q_INT32)), this,
	SLOT(onSend(const Q_INT32)));

	kdDebug() << k_funcinfo << "leave" << endl;
}

void SwitchboardBridge::send(const QByteArray& bytes, const Q_UINT32 packetId)
{
	kdDebug() << k_funcinfo << "About to send datachunk of size " << bytes.size() << " bytes" << endl;

	if (TransportBridge::state() == TransportBridge::Connected)
	{
		// If the bridge is connected, try to send the data.
		Q_UINT32 cookie;
		if (!trySendViaSwitchboard(bytes, cookie))
		{
			// If the network is not available, queue the data.
			d->pendingChunks.append(qMakePair(packetId, bytes));
		}
		else
		{
			// Otherwise, add a cookie for the sent data.
			kdDebug() << k_funcinfo << "datachunk has id of " << cookie
			<< " (cookie= " << packetId << ")" << endl;

			d->cookies.insert(cookie, packetId);
		}
	}
	else
	{
		// If the bridge is not connected, queue the data.
		d->pendingChunks.append(qMakePair(packetId, bytes));
	}
}

bool SwitchboardBridge::trySendViaSwitchboard(const QByteArray& bytes, Q_UINT32& cookie)
{
	cookie = 0;

	if (d->switchboard == 0l)
	{
		kdDebug() << k_funcinfo << "Switchboard not connected -- returning" << endl;
		return false;
	}

	// Send the data via the switchboard.
	const Q_INT32 tId = d->switchboard->send(bytes);
	if (tId >= 0)
	{
		// If the transaction id is valid, set
		// set the cookie for the sent data.
		cookie = tId;
	}

	return (tId >= 0);
}

//BEGIN Switchboard Bridge Event Handling Functions

void SwitchboardBridge::onDataReceived(const QByteArray& bytes)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	const Q_INT32 maxDataChunkSize = d->properties["mtu"].toInt();
	if (bytes.size() > (sizeof(Packet::Header) + maxDataChunkSize + 4))
	{
		kdDebug() << k_funcinfo << "Got data whose size exceeds mtu size="
			<< maxDataChunkSize << " bytes -- ignoring it." << endl;
		return;
	}

	emit dataReceived(bytes);

	kdDebug() << k_funcinfo << "leave" << endl;
}

void SwitchboardBridge::onSend(const Q_INT32 id)
{
	kdDebug() << k_funcinfo << "enter" << endl;

	if (d->cookies.contains(id))
	{
		Q_UINT32 packetId = d->cookies[id];
		kdDebug() << k_funcinfo << "cookie= " << packetId << endl;

		emit dataSent(packetId);
		d->cookies.remove(id);
	}

	kdDebug() << k_funcinfo << "leave" << endl;
}

//END

void SwitchboardBridge::onSendPendingPackets()
{
	QValueList< QPair<Q_UINT32, QByteArray> >::Iterator i = d->pendingChunks.begin();
	while (i != d->pendingChunks.end())
	{
		Q_UINT32 packetId = (*i).first;
		QByteArray bytes = (*i).second;

		Q_UINT32 cookie;
		if (!trySendViaSwitchboard(bytes, cookie))
		{
			// If the switchboard is not available, stop.
			break;
		}
		else
		{
			// Otherwise, add a cookie for the sent data.
			d->cookies.insert(cookie, packetId);
			d->pendingChunks.remove(i);
		}
	}
}

void SwitchboardBridge::onConnect()
{
	if (state() == TransportBridge::Connected)
	{
		kdDebug() << k_funcinfo << "Already connected" << endl;
		return;
	}

	setState(TransportBridge::Connected);
	kdDebug() << k_funcinfo << endl;

	if (d->pendingChunks.size() > 0)
	{
		QTimer::singleShot(0, this, SLOT(onSendPendingPackets()));
	}

	// Raise the connected event signal.
	emit connected();
}

void SwitchboardBridge::onDisconnect()
{
	if (state() == TransportBridge::Connected)
	{
		setState(TransportBridge::Disconnected);
		kdDebug() << k_funcinfo << endl;
		// Raise the disconnected event signal.
		emit disconnected();
	}
}

}

#include "switchboardbridge.moc"

