/*
    ircprotocol.h - IRC Protocol

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

#ifndef IRCPROTOCOL_H
#define IRCPROTOCOL_H

#include "kopeteonlinestatus.h"
#include "kopeteprotocol.h"
#include "kopetecontactproperty.h"
#include "kopetemimetypehandler.h"

#include <dom/dom_node.h>
#include <qdict.h>

#define m_protocol (IRCProtocol::protocol())

namespace Kopete
{
class Account;
class MetaContact;
}

class AddContactPage;

class EditAccountWidget;
class IRCAccount;

class QStringList;
class QWidget;
class KopeteView;

class IRCNetwork;
class IRCHost;
class NetworkConfig;

class IRCProtocolHandler : public Kopete::MimeTypeHandler
{
	public:

		IRCProtocolHandler();

		void handleURL( const KURL &url ) const;
};

static const QString CHAT_VIEW( QString::fromLatin1("kopete_chatwindow") );

/**
 * @author Nick Betcher <nbetcher@kde.org>
 */
class IRCProtocol : public Kopete::Protocol
{
	Q_OBJECT

public:
	enum IRCStatus
	{
		Offline        = 1,                 //! An offline user.
		Connecting     = 2,                 //! User that is connecting.
		Away           = 4,                 //! User that is away. May be regular user, voiced user or (server) operator.
		Online         = 8,                 //! This user is online.
		Voiced         = 16,                //! This user is voiced.
		Operator       = 32,                //! This user is a channel operator.
		ServerOperator = 1024,              //! This user is a server operator.
		OfflineChannel = 4096,              //! This channel is offline.
		OnlineChannel  = 8192,              //! This channel is online.
		OfflineServer  = 16384,             //! This server is offline.
		OnlineServer   = 32768              //! This server is online.
	};

	IRCProtocol( QObject *parent, const char *name, const QStringList &args );
	~IRCProtocol();

	/** Kopete::Protocol reimplementation */
	virtual AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *account);

	/**
	 * Deserialize contact data
	 */
	virtual Kopete::Contact *deserializeContact( Kopete::MetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	virtual KopeteEditAccountWidget* createEditAccountWidget(Kopete::Account *account, QWidget *parent);

	virtual Kopete::Account* createNewAccount(const QString &accountId);

	virtual QPtrList<KAction> *customChatWindowPopupActions( const Kopete::Message &, DOM::Node & );

	static IRCProtocol *protocol();

	/**
	 * Maps the given IRC status to Kopete::OnlineStatus.
	 */
	const Kopete::OnlineStatus statusLookup( IRCStatus status ) const;

	const Kopete::OnlineStatus m_ServerStatusOnline;
	const Kopete::OnlineStatus m_ServerStatusOffline;

	const Kopete::OnlineStatus m_ChannelStatusOnline;
	const Kopete::OnlineStatus m_ChannelStatusOffline;

	const Kopete::OnlineStatus m_UserStatusOpVoice;
	const Kopete::OnlineStatus m_UserStatusOpVoiceAway;
	const Kopete::OnlineStatus m_UserStatusOp;
	const Kopete::OnlineStatus m_UserStatusOpAway;
	const Kopete::OnlineStatus m_UserStatusVoice;
	const Kopete::OnlineStatus m_UserStatusVoiceAway;
	const Kopete::OnlineStatus m_UserStatusOnline;
	const Kopete::OnlineStatus m_UserStatusAway;
	const Kopete::OnlineStatus m_UserStatusConnecting;
	const Kopete::OnlineStatus m_UserStatusOffline;

	const Kopete::OnlineStatus m_StatusUnknown;

	// irc channnel-contact properties
	const Kopete::ContactPropertyTmpl propChannelTopic;
	const Kopete::ContactPropertyTmpl propChannelMembers;
	const Kopete::ContactPropertyTmpl propHomepage;

	// irc user-contact properties
	const Kopete::ContactPropertyTmpl propLastSeen;
	const Kopete::ContactPropertyTmpl propUserInfo;
	const Kopete::ContactPropertyTmpl propServer;
	const Kopete::ContactPropertyTmpl propChannels;
	const Kopete::ContactPropertyTmpl propHops;
	const Kopete::ContactPropertyTmpl propFullName;
	const Kopete::ContactPropertyTmpl propIsIdentified;

	bool commandInProgress(){ return m_commandInProgress; }
	void setCommandInProgress( bool ip ) { m_commandInProgress = ip; }

	QDict<IRCNetwork> &networks(){ return m_networks; }
	void addNetwork( IRCNetwork *network );

	void editNetworks( const QString &networkName = QString::null );

signals:
	void networkConfigUpdated( const QString &selectedNetwork );

private slots:
	// FIXME: All the code for managing the networks list should be in another class - Will
	void slotUpdateNetworkConfig();
	void slotUpdateNetworkHostConfig();
	void slotMoveServerUp();
	void slotMoveServerDown();
	void slotSaveNetworkConfig();
	void slotReadNetworks();
	void slotDeleteNetwork();
	void slotDeleteHost();
	void slotNewNetwork();
	void slotRenameNetwork();
	void slotNewHost();
	void slotHostPortChanged( int value );
	// end of network list specific code

	void slotMessageFilter( Kopete::Message &msg );

	void slotRawCommand( const QString &args, Kopete::ChatSession *manager );
	void slotQuoteCommand( const QString &args, Kopete::ChatSession *manager );
	void slotCtcpCommand( const QString &args, Kopete::ChatSession *manager );
	void slotPingCommand( const QString &args, Kopete::ChatSession *manager );

	void slotMotdCommand( const QString &args, Kopete::ChatSession *manager);
	void slotListCommand( const QString &args, Kopete::ChatSession *manager);
	void slotTopicCommand( const QString &args, Kopete::ChatSession *manager);
	void slotJoinCommand( const QString &args, Kopete::ChatSession *manager);
	void slotNickCommand( const QString &args, Kopete::ChatSession *manager);
	void slotWhoisCommand( const QString &args, Kopete::ChatSession *manager);
	void slotWhoWasCommand( const QString &args, Kopete::ChatSession *manager);
	void slotWhoCommand( const QString &args, Kopete::ChatSession *manager);
	void slotMeCommand( const QString &args, Kopete::ChatSession *manager);
	void slotAllMeCommand( const QString &args, Kopete::ChatSession *manager);
	void slotModeCommand( const QString &args, Kopete::ChatSession *manager);
	void slotQueryCommand( const QString &args, Kopete::ChatSession *manager);

	void slotKickCommand( const QString &args, Kopete::ChatSession *manager);
	void slotBanCommand( const QString &args, Kopete::ChatSession *manager);
	void slotOpCommand( const QString &args, Kopete::ChatSession *manager);
	void slotDeopCommand( const QString &args, Kopete::ChatSession *manager);
	void slotVoiceCommand( const QString &args, Kopete::ChatSession *manager);
	void slotDevoiceCommand( const QString &args, Kopete::ChatSession *manager);
	void slotQuitCommand( const QString &args, Kopete::ChatSession *manager);
	void slotPartCommand( const QString &args, Kopete::ChatSession *manager);
	void slotInviteCommand( const QString &args, Kopete::ChatSession *manager);

	void slotViewCreated( KopeteView * );

private:
	static IRCProtocol *s_protocol;

	void simpleModeChange( const QString &, Kopete::ChatSession *, const QString &mode );

	// FIXME: All the code for managing the networks list should be in another class - Will
	void storeCurrentNetwork();
	void storeCurrentHost();

	NetworkConfig *netConf;
	QString m_uiCurrentNetworkSelection;
	QString m_uiCurrentHostSelection;
	// end of network list specific code

	DOM::Node activeNode;
	IRCAccount *activeAccount;

	bool m_commandInProgress;

	QDict<IRCNetwork> m_networks;
	QDict<IRCHost> m_hosts;
	IRCProtocolHandler *m_protocolHandler;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

