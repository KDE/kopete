/*
    msnp2p.h - msn p2p protocol

    Copyright (c) 2003-2005 by Olivier Goffart        <ogoffart@ kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNP2P_H
#define MSNP2P_H

#include <qobject.h>
#include <q3strlist.h>

#define MSN_WEBCAM 0
#define MSN_NEWFILETRANSFER 0


namespace Kopete { class Transfer; }
namespace Kopete { struct FileTransferInfo; }

class KTempFile;
class QFile;
class MSNP2PDisplatcher;

/**
 * @author Olivier Goffart
 * base class for all MSNP2P* classes
 */
class MSNP2P : public QObject
{
   Q_OBJECT
public:

	struct MessageStruct
	{
		unsigned int dataMessageSize;
		unsigned int totalSize;
		unsigned int dataOffset;
		QByteArray message;
	};


	MSNP2P( unsigned long int sessionID , MSNP2PDisplatcher *parent);
	MSNP2P( QObject* parent=0L);
	virtual ~MSNP2P();

	enum MessageType { BYE, OK, DECLINE, ERROR, INVITE };

	/**
	 *  you should call the base when reimplementing
	 * */
	virtual void parseMessage(MessageStruct & );

	/**
	 * send the MSNSLP command in a msn p2p message
	 * dataMessage cen be a QCString in case of text message
	 */
	void sendP2PMessage( const QByteArray& dataMessage );

	/**
	 * make and send a P2P message.
	 * @ref sendP2PMessage is used to send the message.
	 *
	 * @param content is the content of the message. it must be terminated by two \r\n\r\n
	 */
	void makeMSNSLPMessage( MessageType type, QString content );

	/**
	 * send the ACK
	 */
	void sendP2PAck( const char * originalHeader) ;

	/**
	 *
	 */
	virtual void error();


	unsigned long int m_msgIdentifier;
	unsigned long int m_sessionId;
	unsigned long int m_totalDataSize;
	unsigned long int m_offset;

	/**
	 * the footer
	 * default: 0
	 * we're transfered a file or a picture : \1
	 * we're transfering an image: \3
	 */
	char m_footer;

	QString m_CallID;
	QString m_branch;

	QString m_myHandle;
	QString m_msgHandle;

	MSNP2PDisplatcher *m_parent;

};


#if MSN_WEBCAM
class MSNP2PWebcam : public MSNP2P
{
//Q_OBJECT
public:
	MSNP2PWebcam( unsigned long int sessionID , MSNP2PDisplatcher *parent);
	~MSNP2PWebcam( );

	virtual void parseMessage(MessageStruct & );
	virtual void error();

	//this should be moved in MSNP2P
	void sendBigP2PMessage( const QByteArray& dataMessage );

//private:
	void makeSIPMessage(const QString &message);
	QString m_content;

};
#endif



#endif

