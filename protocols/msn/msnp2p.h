/*
    msnp2p.h - msn p2p protocol

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

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


class KopeteTransfer;
struct KopeteFileTransferInfo;

class KTempFile;
class QFile;

/**
 * @author Olivier Goffart
 *
 * This class help the MSNSwithboardSocket to handle the MSN-P2P messages
 */
class MSNP2P : public QObject
{
	Q_OBJECT

public:
	MSNP2P(  QObject *parent=0L , const char *name=0L);
	~MSNP2P();

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

	enum MessageType { BYE, OK, DECLINE, ERROR, INVITE };

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





private slots:
	void slotSendData();
	void slotTransferAccepted(KopeteTransfer*, const QString& );
	void slotFileTransferRefused( const KopeteFileTransferInfo & );
	void slotKopeteTransferDestroyed();

public slots:
	/**
	 * Load the dysplayImage.
	 */
	void requestDisplayPicture( const QString &myHandle, const QString &msgHandle, QString msnObject);

	/**
	 * Abort the current transfer.
	 */
	void abortCurrentTransfer();

private:
	//for the display image
	KTempFile *m_file;
	QFile *m_Sfile;
	QFile *m_Rfile;
	QString m_obj;
	
	unsigned long int m_msgIdentifier;
	unsigned long int m_sessionId;
	unsigned long int m_totalDataSize;
	unsigned long int m_offset;
	
	QString m_CallID;
	QString m_branch;


	QString m_myHandle;
	QString m_msgHandle;
	KopeteTransfer *m_kopeteTransfer;

};

#endif

