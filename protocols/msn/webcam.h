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

class MimicWrapper;
class QLabel;
class MSNWebcamDialog;
class QTimerEvent;

namespace P2P {


class Webcam  : public TransferContext
{  Q_OBJECT
	public:
		enum Who { wProducer , wViewer };
		
		Webcam( Who who , const QString& to, Dispatcher *parent, Q_UINT32 sessionID);
		~Webcam( );

		virtual void processMessage(const Message& message);
		
	public slots:
		void askIncommingInvitation();
		virtual void acknowledged();
		void sendBYEMessage();
	
	private:
		void makeSIPMessage(const QString &message, Q_UINT8 XX=0, Q_UINT8 YY=9 , Q_UINT8 ZZ=0);
		void sendBigP2PMessage( const QByteArray& dataMessage );
		void closeAllOtherSockets();
		QString m_content;
		
		QString xml(uint session , uint rid);
		int getAvailablePort();

		
		KNetwork::KServerSocket   *m_listener;
		KNetwork::KBufferedSocket *m_webcamSocket;
		
		enum WebcamStatus { wsNegotiating , wsConnecting, wsConnected, wsTransfer  } ;
		
		Who m_who;
		
		QString m_myAuth;
		QString m_peerAuth;
		
		MimicWrapper *m_mimic;
		MSNWebcamDialog *m_widget;
				
		QValueList<KNetwork::KBufferedSocket* > m_allSockets;
		QMap<KNetwork::KBufferedSocket*, WebcamStatus> m_webcamStates;
		
		int m_timerId;
		int m_timerFps;

	private slots:
		void slotListenError(int errorCode);
		void slotAccept();
		void slotSocketRead();
		void slotSocketClosed();
		void slotSocketError(int errorCode);
		void slotSocketConnected();
//		void slotReadyWrite();
	protected:
		virtual void timerEvent( QTimerEvent * );
};

}

#endif

#endif
