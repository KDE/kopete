/*
    kirc.h - IRC Client

    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003      by Michel Hermier <michel.hermier@wanadoo.fr>
    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRC_H
#define KIRC_H

#include <qdict.h>
#include <qintdict.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kdeversion.h>

#include <kdebug.h>
#if KDE_VERSION < KDE_MAKE_VERSION( 3, 1, 90 )
#include <kdebugclasses.h>
#endif

#include "kirctransfer.h"
#include "kircmessage.h"

class QTimer;
class QRegExp;

class KIRCMethodFunctorCall;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 * @author Jason Keirstead <jason@keirstead.org>
 */
class KIRC : public QObject
{
	Q_OBJECT

public:
	typedef enum EngineError
	{
		ParsingFailed,
		UnknownCommand,
		UnknownNumericReply,
		InvalidNumberOfArguments,
		MethodFailed
	};

	typedef enum EngineStatus
	{
		Disconnected = 0,
		Connecting = 1,
		Authentifying = 2,
		Connected = 3,
		Closing = 4
	};

	KIRC( QObject *parent = 0, const char* name = 0 );
	~KIRC();

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

	EngineStatus status() const
		{ return m_status; }

	inline bool isDisconnected() const
		{ return m_status == Disconnected; }

	inline bool isConnected() const
		{ return m_status == Connected; }

	inline void setCodec( const QString &nick, const QTextCodec *codec )
		{ codecs.replace( nick, codec ); }

	inline static bool isChannel( const QString &s )
		{ return isChannelRegex.exactMatch(s); };

	/**
	 * Send a quit message for the given reason.
	 * If now is set to true the connection is closed and no event message is sent.
	 * Therefore setting now to true should only be used while destroying the object.
	 */
	void quitIRC(const QString &reason, bool now=false);

	/* IRC commands with numeric replies only */
	void isOn(const QStringList &nickList); /* 303 */
	void setAway(bool isAway, const QString &awayMessage = QString::null); /* 301-305-306 */
	void whoisUser(const QString &user); /* 311-312-313-317-318-319 */
	void list(); /* 321-322-323 */
	void motd(const QString &server = QString::null); /* 372-375-376 */

	/* IRC usual commands */
	void changeUser(const QString &newUsername, const QString &hostname, const QString &newRealname);
	void changeUser(const QString &newUsername, Q_UINT8 mode, const QString &newRealname);
	void changeNickname(const QString &newNickname);
	void sendNotice(const QString &target, const QString &message);
	void changeMode(const QString &target, const QString &mode);
	void joinChannel(const QString &name, const QString &key);
	void messageContact(const QString &contact, const QString &message);
	void setTopic(const QString &channel, const QString &topic);
	void kickUser(const QString &user, const QString &channel, const QString &reason);
	void partChannel(const QString &name, const QString &reason);

	/* CTCP commands */
	void CtcpRequestCommand(const QString &contact, const QString &command);
	void CtcpRequest_action(const QString &contact, const QString &message);
	void CtcpRequest_dcc(const QString &, const QString &, unsigned int port, KIRCTransfer::Type type);
	void CtcpRequest_pingPong(const QString &target);
	void CtcpRequest_version(const QString &target);

	/* Custom CTCP replies handling */
	inline QString &customCtcp( const QString &s )
		{ return customCtcpMap[s];  }

	inline void addCustomCtcp( const QString &ctcp, const QString &reply )
		{ customCtcpMap[ ctcp.lower() ] = reply; }

	//Message output
	KIRCMessage writeRawMessage( const QString &message, bool mustBeConnected = true );

	KIRCMessage writeMessage(const QString &message, bool mustBeConnected = true);

	KIRCMessage writeMessage( const QString &command, const QString &arg,
		const QString &suffix = QString::null, bool mustBeConnected = true );

	KIRCMessage writeCtcpMessage(const QString &command, const QString &to, const QString &suffix,
			const QString &ctcpCommand, const QString &ctcpArgs, const QString &ctcpSuffix = QString::null,
			bool emitRepliedCtcp = true);

	inline KIRCMessage writeCtcpQueryMessage(const QString &to, const QString &suffix,
			const QString &ctcpCommand, const QString &ctcpArg = QString::null,
			const QString &ctcpSuffix = QString::null, bool emitRepliedCtcp = true)
		{ return writeCtcpMessage("PRIVMSG", to, suffix, ctcpCommand, ctcpArg, ctcpSuffix, emitRepliedCtcp); }

	inline KIRCMessage writeCtcpReplyMessage(const QString &to, const QString &suffix,
			const QString &ctcpCommand, const QString &ctcpArg = QString::null,
			const QString &ctcpSuffix = QString::null, bool emitRepliedCtcp = true)
		{ return writeCtcpMessage("NOTICE", to, suffix, ctcpCommand, ctcpArg, ctcpSuffix, emitRepliedCtcp); }

	inline KIRCMessage writeCtcpErrorMessage(const QString &to, const QString &ctcpLine,
			const QString &errorMsg, bool emitRepliedCtcp=true)
		{ return writeCtcpReplyMessage(to, QString::null, "ERRMSG", ctcpLine, errorMsg, emitRepliedCtcp); }

public slots:
	void showInfoDialog();

signals:
	//Engine Signals
	void connectedToServer();
	void disconnected();
	void successfulQuit();
	void connectionTimeout();
	void internalError(KIRC::EngineError, const KIRCMessage &);
	void statusChanged(KIRC::EngineStatus newStatus);
	void sentMessage(const KIRCMessage &);
	void receivedMessage(const KIRCMessage &);
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
	void incomingExistingTopic(const QString &, const QString &); /* 332 */
	void incomingTopicUser(const QString &channel, const QString &user, const QDateTime &time); /*333*/
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
	void incomingAction(const QString &originating, const QString &target, const QString &message);
	void incomingUserModeChange(const QString &nick, const QString &mode);

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
	void incomingPrivAction(const QString &, const QString &, const QString &);
	void incomingKick(const QString &channel, const QString &nick,
		const QString &nickKicked, const QString &reason);

	void incomingUserIsAway(const QString &nick, const QString &awayMessage);
	void incomingListedChan(const QString &chan, uint users, const QString &topic);
	void incomingEndOfList();

	void repliedCtcp(const QString &type, const QString &ctcpMessage);
	void incomingCtcpReply(const QString &type, const QString &target, const QString &messageReceived);

private slots:
	void slotConnected();
	void slotConnectionClosed();
	void slotAuthFailed();
	void slotReadyRead();
	void error(int errCode = 0);
	void quitTimeout();

private:
	void registerCtcpMethods();
	void registerNumericReplies();
	void setStatus(EngineStatus status);
	bool canSend( bool mustBeConnected ) const;
	bool invokeCtcpCommandOfMessage(const KIRCMessage &message, const QDict<KIRCMethodFunctorCall> &map);

	//Joins a bunch of strings with " "
	inline static const QString join( const QString &arg1, const QString &arg2 = QString::null,
		const QString &arg3 = QString::null, const QString &arg4 = QString::null,
		const QString &arg5 = QString::null )
	{
		QStringList ret( arg1 );
		ret << arg2 << arg3 << arg4 << arg5;
		return ret.join( QChar(' ') ).stripWhiteSpace();
	}

	typedef bool ircMethod(const KIRCMessage &msg);
	typedef bool (KIRC::*pIrcMethod)(const KIRCMessage &msg);

	//KIRCMethodFunctor registration
	void addIrcMethod(QDict<KIRCMethodFunctorCall> &map, const char *str, KIRCMethodFunctorCall *method);

	void addIrcMethod(QDict<KIRCMethodFunctorCall> &map, const char *str, pIrcMethod method,
			int argsSize_min=-1, int argsSize_max=-1, const char *helpMessage=0);

	void addNumericIrcMethod(int id, KIRCMethodFunctorCall *method);

	void addNumericIrcMethod(int id, pIrcMethod method, int argsSize_min=-1,
		int argsSize_max=-1, const char *helpMessage=0);

	inline void addIrcMethod(const char *str, KIRCMethodFunctorCall *method)
		{ addIrcMethod(m_IrcMethods, str, method); }

	inline void addIrcMethod(const char *str, pIrcMethod method, int argsSize_min=-1,
			int argsSize_max=-1, const char *helpMessage=0)
		{ addIrcMethod(m_IrcMethods, str, method, argsSize_min, argsSize_max, helpMessage); }

	inline void addCtcpQueryIrcMethod(const char *str, KIRCMethodFunctorCall *method)
		{ addIrcMethod(m_IrcCTCPQueryMethods, str, method); }

	inline void addCtcpQueryIrcMethod( const char *str, pIrcMethod method, int argsSize_min=-1,
			int argsSize_max=-1, const char *helpMessage=0)
		{ addIrcMethod(m_IrcCTCPQueryMethods, str, method, argsSize_min, argsSize_max, helpMessage); }

	inline void addCtcpReplyIrcMethod(const char *str, KIRCMethodFunctorCall *method)
		{ addIrcMethod(m_IrcCTCPReplyMethods, str, method); }

	inline void addCtcpReplyIrcMethod( const char *str, pIrcMethod method, int argsSize_min=-1,
			int argsSize_max=-1, const char *helpMessage=0)
		{ addIrcMethod(m_IrcCTCPReplyMethods, str, method, argsSize_min, argsSize_max, helpMessage); }

	ircMethod nickChange;
	ircMethod notice;
	ircMethod joinChannel;
	ircMethod modeChange;
	ircMethod topicChange;
	ircMethod privateMessage;
	ircMethod kick;
	ircMethod partChannel;
	ircMethod ping;
	ircMethod pong;
	ircMethod quitIRC;

	ircMethod numericReply_001;
	ircMethod numericReply_004;

	ircMethod numericReply_251;
	ircMethod numericReply_252;
	ircMethod numericReply_253;
	ircMethod numericReply_254;
	ircMethod numericReply_265;
	ircMethod numericReply_266;

	ircMethod numericReply_303;
	ircMethod numericReply_305;
	ircMethod numericReply_306;
	ircMethod numericReply_307;
	ircMethod numericReply_311;
	ircMethod numericReply_312;
	ircMethod numericReply_314;
	ircMethod numericReply_317;
	ircMethod numericReply_319;
	ircMethod numericReply_322;
	ircMethod numericReply_324;
	ircMethod numericReply_328;
	ircMethod numericReply_329;
	ircMethod numericReply_331;
	ircMethod numericReply_332;
	ircMethod numericReply_333;
	ircMethod numericReply_352;
	ircMethod numericReply_353;
	ircMethod numericReply_372;
	ircMethod numericReply_376;

	ircMethod numericReply_433;
	ircMethod numericReply_464;
	ircMethod numericReply_471;
	ircMethod numericReply_473;
	ircMethod numericReply_474;
	ircMethod numericReply_475;

	ircMethod CtcpQuery_action;
	ircMethod CtcpQuery_clientInfo;
	ircMethod CtcpQuery_finger;
	ircMethod CtcpQuery_dcc;
	ircMethod CtcpQuery_pingPong;
	ircMethod CtcpQuery_source;
	ircMethod CtcpQuery_time;
	ircMethod CtcpQuery_userInfo;
	ircMethod CtcpQuery_version;

	ircMethod CtcpReply_errorMsg;
	ircMethod CtcpReply_pingPong;
	ircMethod CtcpReply_version;

	//Static ignore
	static KIRCMethodFunctorCall *IgnoreMethod;

	//Static regexes
	static const QRegExp isChannelRegex;
	static const QRegExp m_RemoveLinefeeds;

	EngineStatus m_status;
	QString m_Host;
	Q_UINT16 m_Port;

	QString m_Username;
	QString m_Realname;
	QString m_Nickname;
	QString m_Passwd;
	bool m_ReqsPasswd;
	bool m_FailedNickOnLogin;
	bool m_useSSL;
	int connectTimeout;

	QString m_VersionString;
	QString m_UserString;
	QString m_SourceString;
	QString m_PendingNick;

	QDict<KIRCMethodFunctorCall> m_IrcMethods;
	QIntDict<KIRCMethodFunctorCall> m_IrcNumericMethods;
	QDict<KIRCMethodFunctorCall> m_IrcCTCPQueryMethods;
	QDict<KIRCMethodFunctorCall> m_IrcCTCPReplyMethods;

	QMap<QString, QString> customCtcpMap;
	QDict<QTextCodec> codecs;
	QTextCodec *defaultCodec;

	KExtendedSocket *m_sock;
};

#endif // KIRC_H

// vim: set noet ts=4 sts=4 sw=4:

