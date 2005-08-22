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

#include <qobject.h>
#include <qvaluelist.h>

struct IRCHost
{
	QString host;
	uint port;
	QString password;
	bool ssl;
};

typedef QValueList<IRCHost> IRCHostList;

struct IRCNetwork
{
	QString name;
	QString description;
	IRCHostList hosts;
};

class IRCNetworkList
	: public QObject
{
	Q_OBJECT

private:
	IRCNetworkList();

public:
	static IRCNetworkList *self();

	QValueList<IRCNetwork> networks() const;

public slots:
	bool slotReadNetworks();
	bool slotSaveNetworkConfig();

//	void addNetwork(const IRCNetwork &network);

private:
	QValueList<IRCNetwork> m_networks;
};

#endif
