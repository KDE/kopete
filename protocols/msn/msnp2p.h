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
#include <qstrlist.h>


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

/**
 * @author Olivier Goffart
 *
 * This class help the MSNSwithboardSocket to handle the MSN-P2P messages
 * It displash the message to MSNP2PIncoming or MSNP2POutgoing  classes
 */
class MSNP2PDisplatcher : public MSNP2P
{
	Q_OBJECT

public:
	MSNP2PDisplatcher(  QObject *parent=0L , const char *name=0L);
	~MSNP2PDisplatcher();

	void finished(MSNP2P *);

public slots:
	/**
	 * parse an incoming message
	 */
	void slotReadMessage( const QByteArray &msg );

signals:
	/**
	 * should be connected to the MSNSwitchBoardSocket's sendCommand function
	 */
	void sendCommand( const QString &cmd, const QString &args = QString::null,
		bool addId = true, const QByteArray &body = QByteArray() , bool binary=false );

	void fileReceived( KTempFile * , const QString &msnObject );

private:

	QMap< unsigned long int, MSNP2P* > m_p2pList;


private slots:
	void slotTransferAccepted(Kopete::Transfer*, const QString& );
	void slotFileTransferRefused( const Kopete::FileTransferInfo & );


public slots:
	/**
	 * Load the dysplayImage.
	 */
	void requestDisplayPicture( const QString &myHandle, const QString &msgHandle, QString msnObject);

	/**
	 * Send the following image
	 */
	void sendImage( const QString &fileName);

protected:
	virtual void parseMessage(MessageStruct & );

	//let's access signal dirrectly
	friend class MSNP2P;
	friend class MSNP2PIncoming;
};


class MSNP2PIncoming : public MSNP2P
{
	Q_OBJECT
public:
	MSNP2PIncoming( unsigned long int sessionID , MSNP2PDisplatcher *parent);
	~MSNP2PIncoming( );

	
	virtual void parseMessage(MessageStruct & );
	virtual void error();

	//for the display image
	KTempFile *m_file;
	QFile *m_Rfile;
	QString m_obj;

	QString fullContentMessage;  //used for typewrited images messages

		
	Kopete::Transfer *m_kopeteTransfer;

public slots:
	/**
	 * Abort the current transfer.
	 */
		void abortCurrentTransfer();
		void slotKopeteTransferDestroyed();

};


class MSNP2POutgoing : public MSNP2P
{
	Q_OBJECT
public:
	MSNP2POutgoing( unsigned long int sessionID , MSNP2PDisplatcher *parent);
	~MSNP2POutgoing( );

	virtual void parseMessage(MessageStruct & );

	QFile *m_Sfile;
	QByteArray m_imageToSend;


private slots:
	void slotSendData();
};



#endif

