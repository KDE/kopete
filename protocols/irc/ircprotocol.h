/*
    ircprotocol.h - IRC Protocol

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

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

#ifndef IRCPROTOCOL_H
#define IRCPROTOCOL_H

#include "kopeteonlinestatus.h"
#include "kopeteprotocol.h"

class KopeteMetaContact;
class AddContactPage;
class KIRC;

class EditAccountWidget;
class KopeteAccount;
class IRCAccount;
class KActionCollection;

class QStringList;
class QWidget;
class KSParser;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 */
class IRCProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	IRCProtocol( QObject *parent, const char *name, const QStringList &args );
	~IRCProtocol();

	/** KopeteProtocol reimplementation */
	virtual AddContactPage *createAddContactWidget(QWidget *parent, KopeteAccount *account);

	/**
	 * Deserialize contact data
	 */
	virtual void deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	virtual EditAccountWidget* createEditAccountWidget(KopeteAccount *account, QWidget *parent);

	virtual KopeteAccount* createNewAccount(const QString &accountId);

	virtual KActionCollection *customChatWindowPopupActions( const KopeteMessage &, DOM::Node & );

	static IRCProtocol *protocol();

	static KopeteOnlineStatus IRCServerOnline() { return s_protocol->m_ServerOnline; };
	static KopeteOnlineStatus IRCServerOffline() { return s_protocol->m_ServerOffline; };

	static KopeteOnlineStatus IRCChannelOnline() { return s_protocol->m_ChannelOnline; };
	static KopeteOnlineStatus IRCChannelOffline() { return s_protocol->m_ChannelOffline; };

	static KopeteOnlineStatus IRCUserOnline() { return s_protocol->m_UserOnline; };
	static KopeteOnlineStatus IRCUserOp() { return s_protocol->m_UserOp; };
	static KopeteOnlineStatus IRCUserVoice() { return s_protocol->m_UserVoice; };
	static KopeteOnlineStatus IRCUserOffline() { return s_protocol->m_UserOffline; };
	static KopeteOnlineStatus IRCUserAway() { return s_protocol->m_UserAway; };
	static KopeteOnlineStatus IRCUserConnecting() { return s_protocol->m_UserConnecting; };

	static KopeteOnlineStatus IRCUnknown() { return s_protocol->m_Unknown; }

private slots:
	void slotMessageFilter( KopeteMessage &msg );

	void slotListCommand( const QString &args, KopeteMessageManager *manager);
	void slotTopicCommand( const QString &args, KopeteMessageManager *manager);
	void slotJoinCommand( const QString &args, KopeteMessageManager *manager);
	void slotNickCommand( const QString &args, KopeteMessageManager *manager);
	void slotWhoisCommand( const QString &args, KopeteMessageManager *manager);
	void slotMeCommand( const QString &args, KopeteMessageManager *manager);
	void slotModeCommand( const QString &args, KopeteMessageManager *manager);
	void slotQueryCommand( const QString &args, KopeteMessageManager *manager);

	void slotKickCommand( const QString &args, KopeteMessageManager *manager);
	void slotBanCommand( const QString &args, KopeteMessageManager *manager);
	void slotOpCommand( const QString &args, KopeteMessageManager *manager);
	void slotDeopCommand( const QString &args, KopeteMessageManager *manager);
	void slotVoiceCommand( const QString &args, KopeteMessageManager *manager);
	void slotDevoiceCommand( const QString &args, KopeteMessageManager *manager);


private:
	static IRCProtocol *s_protocol;

	const KopeteOnlineStatus m_ServerOnline;
	const KopeteOnlineStatus m_ServerOffline;

	const KopeteOnlineStatus m_ChannelOnline;
	const KopeteOnlineStatus m_ChannelOffline;

	const KopeteOnlineStatus m_UserOp;
	const KopeteOnlineStatus m_UserVoice;
	const KopeteOnlineStatus m_UserOnline;
	const KopeteOnlineStatus m_UserAway;
	const KopeteOnlineStatus m_UserConnecting;
	const KopeteOnlineStatus m_UserOffline;

	const KopeteOnlineStatus m_Unknown;

	KActionCollection *mActions;

	void simpleModeChange( const QString &, KopeteMessageManager *, const QString &mode );

	DOM::Node activeNode;
	IRCAccount *activeAccount;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

