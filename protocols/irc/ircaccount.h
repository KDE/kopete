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

#include "ircprotocol.h"

#include "kircengine.h"

#include "ircnetwork.h"

#include "kopetepasswordedaccount.h"

class ChannelListDialog;

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

public:
	IRCAccount(IRCProtocol *p, const QString &accountid, const QString &autoConnect = QString::null,
			const QString& networkName = QString::null, const QString &nickName = QString::null);
	virtual ~IRCAccount();

	void setAutoShowServerWindow( bool show );

	void setNickName(const QString &);
	const QString nickName() const;
/*
	void setAltNick(const QString &);
	const QString altNick() const;
*/
	void setUserName(const QString &);
	const QString userName() const;

	void setRealName(const QString &);
	const QString realName() const;

	const QStringList connectCommands() const;

	void setConnectCommands( const QStringList & ) const;

	void setDefaultPart( const QString & );

//	void setNetwork( const QString & );
	void setNetwork(const IRCNetwork &network);

	void setDefaultQuit( const QString & );

	void setCodec( QTextCodec *codec );

	void setMessageDestinations( int serverNotices, int serverMessages,
		int informationReplies, int errorMessages );

	QTextCodec *codec() const;

	const QString defaultPart() const;

	const QString defaultQuit() const;

	const QString networkName() const;

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

	/** Reimplemented from Kopete::Account */
	void setOnlineStatus( const Kopete::OnlineStatus& status , const QString &reason = QString::null);

	// Returns the KIRC engine instance
	KIRC::Engine *engine() const { return m_engine; }

	// Returns the IRCProtocol instance for contacts
	IRCProtocol *protocol() const { return m_protocol; }

	// Returns the Kopete::Contact of the user
	IRCContact *mySelf() const;

	// Returns the Kopete::Contact of the server of the user
	IRCContact *myServer() const;

	void successfullyChangedNick(const QString &, const QString &);

	virtual void connectWithPassword( const QString & );
	virtual void disconnect();

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
	void slotGoAway( const QString &reason );
//	void slotJoinNamedChannel( const QString &channel );
//	void slotNickInUse( const QString &nick );

	void slotShowServerWindow();

private:
	Kopete::ChatSession *m_manager;
	bool autoShowServerWindow;
	QString autoConnect;

	KIRC::Engine *m_engine;
	IRCNetwork m_network;
	uint currentHost;
	QTextCodec *mCodec;

	ChannelListDialog *m_channelList;

	QValueList<IRCContact *> m_contacts;
	IRCContact *m_server;
	IRCContact *m_self;

	QMap<QString, QString> m_customCtcp;
	Kopete::ChatSession *commandSource;

	Kopete::AwayAction *m_awayAction;

	KAction *m_joinChannelAction;
	KAction *m_searchChannelAction;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

