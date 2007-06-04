// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2004 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
//
// gadurichtextformat.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.


#ifndef GADUDCCSERVER_H
#define GADUDCCSERVER_H

#include <qobject.h>
#include <qhostaddress.h>

class QSocketNotifier;
class gg_dcc;

class GaduDCCServer: public QObject {
	Q_OBJECT
public:
	explicit GaduDCCServer( QHostAddress* dccIp = NULL, unsigned int port = 1550 );
	~GaduDCCServer();
	unsigned int listeingPort();

signals:
	void incoming( gg_dcc*, bool& );

private slots:
	void watcher();

private:
	void enableNotifiers( int );
	void disableNotifiers();
	void checkDescriptor();

	void destroyNotifiers();
	void createNotifiers( bool );
	void closeDCC();

	QHostAddress config_dccip;
	QHostAddress config_extip;

	gg_dcc* dccSock;

	QSocketNotifier* read_;
	QSocketNotifier* write_;
};

#endif
