// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Current author and maintainer: Grzegorz Jaskiewicz
//				gj at pointblue.com.pl
//
// Copyright (C) 	2002-2003	 Zack Rusin <zack@kde.org>
//
// gaducommands.h - all basic, and not-session dependent commands
// (meaning you don't have to be logged in for any
//  of these). These delete themselves, meaning you don't
//  have to/can't delete them explicitely and have to create
//  them dynamically (via the 'new' call).
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

#ifndef GADUCOMMANDS_H
#define GADUCOMMANDS_H

#include <libgadu.h>
#include <qobject.h>
#include <qstringlist.h>

class QSocketNotifier;
class QStringList;

class GaduCommand : public QObject
{
	Q_OBJECT
public:
	GaduCommand( QObject *parent=0, const char* name=0 );
	virtual ~GaduCommand();

	virtual void execute()=0;

	bool done() const;

signals:
	//e.g. emit done( i18n("Done"), i18n("Registration complete") );
	void done( const QString& title, const QString& what );
	void error( const QString& title, const QString& error );
	void socketReady();
protected:
	char* qstrToChar( const QString& str );
	void checkSocket( int fd, int checkWhat );
	void enableNotifiers( int checkWhat );
	void disableNotifiers();

	bool done_;
protected slots:
	void forwarder();
private:
	QSocketNotifier *read_;
	QSocketNotifier *write_;
};

//due to a bug in moc. moc will remove the keyword
// "struct" in the signals definitions breaking compilation
typedef struct gg_search gg_search_qt;

class SearchCommand : public GaduCommand
{
	Q_OBJECT
public:
	SearchCommand( QObject *parent=0, const char* name=0 );
	~SearchCommand();

	void searchMode0( const QString& nickname, const QString& first_name,
										const QString& last_name, const QString& city,
										int gender, int min_birth, int max_birth,
										int active, int start );
	void searchMode1( const QString& email, int active, int start );
	void searchMode2( const QString& phone, int active, int start );
	void searchMode3( uin_t uin, int active, int start );
	void execute();
signals:
	void done( gg_search_qt* );
protected slots:
	void watcher();
private:
	const struct gg_search_request *request_;
	struct gg_http	*session_;
};

class RegisterCommand : public GaduCommand
{
	Q_OBJECT
public:
	RegisterCommand( QObject *parent=0, const char* name=0 );
	RegisterCommand( const QString& email, const QString& password ,
									 QObject *parent=0, const char* name=0 );
	~RegisterCommand();

	void setUserinfo( const QString& email, const QString& password );
	void execute();
	unsigned int newUin();
protected slots:
	void watcher();
private:
	QString email_;
	QString password_;
	struct gg_http	*session_;
	int uin;
};

class RemindPasswordCommand : public GaduCommand
{
	Q_OBJECT
public:
	RemindPasswordCommand( uin_t uin, QObject *parent=0, const char* name=0 );
	RemindPasswordCommand( QObject *parent=0, const char* name=0 );
	~RemindPasswordCommand();

	void setUIN( uin_t uin );
	void execute();
protected slots:
	void watcher();
private:
	uin_t uin_;
	struct gg_http	*session_;
};

class ChangePasswordCommand : public GaduCommand
{
	Q_OBJECT
public:
	ChangePasswordCommand( QObject *parent=0, const char* name=0 );
	~ChangePasswordCommand();

	void setInfo( uin_t uin, const QString& passwd, const QString& newpasswd,
								const QString& newemail );
	void execute();
protected slots:
	void watcher();
private:
	struct gg_http	*session_;
	QString passwd_;
	QString newpasswd_;
	QString newemail_;
	uin_t uin_;
};

class ChangeInfoCommand : public GaduCommand
{
	Q_OBJECT
public:
	ChangeInfoCommand( QObject *parent=0, const char* name=0 );
	~ChangeInfoCommand();

	void setInfo( uin_t uin, const QString& passwd,
								const QString& firstName, const QString& lastName,
								const QString& nickname, const QString& email,
								int born, int gender, const QString& city );
	void execute();
protected slots:
	void watcher();
private:
	struct gg_change_info_request info_;
	struct gg_http	*session_;
	uin_t uin_;
	QString passwd_;
};

class UserlistPutCommand : public GaduCommand
{
	Q_OBJECT
public:
	UserlistPutCommand( QObject *parent=0, const char* name=0 );
	UserlistPutCommand( uin_t uin, const QString& password, const QStringList& contacts,
											QObject *parent=0, const char* name=0 );
	~UserlistPutCommand();

	void setInfo( uin_t uin, const QString& password, const QStringList& contacts );
	void execute();
protected slots:
	void watcher();
private:
	struct gg_http	*session_;
	uin_t uin_;
	QString password_;
	QStringList contacts_;
};

class UserlistGetCommand : public GaduCommand
{
	Q_OBJECT
public:
	UserlistGetCommand( QObject *parent=0, const char* name=0 );
	~UserlistGetCommand();

	void setInfo( uin_t uin, const QString& password );
	void execute();
signals:
	void done( const QStringList& );
protected slots:
	void watcher();

private:
	struct gg_http	*session_;
	uin_t uin_;
	QString password_;
};


#endif
