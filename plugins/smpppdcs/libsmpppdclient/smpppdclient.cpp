/*
	smpppdclient.cpp
 
	Copyright (c) 2006      by Heiko Schaefer        <heiko@rangun.de>
 
	Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; version 2 of the License.               *
	*                                                                       *
	*************************************************************************
*/

#include <kstreamsocket.h>

#include "smpppdunsettled.h"
#include "smpppdclient.h"

using namespace SMPPPD;

Client::Client()
        : m_state(NULL), m_sock(NULL), m_serverID(QString::null), m_serverVer(QString::null), m_password(QString::null) {
    changeState(Unsettled::instance());
}

Client::~Client() {
    disconnect();
}

bool Client::connect(const QString& server, uint port) {
    return m_state->connect(this, server, port);
}

void Client::disconnect() {
    m_state->disconnect(this);
}

QStringList Client::getInterfaceConfigurations() {
    return m_state->getInterfaceConfigurations(this);
}

bool Client::statusInterface(const QString& ifcfg) {
    return m_state->statusInterface(this, ifcfg);
}

QString Client::serverID() const {
    return m_serverID;
}

QString Client::serverVersion() const {
    return m_serverVer;
}

QStringList Client::read() const {
    QStringList qsl;

    if(isReady()) {
        QDataStream stream(m_sock);
        char s[1024];

        stream.readRawBytes(s, 1023);
        char *sp = s;

        for(int i = 0; i < 1024; i++) {
            if(s[i] == '\n') {
                s[i] = 0;
                qsl.push_back(sp);
                sp = &(s[i+1]);
            }
        }
    }

    return qsl;
}

void Client::write(const char * cmd) {
    if(isReady()) {
        QDataStream stream(m_sock);
        stream.writeRawBytes(cmd, strlen(cmd));
        stream.writeRawBytes("\n", strlen("\n"));
        m_sock->flush();
    }
}

bool Client::isReady() const {
    return m_sock && m_sock->state() == KNetwork::KStreamSocket::Connected;
}

bool Client::isOnline() {
	
    if(isReady()) {
        QStringList ifcfgs = getInterfaceConfigurations();
        for(uint i = 0; i < ifcfgs.count(); i++) {
            if(statusInterface(ifcfgs[i])) {
                return true;
            }
        }
    }

    return false;
}
