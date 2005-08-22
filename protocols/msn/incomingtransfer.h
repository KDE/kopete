/*
    incomingtransfer.h - msn p2p protocol

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

#ifndef INCOMINGTRANSFER_H
#define INCOMINGTRANSFER_H

#include "p2p.h"
#include "dispatcher.h"

namespace KNetwork{
	class KServerSocket;
}

/**
@author Kopete Developers
*/
namespace P2P{
	class IncomingTransfer : public P2P::TransferContext
	{	Q_OBJECT
		public:
			IncomingTransfer(const QString& from, P2P::Dispatcher *dispatcher, Q_UINT32 sessionId);
			virtual ~IncomingTransfer();

		private slots:
			void slotListenError(int errorCode);
			void slotAccept();
			void slotSocketRead();
			void slotSocketClosed();
			void slotSocketError(int errorCode);
			
			void slotTransferAccepted(Kopete::Transfer* transfer, const QString& fileName);
			void slotTransferRefused(const Kopete::FileTransferInfo& info);

			
		private:
			virtual void acknowledged();
			virtual void processMessage(const Message& message);

			KTempFile *m_tempFile;
			KNetwork::KServerSocket   *m_listener;
	};
}

#endif
