/*
    tlshandler.h - Kopete Groupwise Protocol
      
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
