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

#include <dom/dom_node.h>

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

	virtual KopeteEditAccountWidget* createEditAccountWidget(KopeteAccount *account, QWidget *parent);

	virtual KopeteAccount* createNewAccount(const QString &accountId);

	virtual KActionCollection *customChatWindowPopupActions( const KopeteMessage &, DOM::Node & );
	
	virtual bool supportsRichText();

	static IRCProtocol *protocol();

	const KopeteOnlineStatus m_ServerStatusOnline;
	const KopeteOnlineStatus m_ServerStatusOffline;

	const KopeteOnlineStatus m_ChannelStatusOnline;
	const KopeteOnlineStatus m_ChannelStatusOffline;

	const KopeteOnlineStatus m_UserStatusOp;
	const KopeteOnlineStatus m_UserStatusVoice;
	const KopeteOnlineStatus m_UserStatusOnline;
	const KopeteOnlineStatus m_UserStatusAway;
	const KopeteOnlineStatus m_UserStatusConnecting;
	const KopeteOnlineStatus m_UserStatusOffline;

	const KopeteOnlineStatus m_StatusUnknown;

private slots:
	void slotMessageFilter( KopeteMessage &msg );

	void slotRawCommand( const QString &args, KopeteMessageManager *manager );
	void slotQuoteCommand( const QString &args, KopeteMessageManager *manager );
	void slotCtcpCommand( const QString &args, KopeteMessageManager *manager );

	void slotMotdCommand( const QString &args, KopeteMessageManager *manager);
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
	void slotQuitCommand( const QString &args, KopeteMessageManager *manager);
	void slotPartCommand( const QString &args, KopeteMessageManager *manager);
	void slotInviteCommand( const QString &args, KopeteMessageManager *manager);

private:
	static IRCProtocol *s_protocol;

	KActionCollection *mActions;

	void simpleModeChange( const QString &, KopeteMessageManager *, const QString &mode );

	DOM::Node activeNode;
	IRCAccount *activeAccount;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

