// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003-2004 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 2002-2003	 Zack Rusin 	<zack@kde.org>
//
// gaducommands.h - all basic, and not-session dependent commands
// (meaning you don't have to be logged in for any of these).
// These delete themselves, meaning you don't
//  have to/can't delete them explicitly and have to create
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef GADUCOMMANDS_H
#define GADUCOMMANDS_H

#include <libgadu.h>

#include <qobject.h>
#include <QPixmap>

class QSocketNotifier;
class QPixmap;

class GaduCommand : public QObject
{
	Q_OBJECT

public:
	GaduCommand( QObject* parent = 0 );
	virtual ~GaduCommand();

	virtual void execute() = 0;

	bool done() const;

signals:
	//e.g. emit done( i18n("Done"), i18n("Registration complete") );
	void done( const QString& title, const QString& what );
	void error( const QString& title, const QString& error );
	void socketReady();
	void operationStatus( const QString );

protected:
	void checkSocket( int, int );
	void enableNotifiers( int );
	void disableNotifiers();
	void deleteNotifiers();

	bool done_;

protected slots:
	void forwarder();

private:
	QSocketNotifier*	read_;
	QSocketNotifier*	write_;
};

class RegisterCommand : public GaduCommand
{
	Q_OBJECT

public:
	RegisterCommand( QObject* parent = 0 );
	RegisterCommand( const QString& email, const QString& password ,
					QObject* parent = 0 );
	~RegisterCommand();

	void setUserinfo( const QString& email, const QString& password, const QString& token );
	void execute();
	unsigned int newUin();
	void requestToken();
	void cancel();

signals:
	void tokenRecieved( QPixmap, QString );

protected slots:
	void watcher();

private:
	enum RegisterState{ RegisterStateNoToken, RegisterStateWaitingForToken, RegisterStateGotToken, RegisterStateWaitingForNumber, RegisterStateDone };
	RegisterState	state;
	QString			email_;
	QString			password_;
	struct gg_http*	session_;
	int 			uin;
	QString			tokenId;
	QString			tokenString;
};

class RemindPasswordCommand : public GaduCommand
{
	Q_OBJECT

public:
	explicit RemindPasswordCommand( uin_t uin, QObject* parent = 0 );
	RemindPasswordCommand( QObject* parent = 0 );
	~RemindPasswordCommand();

	void setUIN( uin_t );
	void execute();

protected slots:
	void watcher();

private:
	uin_t			uin_;
	struct gg_http*	session_;
};

class ChangePasswordCommand : public GaduCommand
{
	Q_OBJECT

public:
	ChangePasswordCommand( QObject* parent = 0 );
	~ChangePasswordCommand();

	void setInfo( uin_t uin, const QString& passwd, const QString& newpasswd,
				const QString& newemail );
	void execute();

protected slots:
	void watcher();

private:
	struct gg_http*	session_;
	QString			passwd_;
	QString			newpasswd_;
	QString			newemail_;
	uin_t			uin_;
};


#endif
