/*
    ircaccount.h - IRC Account

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

#ifndef IRCACCOUNT_H
#define IRCACCOUNT_H

#include "ircnetwork.h"

#include "kircsocket.h"

#include "kopetepasswordedaccount.h"

class IRCContact;
class IRCProtocol;

namespace KIRC
{
class Client;
class Event;
}

namespace Kopete
{
class AwayAction;
class Contact;
class Message;
class ChatSession;
class MetaContact;
}

class KAction;
class KActionMenu;

class IRCAccount
	: public Kopete::PasswordedAccount
{
	Q_OBJECT

	Q_PROPERTY(int codecMib READ codecMib WRITE setCodecFromMib)
	Q_PROPERTY(QString networkName READ networkName WRITE setNetworkByName)

	Q_PROPERTY(QString userName READ userName WRITE setUserName)
	Q_PROPERTY(QString realName READ realName WRITE setRealName)
//	Q_PROPERTY(QString password READ password WRITE setPassword)

	Q_PROPERTY(QString nickName READ nickName WRITE setNickName)
//	Q_PROPERTY(QStringList nickNames READ nickNames WRITE setNickNames)

	Q_PROPERTY(QString defaultPartMessage READ defaultPartMessage WRITE setDefaultPartMessage)
	Q_PROPERTY(QString defaultQuitMessage READ defaultQuitMessage WRITE setDefaultQuitMessage)

	Q_PROPERTY(bool autoShowServerWindow READ autoShowServerWindow WRITE setAutoShowServerWindow)

public:
	IRCAccount(const QString &accountid, const QString &autoConnect = QString::null,
			const QString& networkName = QString::null, const QString &nickName = QString::null);
	virtual ~IRCAccount();

public: // READ properties accessors.

	int codecMib() const;

	const QString networkName() const;

	const QString userName() const;

	const QString realName() const;

	const QString nickName() const;

//	const QString altNick() const;

	const QString defaultPartMessage() const;

	const QString defaultQuitMessage() const;

	bool autoShowServerWindow() const;

public slots: // WRITE properties accessors.

	void setCodecFromMib(int mib);

	void setNetworkByName( const QString & );

	void setUserName( const QString & );

	void setRealName( const QString & );

	void setNickName( const QString & );

//	void setAltNick( const QString & );

	void setDefaultPartMessage( const QString & );

	void setDefaultQuitMessage( const QString & );

	void setAutoShowServerWindow( bool autoShow );

public:
	// Returns the KIRC engine instance
	KIRC::Client *client() const;

	QTextCodec *codec() const;
	void setCodec( QTextCodec *codec );

//	IRCNetwork network();

	const QStringList connectCommands() const;

	void setConnectCommands( const QStringList & ) const;

public:

	QMap< QString, QString > customCtcp() const;

	void setCustomCtcpReplies( const QMap< QString, QString > &replys ) const;

	const QMap<QString, QString> customCtcpReplies() const;

	void setCurrentCommandSource( Kopete::ChatSession *session );

	Kopete::ChatSession *currentCommandSource();

	IRCContact *getContact(const QByteArray &name, Kopete::MetaContact *metac=0);
	IRCContact *getContact(const KIRC::Entity::Ptr &entity, Kopete::MetaContact *metac=0);

	virtual bool isConnected();

	virtual KActionMenu *actionMenu();

	/** Reimplemented from Kopete::Account */
	virtual void setOnlineStatus( const Kopete::OnlineStatus& status , const QString &reason = QString::null);

	// Returns the Kopete::Contact of the user
	IRCContact *mySelf() const;

	// Returns the Kopete::Contact of the server of the user
	IRCContact *myServer() const;

public slots:

	virtual void setAway( bool isAway, const QString &awayMessage = QString::null );

	virtual void connectWithPassword( const QString & );

	virtual void disconnect() { quit(); } // REMOVE ME ASAP

public slots:
	void quit( const QString &quitMessage = QString::null );

protected:
	virtual bool createContact( const QString &contactId, Kopete::MetaContact *parentContact ) ;

private slots:
	void clientConnectionStateChanged(KIRC::Socket::ConnectionState newstate);

	void destroyed(IRCContact *contact);

	void receivedEvent(KIRC::Event *event);

	void slotPerformOnConnectCommands();

	void slotShowServerWindow();

private:
	void clientSetup();
	void clientConnect();

private:
	Q_DISABLE_COPY(IRCAccount)

	class Private;
	Private * const d;
};

#endif
