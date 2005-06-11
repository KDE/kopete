/*
    kircengine.h - IRC Client

    Copyright (c) 2003-2004 by Michel Hermier <michel.hermier@wanadoo.fr>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCENGINE_H
#define KIRCENGINE_H

#include "kircentity.h"
#include "kircmessage.h"
#include "kircmessageredirector.h"
#include "kirctransfer.h"

#include <kdeversion.h>

// FIXME: Move the following kdedebug class to the *.cpp.
#include <kdebug.h>
#if KDE_VERSION < KDE_MAKE_VERSION( 3, 1, 90 )
#include <kdebugclasses.h>
#endif

#include <qdatetime.h>
#include <qdict.h>
#include <qintdict.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

class QRegExp;

namespace KIRC
{

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class Engine
	: public QObject
{
	Q_OBJECT

//	Q_PROPERTY(QUrl serverURL READ serverURL WRITE setServerURL)

//	Extracted from the base of the serverURL.
//	Q_PROPERTY(bool useSSL);
//	Q_PROPERTY(QString user READ user);
//	Q_PROPERTY(QString password);
//	Q_PROPERTY(QString host READ host);
//	Q_PROPERTY(int port READ host);

//	Extracted from the query of the serverURL.
//	Q_PROPERTY(bool reqsPasswd);
//	Q_PROPERTY(QString name); // real name
//	Q_PROPERTY(QStringList nickList READ nickList WRITE setNickList)
//	Q_PROPERTY(QString nick READ nick)
//	Q_PROPERTY(QStringList portList)

	Q_ENUMS(Status)

public:
	enum Error
	{
		ParsingFailed,
		UnknownCommand,
		UnknownNumericReply,
		InvalidNumberOfArguments,
		MethodFailed
	};

	enum Status
	{
		Idle,
		Connecting,
		Authentifying,
		Connected,
		Closing,
		AuthentifyingFailed,
		Timeout,
		Disconnected
	};

	enum ServerMessageType
	{
		ErrorMessage = -1,
		PrivateMessage,
		InfoMessage,

		MessageOfTheDayMessage,
		MessageOfTheDayCondensedMessage
	};

	Engine( QObject *parent = 0, const char* name = 0 );
	~Engine();

//	QString nick() const;
//	QStringList nickList() const;
//	void setNickList(const QStringList& nickList);

//	QUrl serverURL() const;
//	bool setServerURL(const QUrl &url);

	inline const QString &currentHost() const
		{ return m_Host; };

	inline Q_UINT16 currentPort()
		{ return m_Port; }

	inline const QString &nickName() const
		{ return m_Nickname; };

	inline const QString &password() const
		{ return m_Passwd; }

	inline void setPassword(const QString &passwd)
		{ m_Passwd = passwd; };

	inline const QString &userName() const
		{ return m_Username; }

	void setUserName(const QString &newName);

	void setRealName(const QString &newName);
	inline const QString &realName() const
		{ return m_realName; }

	inline const bool reqsPassword() const
		{ return m_ReqsPasswd; };

	inline void setReqsPassword(bool b)
		{ m_ReqsPasswd = b; };

	const bool useSSL() const { return m_useSSL; };
	void setUseSSL( bool useSSL );

	inline const QTextCodec *codec() const
		{ return defaultCodec; };

	const QTextCodec *codecForNick( const QString &nick ) const;

	inline void setDefaultCodec( QTextCodec* codec )
		{ defaultCodec = codec; };

	void setVersionString(const QString &versionString);
	void setUserString(const QString &userString);
	void setSourceString(const QString &sourceString);
	void connectToServer(const QString &host, Q_UINT16 port, const QString &nickname, bool useSSL = false);

	KExtendedSocket *socket()
		{ return m_sock; };

	inline KIRC::Engine::Status status() const
		{ return m_status; }

	inline bool isDisconnected() const
		{ return m_status == Disconnected || m_status == Idle; }

	inline bool isConnected() const
		{ return m_status == Connected; }

	inline void setCodec( const QString &nick, const QTextCodec *codec )
		{ codecs.replace( nick, codec ); }

	/* Custom CTCP replies handling */
	inline QString &customCtcp( const QString &s )
	{ return customCtcpMap[s];  }

	inline void addCustomCtcp( const QString &ctcp, const QString &reply )
	{ customCtcpMap[ ctcp.lower() ] = reply; }

	KIRC::EntityPtr getEntity(const QString &name);

public slots:
	//Message output
	void writeRawMessage(const QString &message);

	void writeMessage(const QString &message, const QTextCodec *codec = 0 );
	void writeMessage(const QString &command, const QStringList &args,
		const QString &suffix = QString::null, const QTextCodec *codec = 0);

	void writeCtcpMessage(const QString &command, const QString &to, const QString &ctcpMessage);

	void writeCtcpMessage(const QString &command, const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs, const QString &ctcpSuffix = QString::null,
		bool emitRepliedCtcp = true);

	inline void writeCtcpQueryMessage(const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs = QStringList(), const QString &ctcpSuffix = QString::null,
		bool emitRepliedCtcp = true)
		{ return writeCtcpMessage("PRIVMSG", to, suffix, ctcpCommand, ctcpArgs, ctcpSuffix, emitRepliedCtcp); }

	inline void writeCtcpReplyMessage(const QString &to, const QString &ctcpMessage)
		{ writeCtcpMessage("NOTICE", to, ctcpMessage); }

	inline void writeCtcpReplyMessage(const QString &to, const QString &suffix,
		const QString &ctcpCommand, const QStringList &ctcpArgs = QStringList(), const QString &ctcpSuffix = QString::null,
		bool emitRepliedCtcp = true)
		{ return writeCtcpMessage("NOTICE", to, suffix, ctcpCommand, ctcpArgs, ctcpSuffix, emitRepliedCtcp); }

	inline void writeCtcpErrorMessage(const QString &to, const QString &ctcpLine, const QString &errorMsg,
		bool emitRepliedCtcp=true)
		{ return writeCtcpReplyMessage(to, QString::null, "ERRMSG", ctcpLine, errorMsg, emitRepliedCtcp); }

	bool bind(const QString &command, QObject *object, const char *member,
		  int minArgs = KIRC::MessageRedirector::Unknown,
		  int maxArgs = KIRC::MessageRedirector::Unknown,
		  const QString &helpMessage = QString::null);

	bool bind(int id, QObject *object, const char *member,
		  int minArgs = KIRC::MessageRedirector::Unknown,
		  int maxArgs = KIRC::MessageRedirector::Unknown,
		  const QString &helpMessage = QString::null);

	bool bindCtcpQuery(const QString &command, QObject *object, const char *member,
			   int minArgs = KIRC::MessageRedirector::Unknown,
			   int maxArgs = KIRC::MessageRedirector::Unknown,
			   const QString &helpMessage = QString::null);

	bool bindCtcpReply(const QString &command, QObject *object, const char *member,
			   int minArgs = KIRC::MessageRedirector::Unknown,
			   int maxArgs = KIRC::MessageRedirector::Unknown,
			   const QString &helpMessage = QString::null);


	void away(bool isAway, const QString &awayMessage = QString::null);
	void ison(const QStringList &nickList);
	void join(const QString &name, const QString &key);
	void kick(const QString &user, const QString &channel, const QString &reason);
	void list();
	void mode(const QString &target, const QString &mode);
	void motd(const QString &server = QString::null);
	void nick(const QString &newNickname);
	void notice(const QString &target, const QString &message);
	void part(const QString &name, const QString &reason);
	void pass(const QString &password);
	void privmsg(const QString &contact, const QString &message);

	/**
	 * Send a quit message for the given reason.
	 * If now is set to true the connection is closed and no event message is sent.
	 * Therefore setting now to true should only be used while destroying the object.
	 */
	void quit(const QString &reason, bool now=false);

	void topic(const QString &channel, const QString &topic);
	void user(const QString &newUsername, const QString &hostname, const QString &newRealname);
	void user(const QString &newUsername, Q_UINT8 mode, const QString &newRealname);
	void whois(const QString &user);


	/* CTCP commands */
	void CtcpRequestCommand(const QString &contact, const QString &command);
	void CtcpRequest_action(const QString &contact, const QString &message);
	void CtcpRequest_dcc(const QString &, const QString &, unsigned int port, KIRC::Transfer::Type type);
	void CtcpRequest_ping(const QString &target);
	void CtcpRequest_version(const QString &target);

public slots:
	void showInfoDialog();

signals:
	void statusChanged(KIRC::Engine::Status newStatus);
	void internalError(KIRC::Engine::Error, KIRC::Message &);

	void receivedMessage(KIRC::Message &);

	/**
	 * Emit a received message.
	 * The received message could have been translated to your locale.
	 *
	 * @param type the message type.
	 * @param from the originator of the message.
	 * @param to is the list of entities that are related to this message.
	 * @param msg the message (usually translated).
	 *
	 * @note Most of the following numeric messages should be deprecated, and call this method instead.
	 *	 Most of the methods, using it, update KIRC::Entities.
	 *	 Lists based messages are sent via dedicated API, therefore they don't use this.
	 */
	// @param args the args to apply to this message.
	void receivedMessage(	KIRC::Engine::ServerMessageType type,
				const KIRC::EntityPtr &from,
				const KIRC::EntityPtrList &to,
				const QString &msg);

	void successfullyChangedNick(const QString &, const QString &);

	//ServerContact Signals
	void incomingMotd(const QString &motd);
	void incomingNotice(const QString &originating, const QString &message);
	void incomingHostInfo(const QString &servername, const QString &version,
		const QString &userModes, const QString &channelModes);
	void incomingYourHostInfo(const QString &servername, const QString &version,
		const QString &userModes, const QString &channelModes);
	void incomingConnectString(const QString &clients);

	//Channel Contact Signals
	void incomingMessage(const QString &originating, const QString &target, const QString &message);
	void incomingTopicChange(const QString &, const QString &, const QString &);
	void incomingExistingTopic(const QString &, const QString &);
	void incomingTopicUser(const QString &channel, const QString &user, const QDateTime &time);
	void incomingJoinedChannel(const QString &channel,const QString &nick);
	void incomingPartedChannel(const QString &channel,const QString &nick, const QString &reason);
	void incomingNamesList(const QString &channel, const QStringList &nicknames);
	void incomingEndOfNames(const QString &channel);
	void incomingChannelMode(const QString &channel, const QString &mode, const QString &params);
	void incomingCannotSendToChannel(const QString  &channel, const QString &message);
	void incomingChannelModeChange(const QString &channel, const QString &nick, const QString &mode);
	void incomingChannelHomePage(const QString &channel, const QString &url);

	//Contact Signals
	void incomingPrivMessage(const QString &, const QString &, const QString &);
	void incomingQuitIRC(const QString &user, const QString &reason);
	void incomingUserModeChange(const QString &nick, const QString &mode);
	void incomingNoSuchNickname(const QString &nick);

	// CTCP Signals
//	void action(const QString &from, const QString &to, const QString &message);
	void incomingAction(const QString &channel, const QString &originating, const QString &message);
	void incomingPrivAction(const QString &target, const QString &originating, const QString &message);

	//Response Signals
	void incomingUserOnline(const QString &nick);
	void incomingWhoIsUser(const QString &nickname, const QString &username,
		const QString &hostname, const QString &realname);
	void incomingWhoWasUser(const QString &nickname, const QString &username,
		const QString &hostname, const QString &realname);
	void incomingWhoIsServer(const QString &nickname, const QString &server, const QString &serverInfo);
	void incomingWhoIsOperator(const QString &nickname);
	void incomingWhoIsIdentified(const QString &nickname);
	void incomingWhoIsChannels(const QString &nickname, const QString &channel);
	void incomingWhoIsIdle(const QString &nickname, unsigned long seconds); /* 317 */
	void incomingSignOnTime(const QString &nickname, unsigned long seconds); /* 317 */
	void incomingEndOfWhois(const QString &nickname);
	void incomingEndOfWhoWas(const QString &nickname);

	void incomingWhoReply( const QString &nick, const QString &channel, const QString &user, const QString &host,
		const QString &server,bool away, const QString &flag, uint hops, const QString &realName );

	void incomingEndOfWho( const QString &query );

	//Error Message Signals
	void incomingServerLoadTooHigh();
	void incomingNickInUse(const QString &usingNick);
	void incomingNickChange(const QString &, const QString &);
	void incomingFailedServerPassword();
	void incomingFailedChankey(const QString &);
	void incomingFailedChanBanned(const QString &);
	void incomingFailedChanInvite(const QString &);
	void incomingFailedChanFull(const QString &);
	void incomingFailedNickOnLogin(const QString &);
	void incomingNoNickChan(const QString &);
	void incomingWasNoNick(const QString &);

	//General Signals
	void incomingUnknown(const QString &);
	void incomingUnknownCtcp(const QString &);
	void incomingKick(const QString &channel, const QString &nick,
		const QString &nickKicked, const QString &reason);

	void incomingUserIsAway(const QString &nick, const QString &awayMessage);
	void incomingListedChan(const QString &chan, uint users, const QString &topic);
	void incomingEndOfList();

	void incomingCtcpReply(const QString &type, const QString &target, const QString &messageReceived);

private slots:
	void destroyed(KIRC::Entity *entity);

	void slotReadyRead();

	void slotConnected();
	void slotConnectionClosed();
	void error(int errCode = 0);

	void ignoreMessage(KIRC::Message &msg);
	void emitSuffix(KIRC::Message &);

	void error(KIRC::Message &msg);
	void join(KIRC::Message &msg);
	void kick(KIRC::Message &msg);
	void mode(KIRC::Message &msg);
	void nick(KIRC::Message &msg);
	void notice(KIRC::Message &msg);
	void part(KIRC::Message &msg);
	void ping(KIRC::Message &msg);
	void pong(KIRC::Message &msg);
	void privmsg(KIRC::Message &msg);
//	void squit(KIRC::Message &msg);
	void quit(KIRC::Message &msg);
	void topic(KIRC::Message &msg);

	void numericReply_001(KIRC::Message &msg);
	void numericReply_002(KIRC::Message &msg);
	void numericReply_003(KIRC::Message &msg);
	void numericReply_004(KIRC::Message &msg);
	void numericReply_005(KIRC::Message &msg);
	void numericReply_250(KIRC::Message &msg);
	void numericReply_251(KIRC::Message &msg);
	void numericReply_252(KIRC::Message &msg);
	void numericReply_253(KIRC::Message &msg);
	void numericReply_254(KIRC::Message &msg);
	void numericReply_255(KIRC::Message &msg);
	void numericReply_263(KIRC::Message &msg);
	void numericReply_265(KIRC::Message &msg);
	void numericReply_266(KIRC::Message &msg);
	void numericReply_301(KIRC::Message &msg);
	void numericReply_303(KIRC::Message &msg);
//	void numericReply_305(KIRC::Message &msg);
//	void numericReply_306(KIRC::Message &msg);
	void numericReply_307(KIRC::Message &msg);
	void numericReply_311(KIRC::Message &msg);
	void numericReply_312(KIRC::Message &msg);
	void numericReply_313(KIRC::Message &msg);
	void numericReply_314(KIRC::Message &msg);
	void numericReply_315(KIRC::Message &msg);
	void numericReply_317(KIRC::Message &msg);
	void numericReply_318(KIRC::Message &msg);
	void numericReply_319(KIRC::Message &msg);
	void numericReply_320(KIRC::Message &msg);
	void numericReply_322(KIRC::Message &msg);
	void numericReply_323(KIRC::Message &msg);
	void numericReply_324(KIRC::Message &msg);
	void numericReply_328(KIRC::Message &msg);
	void numericReply_329(KIRC::Message &msg);
	void numericReply_331(KIRC::Message &msg);
	void numericReply_332(KIRC::Message &msg);
	void numericReply_333(KIRC::Message &msg);
	void numericReply_352(KIRC::Message &msg);
	void numericReply_353(KIRC::Message &msg);
	void numericReply_366(KIRC::Message &msg);
	void numericReply_369(KIRC::Message &msg);
	void numericReply_372(KIRC::Message &msg);
//	void numericReply_376(KIRC::Message &msg);

	void numericReply_401(KIRC::Message &msg);
	void numericReply_406(KIRC::Message &msg);
	void numericReply_422(KIRC::Message &msg);
	void numericReply_433(KIRC::Message &msg);
	void numericReply_464(KIRC::Message &msg);
	void numericReply_471(KIRC::Message &msg);
	void numericReply_473(KIRC::Message &msg);
	void numericReply_474(KIRC::Message &msg);
	void numericReply_475(KIRC::Message &msg);


	void CtcpQuery_action(KIRC::Message &msg);
	void CtcpQuery_clientinfo(KIRC::Message &msg);
	void CtcpQuery_finger(KIRC::Message &msg);
	void CtcpQuery_dcc(KIRC::Message &msg);
	void CtcpQuery_ping(KIRC::Message &msg);
	void CtcpQuery_source(KIRC::Message &msg);
	void CtcpQuery_time(KIRC::Message &msg);
	void CtcpQuery_userinfo(KIRC::Message &msg);
	void CtcpQuery_version(KIRC::Message &msg);

	void CtcpReply_errmsg(KIRC::Message &msg);
	void CtcpReply_ping(KIRC::Message &msg);
	void CtcpReply_version(KIRC::Message &msg);

private:
	void bindCommands();
	void bindNumericReplies();
	void bindCtcp();

	void setStatus(KIRC::Engine::Status status);
	bool invokeCtcpCommandOfMessage(const QDict<KIRC::MessageRedirector> &map, KIRC::Message &message);

	/*
	 * Methods that handles all the bindings creations.
	 * This methods is used by all the bind(...) methods.
	 */
	bool _bind(QDict<KIRC::MessageRedirector> &dict,
		QString command, QObject *object, const char *member,
		int minArgs, int maxArgs, const QString &helpMessage);

	//Static regexes
	static const QRegExp m_RemoveLinefeeds;

	KIRC::Engine::Status m_status;
	QString m_Host;
	Q_UINT16 m_Port;

//	QUrl serverURL;
//	QUrl currentServerURL;
	QString m_Nickname;
	QString m_Username;
	QString m_realName;
	QString m_Passwd;
	bool m_ReqsPasswd;
	bool m_FailedNickOnLogin;
	bool m_useSSL;

	QValueList<KIRC::Entity *> m_entities;
	KIRC::EntityPtr m_server;
	KIRC::EntityPtr m_self;

	QString m_VersionString;
	QString m_UserString;
	QString m_SourceString;
	QString m_PendingNick;

	QDict<KIRC::MessageRedirector> m_commands;
//	QIntDict<KIRC::MessageRedirector> m_numericCommands;
	QDict<KIRC::MessageRedirector> m_ctcpQueries;
	QDict<KIRC::MessageRedirector> m_ctcpReplies;

	QMap<QString, QString> customCtcpMap;
	QDict<QTextCodec> codecs;
	QTextCodec *defaultCodec;

	KExtendedSocket *m_sock;
};

}

#endif
