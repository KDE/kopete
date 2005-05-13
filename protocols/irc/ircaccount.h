/*
    ircaccount.h - IRC Account

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

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

#include "kopetepasswordedaccount.h"

#include <kdialogbase.h>

#include <qstring.h>
#include <qstringlist.h>

class ChannelListDialog;

class IRCContact;
class IRCChannelContact;
class IRCContactManager;
class IRCServerContact;
class IRCProtocol;
class IRCUserContact;

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

struct IRCHost
{
	QString host;
	uint port;
	QString password;
	bool ssl;
};

struct IRCNetwork
{
	QString name;
	QString description;
	QValueList<IRCHost*> hosts;
};

class IRCAccount
	: public Kopete::PasswordedAccount
{
	friend class IRCEditAccountWidget;
	friend class IRCProtocolHandler;

	Q_OBJECT

public:
	static const QString CONFIG_CODECMIB;
	static const QString CONFIG_NETWORKNAME;
	static const QString CONFIG_NICKNAME;
	static const QString CONFIG_USERNAME;
	static const QString CONFIG_REALNAME;

	enum MessageType
	{
		ConnectReply = 1,
		InfoReply = 2,
		NoticeReply = 4,
		ErrorReply = 8,
		UnknownReply = 16,
		Default = 32
	};

	enum MessageDestination
	{
		ActiveWindow = 1,
		ServerWindow = 2,
		AnonymousWindow = 3,
		KNotify = 4,
		Ignore = 5
	};

	IRCAccount(IRCProtocol *p, const QString &accountid, const QString &autoConnect = QString::null,
			const QString& networkName = QString::null, const QString &nickName = QString::null);
	virtual ~IRCAccount();

	void setNickName( const QString & );

	void setAutoShowServerWindow( bool show );

	void setAltNick( const QString & );
	const QString altNick() const;

	void setUserName( const QString & );
	const QString userName() const;

	void setRealName( const QString & );
	const QString realName() const;

	const QStringList connectCommands() const;

	void setConnectCommands( const QStringList & ) const;

	void setDefaultPart( const QString & );

	void setNetwork( const QString & );

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

	IRCContactManager *contactManager() const { return m_contactManager; }

	// Returns the Kopete::Contact of the user
	IRCUserContact *mySelf() const;

	// Returns the Kopete::Contact of the server of the user
	IRCServerContact *myServer() const;

	void successfullyChangedNick(const QString &, const QString &);

	virtual void connectWithPassword( const QString & );
	virtual void disconnect();

	void quit( const QString &quitMessage = QString::null );

	void listChannels();

	void appendMessage( const QString &message, MessageType type = Default );

protected:
	virtual bool createContact( const QString &contactId, Kopete::MetaContact *parentContact ) ;

private slots:
	void engineStatusChanged(KIRC::Engine::Status newStatus);

	void destroyed(IRCContact *contact);

	void slotFailedServerPassword();
	void slotGoAway( const QString &reason );
	void slotJoinNamedChannel( const QString &channel );
	void slotJoinChannel();
	void slotShowServerWindow();
	void slotNickInUse( const QString &nick );
	void slotNickInUseAlert( const QString &nick );
	void slotServerBusy();
	void slotNoSuchNickname( const QString &nick );
	void slotSearchChannels();
	void slotNewCtcpReply(const QString &type, const QString &target, const QString &messageReceived);
	void slotJoinedUnknownChannel( const QString &channel, const QString &nick );
	void slotPerformOnConnectCommands();

private:
	Kopete::ChatSession *m_manager;
	QString mNickName;
	Kopete::AwayAction *mAwayAction;
	bool triedAltNick;
	bool autoShowServerWindow;
	QString autoConnect;

	KIRC::Engine *m_engine;
	IRCNetwork *m_network;
	uint currentHost;
	QTextCodec *mCodec;

	MessageDestination m_serverNotices;
	MessageDestination m_serverMessages;
	MessageDestination m_informationReplies;
	MessageDestination m_errorMessages;

	ChannelListDialog *m_channelList;

	QValueList<IRCContact *> m_contacts;
	IRCContactManager *m_contactManager;
	IRCServerContact *m_myServer;

	QMap< QString, QString > m_customCtcp;
	Kopete::ChatSession *commandSource;

	KAction *m_joinChannelAction;
	KAction *m_searchChannelAction;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

