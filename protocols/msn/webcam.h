/*
    Copyright (c) 2005 by Olivier Goffart        <ogoffart@ kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef P2PWEBCAM_H
#define P2PWEBCAM_H

#include "p2p.h"

#if MSN_WEBCAM

namespace KNetwork{ class KServerSocket; class KBufferedSocket;  }

namespace P2P {


class Webcam  : public TransferContext
{  Q_OBJECT
	public:
		Webcam( const QString& to, Dispatcher *parent, Q_UINT32 sessionID);
		~Webcam( );

		virtual void acknowledged();
		virtual void processMessage(const Message& message);
	
	private:
		void makeSIPMessage(const QString &message);
		void sendBigP2PMessage( const QByteArray& dataMessage );
		QString m_content;
		
		QString xml(uint session , uint rid);
		
		KNetwork::KServerSocket   *m_listener;
		KNetwork::KBufferedSocket *m_webcamSocket;
		
		enum { wsNegotiating , wsConnecting, wsAuth , wsTransfer   } m_state;
		
		QString m_auth;
		
		
	private slots:
		void slotListenError(int errorCode);
		void slotAccept();
		void slotSocketRead();
		void slotSocketClosed();
		void slotSocketError(int errorCode);
//		void slotReadyWrite();
};

}

#endif

#endif
