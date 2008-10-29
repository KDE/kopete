/*
    Kopete Groupwise Protocol
    connector.cpp - the Groupwise socket connector

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

#include "connector.h"

Connector::Connector(QObject *parent)
:QObject(parent)
{
	setUseSSL(false);
	setPeerAddressNone();
}

Connector::~Connector()
{
}

bool Connector::useSSL() const
{
	return ssl;
}

bool Connector::havePeerAddress() const
{
	return haveaddr;
}

QHostAddress Connector::peerAddress() const
{
	return addr;
}

quint16 Connector::peerPort() const
{
	return port;
}

void Connector::setUseSSL(bool b)
{
	ssl = b;
}

void Connector::setPeerAddressNone()
{
	haveaddr = false;
	addr = QHostAddress();
	port = 0;
}

void Connector::setPeerAddress(const QHostAddress &_addr, quint16 _port)
{
	haveaddr = true;
	addr = _addr;
	port = _port;
}

#include "connector.moc"
