/*
 * Copy of TLSHandler class found in xmpp.h
 * Copyright (C) 2003  Justin Karneges
 */

#ifndef GWTLSHANDLER_H
#define GWTLSHANDLER_H

#include <qobject.h>
//#include<qstring.h>
//#include<qhostaddress.h>
//#include<qstring.h>
//#include<qcstring.h>
//#include<qxml.h>
//#include<qdom.h>
 
class TLSHandler : public QObject
{
	Q_OBJECT
public:
	TLSHandler(QObject *parent=0);
	virtual ~TLSHandler();

	virtual void reset()=0;
	virtual void startClient(const QString &host)=0;
	virtual void write(const QByteArray &a)=0;
	virtual void writeIncoming(const QByteArray &a)=0;

signals:
	void success();
	void fail();
	void closed();
	void readyRead(const QByteArray &a);
	void readyReadOutgoing(const QByteArray &a, int plainBytes);
};

#endif
