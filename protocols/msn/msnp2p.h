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


class KopeteMessage;
class MSNAccount;

class KTempFile;

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
	 * parse an incomming message
	 */
	void slotReadMessage( const QByteArray &msg );

public:
	unsigned long int m_msgIdentifier;

signals:
	/**
	 * should be connected to the MSNSwitchBoardSocket's sendCommand function
	 */
	void sendCommand( const QString &cmd, const QString &args = QString::null,
		bool addId = true, const QByteArray &body = QByteArray() , bool binary=false );

private:
	/**
	 * send the MSNSLP command in a msn p2p message
	 */
	void sendP2PMessage( const QCString& dataMessage );

	/**
	 * send the ACK
	 */
	void sendP2PAck( const char * originalHeader) ;

public slots:
	/**
	 * Load the dysplayImage.
	 */
	void requestDisplayPicture( const QString &myHandle, const QString &msgHandle, QString msnObject);
private:
	//for the display image
	KTempFile *m_file;
	unsigned int m_Tsize;

	QString m_myHandle;
	QString m_msgHandle;



};

#endif

