// gadusession.h
//
// Copyright (C)  2002  Zack Rusin <zack@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#ifndef GADUSESSION_H
#define GADUSESSION_H

#include <qvaluelist.h>
#include <qhostaddress.h>
#include <qobject.h>
#include <qstring.h>

#include <libgadu.h>

class QSocketNotifier;

class GaduSession : public QObject
{
	Q_OBJECT
public:
	GaduSession( QObject *parent=0, const char* name=0 );
	virtual ~GaduSession();
	bool isConnected() const;
  int  status() const;

public slots:
	void login( struct gg_login_params& p );
	void login( uin_t uin, const QString& password,
							int status=GG_STATUS_INVISIBLE , const QString& statusDescr="" );
	void logoff();
	int  notify( uin_t *userlist, int count );
	int  addNotify( uin_t uin );
	int  removeNotify( uin_t uin );
	int  sendMessage( uin_t recipient, const QString& msg,
										int msgClass );
	int  sendMessageCtcp( uin_t recipient, const QString& msg,
												int msgClass );
	int  changeStatus( int status );
	int  changeStatusDescription( int status, const QString& descr );
	int  ping();

	int  dccRequest( uin_t uin );

signals:
	void error( const QString& title, const QString& message );
	void messageReceived( struct gg_event* );
	void ackReceived( struct gg_event* );
	void notify( struct gg_event* );
	void notifyDescription( struct gg_event* );
	void statusChanged( struct gg_event* );
	void pong();
	void connectionFailed( struct gg_event* );
	void connectionSucceed( struct gg_event* );
	void disconnect();

protected slots:
	void enableNotifiers( int checkWhat );
	void disableNotifiers();
	void checkDescriptor();

private:
	struct gg_session *session_;
	QSocketNotifier   *read_;
	QSocketNotifier   *write_;
  int  currentServer_;
  QValueList<QHostAddress> servers_;
  gg_login_params params_;
};


#endif
