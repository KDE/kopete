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

#include "kircengine.h"

#include "ircnetwork.h"

#include "kopetepasswordedaccount.h"

class IRCContact;
class IRCProtocol;

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
	KIRC::Engine *engine() const;

	QTextCodec *codec() const;
	void setCodec( QTextCodec *codec );
/*
	const IRCNetwork &network();
	void setNetwork(const IRCNetwork &network);
*/
	const QStringList connectCommands() const;

	void setConnectCommands( const QStringList & ) const;

public:

	QMap< QString, QString > customCtcp() const;

	void setCustomCtcpReplies( const QMap< QString, QString > &replys ) const;

	const QMap<QString, QString> customCtcpReplies() const;

	void setCurrentCommandSource( Kopete::ChatSession *session );

	Kopete::ChatSession *currentCommandSource();

	IRCContact *getContact(const QString &name, Kopete::MetaContact *metac=0);
	IRCContact *getContact(KIRC::EntityPtr entity, Kopete::MetaContact *metac=0);

public slots:

	virtual KActionMenu *actionMenu();

	virtual void setAway( bool isAway, const QString &awayMessage = QString::null );

	virtual bool isConnected();

	virtual void connectWithPassword( const QString & );

	virtual void disconnect() { quit(); } // REMOVE ME ASAP

	/** Reimplemented from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus& status , const QString &reason = QString::null);

	// Returns the Kopete::Contact of the user
	IRCContact *mySelf() const;

	// Returns the Kopete::Contact of the server of the user
	IRCContact *myServer() const;

	void successfullyChangedNick(const QString &, const QString &);


public slots:
	void quit( const QString &quitMessage = QString::null );

//	void appendMessage( const QString &message, MessageType type = Default );
	void appendErrorMessage( const QString &message );
	void appendInternalMessage( const QString &message );

protected:
	virtual bool createContact( const QString &contactId, Kopete::MetaContact *parentContact ) ;

private slots:
	void engineConnectionStateChanged(KIRC::ConnectionState newstate);

	void destroyed(IRCContact *contact);

	void receivedMessage(	KIRC::MessageType type,
				const KIRC::EntityPtr &from,
				const KIRC::EntityPtrList &to,
				const QString &msg);

	void slotPerformOnConnectCommands();

	void slotFailedServerPassword();

	void slotShowServerWindow();

private:
	void setupEngine();

private:
	Kopete::ChatSession *m_manager;
	QString autoConnect;

	KIRC::Engine *m_engine;
	IRCNetwork m_network;
	uint currentHost;

	QValueList<IRCContact *> m_contacts;
	IRCContact *m_server;
	IRCContact *m_self;

	Kopete::OnlineStatus m_expectedOnlineStatus;
	QString m_expectedReason;

	QMap<QString, QString> m_customCtcp;
	Kopete::ChatSession *m_commandSource;

	Kopete::AwayAction *m_awayAction;

	KAction *m_joinChannelAction;
	KAction *m_searchChannelAction;
};

#endif
