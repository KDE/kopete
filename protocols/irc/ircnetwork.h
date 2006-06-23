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

namespace IRC
{

typedef struct
{
	QString host;
	uint port;
	QString password;
	bool ssl;
} Host;

typedef struct
{
	QString name;
	QString description;
	QList<Host> hosts;
} Network;

typedef QList<IRC::Network> NetworkList;

class Networks
	: public QObject
{
	Q_OBJECT

private:
	Networks();
	~Networks();

public:
	static IRC::Networks *self();

	const IRC::NetworkList &networks() const;

public slots:
	bool slotReadNetworks();
	bool slotSaveNetworkConfig() const;

//	void addNetwork(const IRCNetwork &network);

private:
	class Private;
	Private * const d;
};

}

#endif
