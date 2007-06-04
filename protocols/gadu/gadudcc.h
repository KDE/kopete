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


#ifndef GADUDCC_H
#define GADUDCC_H

#include <qobject.h>
#include <QMap>

class QString;
class gg_dcc;
class GaduDCCTransaction;
class GaduAccount;
class GaduDCCServer;

class GaduDCC: public QObject {
	Q_OBJECT
public:
	GaduDCC(  QObject* parent = 0 );
	~GaduDCC();
	bool unregisterAccount();
	bool registerAccount( GaduAccount* );
	unsigned int listeingPort();
	void unset();
	void execute();
	GaduAccount* account( unsigned int );
	
	QMap<unsigned int,QString> requests;	
signals:
	void dccConnect( GaduDCCTransaction* dccTransaction );

private slots:
	void slotIncoming( gg_dcc*, bool& );

private:
	void closeDCC();
	bool unregisterAccount( unsigned int );

	unsigned int accountId;

	static GaduDCCServer* dccServer;

	static volatile unsigned int referenceCount;
};

#endif
