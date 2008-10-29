/*
    Kopete Groupwise Protocol
    connector.h - the Groupwise socket connector

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
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

#ifndef LIBGW_CONNECTOR_H
#define LIBGW_CONNECTOR_H

#include <qhostaddress.h>
#include <qobject.h>
#include "libgroupwise_export.h"

class ByteStream;

class LIBGROUPWISE_EXPORT Connector : public QObject
{
	Q_OBJECT
public:
	Connector(QObject *parent=0);
	virtual ~Connector();

	virtual void connectToServer(const QString &server)=0;
	virtual ByteStream *stream() const=0;
	virtual void done()=0;

	bool useSSL() const;
	bool havePeerAddress() const;
	QHostAddress peerAddress() const;
	quint16 peerPort() const;

signals:
	void connected();
	void error();

protected:
	void setUseSSL(bool b);
	void setPeerAddressNone();
	void setPeerAddress(const QHostAddress &addr, quint16 port);

private:
	bool ssl;
	bool haveaddr;
	QHostAddress addr;
	quint16 port;
};

#endif
