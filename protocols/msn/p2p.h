/*
    p2p.h - msn p2p protocol

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

#ifndef P2P_H
#define P2P_H

// Qt includes
#include <qobject.h>
#include "messageformatter.h"

namespace Kopete { class Transfer; }
namespace Kopete { struct FileTransferInfo; }
namespace P2P { class Dispatcher; }
namespace KNetwork { class KBufferedSocket; }
class QFile;
class KTempFile;

/**
@author Kopete Developers
*/
namespace P2P{

	enum TransferType { UserDisplayIcon = 1, CustomEmotions = 2, File = 8};
	enum TransferDirection { Incoming = 1, Outgoing = 8};
	enum MessageType { BYE, OK, DECLINE, ERROR, INVITE };

	enum CommunicationState
	{
		Invitation   = 1,
		Negotiation  = 2,
		DataTransfer = 8,
		Finished     = 16
	};

	struct TransportHeader
	{
		Q_UINT32 sessionId;
		Q_UINT32 identifier;
		Q_INT64  dataOffset;
		Q_INT64  totalDataSize;
		Q_UINT32 dataSize;
		Q_UINT32 flag;
		Q_UINT32 ackSessionIdentifier;
		Q_UINT32 ackUniqueIdentifier;
		Q_INT64  ackDataSize;
	};
	
	struct Message
	{
		public:
			QString mimeVersion;
			QString contentType;
			QString destination;
			TransportHeader header;
			QByteArray body;
			Q_INT32 applicationIdentifier;
	};

	class Uid
	{
		public: static QString createUid();
	};
	
	class TransferContext : public QObject
	{	Q_OBJECT
		public:
			virtual ~TransferContext();

			void acknowledge(const Message& message);
			virtual void acknowledged() = 0;
			void error();
			virtual void processMessage(const Message& message) = 0;
			void sendDataPreparation();
			void sendMessage(MessageType type, const QString& content=QString::null, Q_INT32 flag=0, Q_INT32 appId=0);
			void setType(TransferType type);
			
		public:
			Q_UINT32 m_sessionId;
			Q_UINT32 m_identifier;
			QString  m_branch;
			QString  m_callId;
			QFile   *m_file;
			// TODO do we really need this?
			QString  m_object;
			Q_UINT32 m_transactionId;
			Kopete::Transfer *m_transfer;

		public slots:
			void abort();
			void readyWrite();
			
		protected:
			TransferContext(P2P::Dispatcher *dispatcher);
			void sendData(const QByteArray& bytes);
			virtual void readyToSend();
			
			Q_UINT32 m_baseIdentifier;
			TransferDirection m_direction;
			P2P::Dispatcher *m_dispatcher;
			Q_INT64 m_offset;
			P2P::MessageFormatter m_messageFormatter;
			QString m_recipient;
			QString m_sender;
			KNetwork::KBufferedSocket *m_socket;
   			CommunicationState m_state;
   			TransferType m_type;
	};
}

#endif
