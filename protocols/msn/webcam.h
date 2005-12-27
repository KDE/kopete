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

//#if MSN_WEBCAM
#if 0

#include "p2p.h"

//Added by qt3to4:
#include <QLabel>
#include <QTimerEvent>
#include <QList>



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
		
		Webcam( Who who , const QString& to, Dispatcher *parent, quint32 sessionID);
		virtual ~Webcam();

		virtual void processMessage(const Message& message);
		
	public slots:
		void askIncommingInvitation();
		virtual void acknowledged();
		void sendBYEMessage();
	
	private:
		void makeSIPMessage(const QString &message, quint8 XX=0, quint8 YY=9 , quint8 ZZ=0);
		void sendBigP2PMessage( const QByteArray& dataMessage );
		void closeAllOtherSockets();
		QString m_content;
		
		QString xml(uint session , uint rid);

		
		KNetwork::KServerSocket   *m_listener;
		KNetwork::KBufferedSocket *m_webcamSocket;
		
		enum { wsNegotiating , wsConnecting, wsConnected, wsTransfer  } m_webcamState;
		
		Who m_who;
		
		QString m_myAuth;
		QString m_peerAuth;
		
		MimicWrapper *m_mimic;
		MSNWebcamDialog *m_widget;
				
		QList<KNetwork::KBufferedSocket* > m_allSockets;
		
		int m_timerId;

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
