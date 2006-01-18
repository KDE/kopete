/*
	smpppdstate.h

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

#ifndef SMPPPDSTATE_H
#define SMPPPDSTATE_H

#include <qstringlist.h>

namespace SMPPPD {

class Client;

/**
	@author Heiko Schaefer <heiko@rangun.de>
*/
class State {
	State(const State&);
	State& operator=(const State&);

public:
    State();
    virtual ~State();

	virtual bool connect(Client * client, const QString& server, uint port = 3185);
	virtual void disconnect(Client * client);
	
	virtual QStringList getInterfaceConfigurations(Client * client);
	virtual bool statusInterface(Client * client, const QString& ifcfg);
	
protected:
	QStringList read(Client * client) const;
	void write(Client * client, const char * cmd);
	void changeState(Client * client, State * state);
	KNetwork::KStreamSocket * socket(Client * client) const;
	void setSocket(Client * client, KNetwork::KStreamSocket * sock);
	QString password(Client * client) const;
	void setPassword(Client * client, const QString& pass);
	void setServerID(Client * client, const QString& id);
	void setServerVersion(Client * client, const QString& ver);
	
};

};

#endif
