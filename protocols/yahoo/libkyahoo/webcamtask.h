/*
    Kopete Yahoo Protocol
    Handles incoming webcam connections

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

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

class QString;
class QBuffer;
namespace KNetwork {
	class KStreamSocket;
}
using namespace KNetwork;

enum ConnectionStatus{ InitialStatus, ConnectedStage1, ConnectedStage2, Receiving };
enum PacketType { Image = 0x02, ConnectionClosed = 0x07 }; 

struct YahooWebcamInformation
{
	QString		sender;
	QString		server;
	QString		key;
	ConnectionStatus status;
	PacketType	type;
	uchar		reason;
	Q_INT32		dataLength;
	Q_INT32		timestamp;
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
	bool forMe( Transfer* transfer ) const;
	
	void requestWebcam( const QString &who );
	void closeWebcam( const QString &who );
signals:
	void webcamNotAvailable( const QString & );
	void webcamClosed( const QString &, int );
	void webcamPaused( const QString& );
	void webcamImageReceived( const QString &, const QPixmap &);
private slots:
	void slotConnectionStage1Established();
	void slotConnectionStage2Established();
	void slotConnectionFailed(int);
	void slotRead();

private:
	void parseWebcamInformation( Transfer *transfer );

	void connectStage2( KStreamSocket *socket );
	void processData( KStreamSocket *socket );
	void cleanUpConnection( KStreamSocket *socket );	

	QString keyPending;	// the buddy we have requested the webcam from
	SocketInfoMap socketMap;
};

#endif
