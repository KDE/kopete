// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
// gadusession.h
//
// Copyright (C)	2002	Zack Rusin <zack@kde.org>
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#ifndef GADUSESSION_H
#define GADUSESSION_H

#include <qvaluelist.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qstring.h>

#include <libgadu.h>



struct resLine{
    QString uin;
    QString firstname;
    QString surname;
    QString nickname;
    QString age;
    QString city;
    int     status;
};

typedef QPtrList<resLine> searchResult;


class QSocketNotifier;

class GaduSession : public QObject
{
	Q_OBJECT
public:
	GaduSession( QObject *parent=0, const char* name=0 );
	virtual ~GaduSession();
	bool isConnected() const;
	int	 status() const;

public slots:
	void login( struct gg_login_params& p );
	void login( uin_t uin, const QString& password, bool useTls,
							int status=GG_STATUS_INVISIBLE , const QString& statusDescr="" );
	void logoff();
	int	 notify( uin_t *userlist, int count );
	int	 addNotify( uin_t uin );
	int	 removeNotify( uin_t uin );
	int	 sendMessage( uin_t recipient, const unsigned char *message, int msgClass );
	int	 sendMessageCtcp( uin_t recipient, const QString& msg,
												int msgClass );
	int	 changeStatus( int status );
	int	 changeStatusDescription( int status, const QString& descr );
	int	 ping();

	int	 dccRequest( uin_t uin );

  /*
   *  Initiates search in public directory, we need to be logged on to perform search !
   *  This returns false, if you are unable to search (fe you are not logged on, you don't have memory)
   *  This does not checks parametrs !
   *  Calling this function more times with the same params, will continue this search as long as
   *  @ref pubDirSearchClose() will not be called
   *  You must set @ref pubDirSearchResult() signal before calling this function, otherwise no result
   *  will be returned
   */
    bool pubDirSearch(QString &name, QString &surname, QString &nick, 
			    int UIN, QString &city, int gender, 
			    int ageFrom, int ageTo, bool onlyAlive);
                            
  /*
   *  Releases all allocated memory needed to perform search.
   *  This will be done on each @ref pubDirNewSearch(), if previuos is not released
   */
    void pubDirSearchClose();
  
signals:
	void error( const QString& title, const QString& message );
	void messageReceived( struct gg_event* );
	void ackReceived( struct gg_event* );
	void notify( struct gg_event* );
	void statusChanged( struct gg_event* );
	void pong();
	void connectionFailed( struct gg_event* );
	void connectionSucceed( struct gg_event* );
	void disconnect();
	void pubDirSearchResult( const searchResult & );
  
  
protected slots:
	void enableNotifiers( int checkWhat );
	void disableNotifiers();
	void checkDescriptor();

private:

	void sendResult( gg_pubdir50_t result );

	struct gg_session *session_;
	QSocketNotifier		*read_;
	QSocketNotifier		*write_;
	gg_login_params params_;
	int searchSeqNr_;

};


#endif
