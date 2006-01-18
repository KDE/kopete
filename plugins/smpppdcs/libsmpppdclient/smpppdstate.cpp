/*
	smpppdstate.cpp
 
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

#include "smpppdclient.h"
#include "smpppdstate.h"

using namespace SMPPPD;

State::State() {}

State::~State() {}

QStringList State::read(Client * client) const {
	return client->read();
}

void State::write(Client * client, const char * cmd) {
	client->write(cmd);
}

void State::changeState(Client * client, State * state) {
	client->changeState(state);
}

KNetwork::KStreamSocket * State::socket(Client * client) const {
	return client->m_sock;
}

QString State::password(Client * client) const {
	return client->m_password;
}

void State::setPassword(Client * client, const QString& pass) {
	client->m_password = pass;
}

void State::setServerID(Client * client, const QString& id) {
	client->m_serverID = id;
}

void State::setServerVersion(Client * client, const QString& ver) {
	client->m_serverVer = ver;
}

void State::setSocket(Client * client, KNetwork::KStreamSocket * sock) {
	client->m_sock = sock;
}

bool State::connect(Client * /* client */, const QString& /* server */, uint /* port */) {
	return false;
}

void State::disconnect(Client * /* client */) {}

QStringList State::getInterfaceConfigurations(Client * /* client */) {
	return QStringList();
}

bool State::statusInterface(Client * /* client */, const QString& /* ifcfg */) {
	return false;
}
