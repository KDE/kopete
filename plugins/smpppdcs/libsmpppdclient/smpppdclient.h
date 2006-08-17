/*
	smpppdclient.h
 
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

#ifndef SMPPPDCLIENT_H
#define SMPPPDCLIENT_H

#include <qstringlist.h>

namespace KNetwork {
class KStreamSocket;
};

namespace SMPPPD {

class State;

/**
	@author Heiko Schaefer <heiko@rangun.de>
*/
class Client {
    Client(const Client&);
    Client& operator=(const Client&);

public:
    Client();
    ~Client();

    bool isReady() const;

    bool connect(const QString& server, uint port = 3185);
    void disconnect();

    QStringList getInterfaceConfigurations();
    bool statusInterface(const QString& ifcfg);

    bool isOnline();
    QString serverID() const;
    QString serverVersion() const;

    void setPassword(const QString& password);

private:
    friend class State;

    void changeState(State * newState);
    QStringList read() const;
    void write(const char * cmd);

private:
    State * m_state;
    KNetwork::KStreamSocket * m_sock;
    QString m_serverID;
    QString m_serverVer;
    QString m_password;
};

inline void Client::changeState(State * newState) {
    m_state = newState;
}

inline void Client::setPassword(const QString& password) {
    m_password = password;
}

};

#endif
