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

#include <qstring.h>
#include <qstringlist.h>
#include <kdialogbase.h>

#include "kopeteaccount.h"

class KAction;
class KActionMenu;
class KopeteAwayAction;

class KopeteMetaContact;
class KopeteMessage;
class KopeteContact;
class KopeteMessageManager;

class KIRC;
class IRCProtocol;
class IRCContactManager;
class IRCChannelContact;
class IRCServerContact;
class IRCUserContact;
class ChannelList;

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

class ChannelListDialog : public KDialogBase
{
	Q_OBJECT

	public:
		ChannelListDialog( KIRC *engine, const QString &caption, QObject *target, const char* slotJoinChan );

		void clear();

		void search();

	private slots:
		void slotChannelDoubleClicked( const QString & );

	private:
		KIRC *m_engine;
		ChannelList *m_list;
};

class IRCAccount:  public KopeteAccount
{
	friend class IRCEditAccountWidget;
	friend class IRCProtocolHandler;

	Q_OBJECT

public:
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
		AnonymousWindow = 4,
		KNotify = 8,
		Ignore = 16
	};

	IRCAccount(IRCProtocol *p, const QString &accountid);
	~IRCAccount();

	const QString userName() const;

	const QStringList connectCommands() const;

	void setConnectCommands( const QStringList & ) const;

	void setDefaultPart( const QString & );

	void setNetwork( const QString & );

	void setDefaultQuit( const QString & );

	void setUserName( const QString & );

	void setNickName( const QString & );

	void setAltNick( const QString & );

	void setCodec( QTextCodec *codec );

	QTextCodec *codec() const;

	const QString defaultPart() const;

	const QString defaultQuit() const;

	const QString altNick() const;

	const QString networkName() const;

	QMap< QString, QString > customCtcp() const;

	void setCustomCtcpReplies( const QMap< QString, QString > &replys ) const;

	const QMap<QString, QString> customCtcpReplies() const;

public slots:

	virtual KActionMenu *actionMenu();

	virtual void setAway( bool isAway, const QString &awayMessage = QString::null );

	virtual bool isConnected();

	// Returns the KIRC engine instance
	KIRC *engine() const { return m_engine; }

	// Returns the IRCProtocol instance for contacts
	IRCProtocol *protocol() const { return m_protocol; }

	IRCContactManager *contactManager() const { return m_contactManager; }

	// Returns the KopeteContact of the user
	IRCUserContact *mySelf() const;

	// Returns the KopeteContact of the server of the user
	IRCServerContact *myServer() const;

	void successfullyChangedNick(const QString &, const QString &);

	virtual void connect();
	virtual void disconnect();

	void quit( const QString &quitMessage = QString::null );

	void listChannels();

	void appendMessage( const QString &message, MessageType type = Default );

protected:
	virtual bool addContactToMetaContact( const QString &contactId, const QString &displayName, KopeteMetaContact *parentContact ) ;

protected slots:
	virtual void loaded();

private slots:
	void slotFailedServerPassword();
	void slotGoAway( const QString &reason );
	void slotJoinNamedChannel( const QString &channel );
	void slotJoinChannel();
	void slotShowServerWindow();
	void slotNickInUse( const QString &nick );
	void slotNickInUseAlert( const QString &nick );
	void slotConnectedToServer();
	void slotDisconnected();
	void slotServerBusy();
	void slotSearchChannels();
	void slotNewCtcpReply(const QString &type, const QString &target, const QString &messageReceived);
	void slotJoinedUnknownChannel( const QString &channel, const QString &nick );

private:
	IRCProtocol *m_protocol;
	KopeteMessageManager *m_manager;
	QString mNickName;
	KopeteAwayAction *mAwayAction;
	bool triedAltNick;

	KIRC *m_engine;
	IRCNetwork *m_network;
	uint currentHost;
	QTextCodec *mCodec;

	ChannelListDialog *m_channelList;

	IRCContactManager *m_contactManager;
	IRCServerContact *m_myServer;

	QMap< QString, QString > m_customCtcp;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

