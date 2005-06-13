/*
    msn p2p protocol

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

#ifndef MSNP2PDISPLATCHER_H
#define MSNP2PDISPLATCHER_H


#include "msnp2p.h"

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

#if MSN_WEBCAM
public:
	/**
	 * Start a webcam transfer
	 */
	 void startWebcam(const QString &myHandle, const QString &msgHandle);
#endif

#if MSN_NEWFILETRANSFER
	/**
	 * send a file.
	 * @param filename must be the name of a file QFile can handle
	 */
	 void sendFile(const QString& filename , unsigned int fileSize, const QString &myHandle, const QString &msgHandle);
#endif

	/**
	 * the key is the MSNObject xml, and the data is the filename
	 */
	QMap<QString,QString> objectList;

protected:
	virtual void parseMessage(MessageStruct & );

	//let's access signal dirrectly
	friend class MSNP2P;
	friend class MSNP2PIncoming;
};

#endif
