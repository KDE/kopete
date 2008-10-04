/*
    Kopete Oscar Protocol
    connector.h - the Oscar socket connector

    Copyright (c) 2004 Matt Rogers <matt.rogers@kdemail.net>
    Based on code Copyright (c) 2004 SuSE Linux AG <http://www.suse.com>
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LIBKYAHOO_CONNECTOR_H
#define LIBKYAHOO_CONNECTOR_H


#include <qobject.h>
#include "qhostaddress.h"

class ByteStream;

class Connector : public QObject
{
	Q_OBJECT
public:
	Connector(QObject *parent=0);
	virtual ~Connector();

	virtual void connectToServer(const QString &server)=0;
	virtual ByteStream *stream() const=0;
	virtual void done()=0;

	bool havePeerAddress() const;
	QHostAddress peerAddress() const;
	quint16 peerPort() const;

signals:
	void connected();
	void error();

protected:
	void setPeerAddressNone();
	void setPeerAddress(const QHostAddress &addr, quint16 port);

private:
	bool haveaddr;
	QHostAddress addr;
	quint16 port;
};

#endif
