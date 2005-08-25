/*
    outgoingtransfer.h - msn p2p protocol

    Copyright (c) 2003-2005 by Olivier Goffart        <ogoffart@ kde.org>
    Copyright (c) 2005      by Gregg Edghill          <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef OUTGOINGTRANSFER_H
#define OUTGOINGTRANSFER_H

#include "p2p.h"
#include "dispatcher.h"
#include <qstringlist.h>

/**
@author Kopete Developers
*/
namespace P2P{
	class OutgoingTransfer : public TransferContext
	{	Q_OBJECT
		public:
			OutgoingTransfer(const QString& to, P2P::Dispatcher *dispatcher, Q_UINT32 sessionId);
			virtual ~OutgoingTransfer();

			void sendImage(const QByteArray& image);

		private slots:
			void slotConnected();
			void slotRead();
			void slotSendData();
			void slotSocketError(int);
			void slotSocketClosed();
			
		private:
			virtual void acknowledged();
			void connectToEndpoint(const QString& hostName);
			virtual void processMessage(const Message& message);

			QStringList m_peerEndpoints;
			QStringList::Iterator m_endpointIterator;
			QString m_remotePort;
			QString m_nonce;
			char m_handshake;

		protected:
			virtual void readyToSend();
	};
}

#endif
