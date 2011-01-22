/*
    Kopete Yahoo Protocol
    Handles incoming webcam connections

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef WEBCAMTASK_H
#define WEBCAMTASK_H

#include "task.h"
#include <qmap.h>
#include <qpixmap.h>
#include <qstringlist.h>

class QString;
class YMSGTransfer;
class QBuffer;
namespace KNetwork {
	class KStreamSocket;
}
using namespace KNetwork;

enum ConnectionStatus{ InitialStatus, ConnectedStage1, ConnectedStage2, Receiving, Sending, SendingEmpty };
enum PacketType { Image, ConnectionClosed, UserRequest, NewWatcher, WatcherLeft };
enum Direction { Incoming, Outgoing };

struct YahooWebcamInformation
{
	QString		sender;
	QString		server;
	QString		key;
	ConnectionStatus status;
	PacketType	type;
	Direction	direction;
	uchar		reason;
	qint32		dataLength;
	qint32		timestamp;
	bool		headerRead;
	QBuffer 	*buffer;
};

typedef QMap< KStreamSocket *, YahooWebcamInformation > SocketInfoMap;

/**
@author André Duffeck
*/
class WebcamTask : public Task
{
	Q_OBJECT
public:
	WebcamTask(Task *parent);
	~WebcamTask();
	
	bool take(Transfer *transfer);

	bool transmitting() { return transmittingData; }
	
	void requestWebcam( const QString &who );
	void closeWebcam( const QString &who );
	
	void registerWebcam();
	void sendWebcamImage( const QByteArray &image );
	void addPendingInvitation( const QString &userId );
	void grantAccess( const QString &userId );
	void closeOutgoingWebcam();
signals:
	void webcamNotAvailable( const QString & );
	void webcamClosed( const QString &, int );
	void webcamPaused( const QString& );
	void webcamImageReceived( const QString &, const QPixmap &);
	void readyForTransmission();
	void stopTransmission();
	void viewerJoined( const QString & );
	void viewerLeft( const QString & );
	void viewerRequest( const QString & );
private slots:
	void slotConnectionStage1Established();
	void slotConnectionStage2Established();
	void slotConnectionFailed(int);
	void slotRead();
	void sendEmptyWebcamImage();
	void doPendingInvitations();
	void transmitWebcamImage();
protected:
	virtual bool forMe( const Transfer* transfer ) const;
private:
	void parseWebcamInformation( YMSGTransfer *transfer );
	void parseData( QByteArray &data, KStreamSocket *socket );

	void connectStage2( KStreamSocket *socket );
	void processData( KStreamSocket *socket );
	void cleanUpConnection( KStreamSocket *socket );	

	QStringList keysPending; // time ordered list of users we have requested the webcam from
	SocketInfoMap socketMap;
	bool transmittingData;
	QStringList pendingInvitations;
	QStringList accessGranted;
	int timestamp;
	QByteArray pictureBuffer;
	bool transmissionPending;
};

#endif
