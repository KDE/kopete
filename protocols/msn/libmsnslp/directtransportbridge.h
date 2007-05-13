/*
    directtransportbridge.h - Peer to Peer Direct Transport Bridge

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__DIRECTTRANSPORTBRIDGE_H
#define CLASS_P2P__DIRECTTRANSPORTBRIDGE_H

#include "transportbridge.h"
#include <qvaluelist.h>

namespace PeerToPeer
{

class DirectTransportBridge : public TransportBridge
{
	Q_OBJECT

	public :
		/** @brief Creates a new instance of the class DirectTransportBridge. */
		DirectTransportBridge(const QValueList<QString>& addresses, const Q_UINT16 port, QObject *parent);
		/** @brief Finalizer. */
		virtual ~DirectTransportBridge();

	protected:
		QValueList<QString> & addresses() const;
		Q_UINT16 port() const;

	private:
		class DirectTransportBridgePrivate;
		DirectTransportBridgePrivate *d;

}; // DirectTransportBridge
}

#endif
