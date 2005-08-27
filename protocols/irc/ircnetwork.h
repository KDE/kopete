/*
    ircnetwork.h - IRC Network List

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2005 by Michel Hermier <michel.hermier@wanadoo.fr>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCNETWORKLIST_H
#define IRCNETWORKLIST_H

#include <QList>
#include <QObject>

struct IRCHost
{
	QString host;
	uint port;
	QString password;
	bool ssl;
};

typedef QList<IRCHost> IRCHostList;

struct IRCNetwork
{
	QString name;
	QString description;
	IRCHostList hosts;
};

typedef QList<IRCNetwork> IRCNetworkList;
/*
class IRCNetworkList
	: public QObject
{
	Q_OBJECT

private:
	IRCNetworkList();

public:
	static IRCNetworkList *self();

	IRCNetworkList networks() const;

public slots:
	bool slotReadNetworks();
	bool slotSaveNetworkConfig();

//	void addNetwork(const IRCNetwork &network);

private:
	IRCNetworkList m_networks;
};
*/
#endif
