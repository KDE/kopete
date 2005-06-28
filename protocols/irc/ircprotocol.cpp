/*
    ircprotocol - IRC Protocol

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
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

#include "ircaccount.h"
#include "irccontact.h"
#include "ircprotocol.h"
#include "ksparser.h"

#include "networkconfigwidget.h"
#include "channellist.h"
#include "ircaddcontactpage.h"
#include "ircguiclient.h"
#include "irceditaccountwidget.h"
#include "irctransferhandler.h"

#include "kircengine.h"

#include "kopeteaccountmanager.h"
#include "kopetechatsessionmanager.h"
#include "kopetecommandhandler.h"
#include "kopeteglobal.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteonlinestatus.h"
#include "kopeteview.h"
#include "kopeteuiglobal.h"

#include <kaction.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kuser.h>

#include <qcheckbox.h>
#include <qdom.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qvalidator.h>

#include <dom/html_element.h>
#include <unistd.h>

using namespace Kopete;

typedef KGenericFactory<IRCProtocol> IRCProtocolFactory;
K_EXPORT_COMPONENT_FACTORY(kopete_irc, IRCProtocolFactory("kopete_irc"))

IRCProtocol *IRCProtocol::s_protocol = 0L;

IRCProtocolHandler::IRCProtocolHandler()
	: MimeTypeHandler(false)
{
	registerAsProtocolHandler(QString::fromLatin1("irc"));
}

void IRCProtocolHandler::handleURL(const KURL &url) const
{
	kdDebug(14120) << url << endl;
	if (!url.isValid())
		return;

	unsigned short port = url.port();
	if (port == 0)
		port = 6667;

	QString chan = url.url().section('/',3);
	if (chan.isEmpty())
		return;

	KUser user(getuid());
	QString accountId = QString::fromLatin1("%1@%2:%3").arg(
		user.loginName(),
		url.host(),
		QString::number(port)
	);

	kdDebug(14120) << accountId << endl;

	IRCAccount *newAccount = new IRCAccount( accountId, chan );
	newAccount->setNickName( user.loginName() );
	newAccount->setUserName( user.loginName() );
	newAccount->connect();
}

IRCProtocol::IRCProtocol( QObject *parent, const char *name, const QStringList & /* args */ )
	: Protocol(IRCProtocolFactory::instance(), parent, name),
	  m_StatusUnknown(OnlineStatus::Unknown, 999, this, 999, "status_unknown", i18n("Status not available"))
{
//	kdDebug(14120) << k_funcinfo << endl;

	s_protocol = this;

	initOnlineStatus();

	//m_status = m_unknownStatus = m_Unknown;

	addAddressBookField("messaging/irc", Plugin::MakeIndexField);

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("raw"),
		SLOT( slotRawCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /raw <text> - Sends the text in raw form to the server."), 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("quote"),
		SLOT( slotQuoteCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /quote <text> - Sends the text in quoted form to the server."), 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("ctcp"),
		SLOT( slotCtcpCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /ctcp <nick> <message> - Send the CTCP message to nick<action>."), 2 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("ping"),
		SLOT( slotPingCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /ping <nickname> - Alias for /CTCP <nickname> PING."), 1, 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("motd"),
		SLOT( slotMotdCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /motd [<server>] - Shows the message of the day for the current or the given server.") );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("list"),
		SLOT( slotListCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /list - List the public channels on the server.") );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("join"),
		SLOT( slotJoinCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /join <#channel 1> [<password>] - Joins the specified channel."), 1, 2 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("topic"),
		SLOT( slotTopicCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /topic [<topic>] - Sets and/or displays the topic for the active channel.") );

	//FIXME: Update help text
	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("whois"),
		SLOT( slotWhoisCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /whois <nickname> - Display whois info on this user."), 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("whowas"),
		SLOT( slotWhoWasCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /whowas <nickname> - Display whowas info on this user."), 1, 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("who"),
		SLOT( slotWhoCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /who <nickname|channel> - Display who info on this user/channel."), 1, 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("query"),
		SLOT( slotQueryCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /query <nickname> [<message>] - Open a private chat with this user."), 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("mode"),
		SLOT( slotModeCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /mode <channel> <modes> - Set modes on the given channel."), 2 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("nick"),
		SLOT( slotNickCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /nick <nickname> - Change your nickname to the given one."), 1, 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("me"),
		SLOT( slotMeCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /me <action> - Do something."), 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("ame"),
		SLOT( slotAllMeCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /ame <action> - Do something in every open chat."), 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("kick"),
		SLOT( slotKickCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /kick <nickname> [<reason>] - Kick someone from the channel (requires operator status).")
		, 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("ban"),
		SLOT( slotBanCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /ban <mask> - Add someone to this channel's ban list. (requires operator status)."),
		1, 1 );

	CommandHandler::commandHandler()->registerAlias( this, QString::fromLatin1("bannick"),
		QString::fromLatin1("ban %1!*@*"),
		i18n("USAGE: /bannick <nickname> - Add someone to this channel's ban list. Uses the hostmask nickname!*@* (requires operator status)."), CommandHandler::SystemAlias, 1, 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("op"),
		SLOT( slotOpCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /op <nickname 1> [<nickname 2> <...>] - Give channel operator status to someone (requires operator status)."),
		1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("deop"),
		SLOT( slotDeopCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /deop <nickname> [<nickname 2> <...>]- Remove channel operator status from someone (requires operator status)."), 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("voice"),
		SLOT( slotVoiceCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /voice <nickname> [<nickname 2> <...>]- Give channel voice status to someone (requires operator status)."),
		1);

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("devoice"),
		SLOT( slotDevoiceCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /devoice <nickname> [<nickname 2> <...>]- Remove channel voice status from someone (requires operator status)."), 1 );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("quit"),
		SLOT( slotQuitCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /quit [<reason>] - Disconnect from IRC, optionally leaving a message.") );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("part"),
		SLOT( slotPartCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /part [<reason>] - Part from a channel, optionally leaving a message.") );

	CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("invite"),
		SLOT( slotInviteCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /invite <nickname> [<channel>] - Invite a user to join a channel."), 1 );

	CommandHandler::commandHandler()->registerAlias( this, QString::fromLatin1("j"),
		QString::fromLatin1("join %1"),
		i18n("USAGE: /j <#channel 1> [<password>] - Alias for JOIN."),
		CommandHandler::SystemAlias, 1, 2 );

	CommandHandler::commandHandler()->registerAlias( this, QString::fromLatin1("msg"),
		QString::fromLatin1("query %s"),
		i18n("USAGE: /msg <nickname> [<message>] - Alias for QUERY <nickname> <message>."), CommandHandler::SystemAlias, 1 );

	QObject::connect( ChatSessionManager::self(), SIGNAL(aboutToDisplay(Kopete::Message &)),
		this, SLOT(slotMessageFilter(Kopete::Message &)) );

	QObject::connect( ChatSessionManager::self(), SIGNAL( viewCreated( KopeteView* ) ),
		this, SLOT( slotViewCreated( KopeteView* ) ) );

	setCapabilities(Protocol::RichBFormatting | Kopete::Protocol::RichUFormatting | Kopete::Protocol::RichColor);

	m_protocolHandler = new IRCProtocolHandler();

	IRCTransferHandler::self(); // Initiate the transfer handling system.
}

IRCProtocol * IRCProtocol::self()
{
	return s_protocol;
}

IRCProtocol::~IRCProtocol()
{
	delete m_protocolHandler;
}

void IRCProtocol::initOnlineStatus()
{
	OnlineStatus ServerOnline(OnlineStatus::Online, 100, this, 0,
		QString::null, i18n("Online"));

	OnlineStatus ServerOffline(OnlineStatus::Offline, 90, this, 0,
		QString::null, i18n("Offline"));
/*
	m_statusMap.insert(ServerOnline.internalStatus(), ServerOnline);
	m_statusMap.insert(ServerOffline.internalStatus(), ServerOffline);

	ChannelOnline(OnlineStatus::Online, 80, this, EntityType::Channel|EntityType::Online,
		QString::null, i18n("Online")),
	m_statusMap.insert(ChannelOnline.internalStatus(), ChannelOnline);

	ChannelOffline(OnlineStatus::Offline, 70, this, EntityType::Channel|EntityType::OfflineChannel,
		QString::null, i18n("Offline")),
	m_statusMap.insert(ChannelOffline.internalStatus(), ChannelOffline);
*/
	KIRC::EntityStatus status;
	status.type = KIRC::User;

	onlineStatusFor(status, OnlineStatusManager::Offline);

	status.online = true;
	onlineStatusFor(status,  OnlineStatusManager::Online);

	status.mode_a = true;
	onlineStatusFor(status,  OnlineStatusManager::Away);

/*
	OnlineStatus UserOnline(OnlineStatus::Online, 25, this, 0,
		QString::null, i18n("Online"), i18n("Online"), OnlineStatusManager::Online);

	OnlineStatus UserAway(OnlineStatus::Away, 2, this, 0,
		"contact_away_overlay", i18n("Away"), i18n("Away"), OnlineStatusManager::Away);

	OnlineStatus UserConnecting(OnlineStatus::Connecting, 1, this, 0,
		"irc_connecting", i18n("Connecting"));

	OnlineStatus UserOffline(OnlineStatus::Offline, 0, this, 0,
		QString::null, i18n("Offline"), i18n("Offline"), OnlineStatusManager::Offline);
*/
}

OnlineStatus IRCProtocol::onlineStatusFor(const KIRC::EntityStatus &status)
{
	return onlineStatusFor(status, 0);
}

OnlineStatus IRCProtocol::onlineStatusFor(const KIRC::EntityStatus &_status, unsigned categories)
{
	// Only copy the needed status
	KIRC::EntityStatus status;
	status.online = _status.online;
	status.mode_a = _status.mode_a;
//	status.mode_i = _status.mode_i;
	status.mode_o = _status.mode_o;
	status.mode_v = _status.mode_v;
	status.mode_O = _status.mode_O;

	OnlineStatus ret = m_statusMap[status];
	if (ret.status() == OnlineStatus::Unknown)
	{
		kdDebug(14120) << k_funcinfo << "New online status." << endl;

		OnlineStatus::StatusType statusType;
		unsigned weight = 0;
		QStringList overlayIcons;
		QString description;

		if (status.online)
		{
			statusType = OnlineStatus::Online;

			weight += 1;
			description = i18n("Online");
			weight <<= 1;

			// Is operator
			if (status.mode_o || status.mode_O)
			{
				weight += 1;
				overlayIcons << "irc_op";
				description = i18n("Operator");
			}
			weight <<= 1;

			// Is Voiced
			if (status.mode_v)
			{
				weight += 1;
				overlayIcons << "irc_voice";
				description = i18n("Voiced");
			}
			weight <<= 1;

			// Is away
			if (status.mode_a)
			{
				statusType = OnlineStatus::Away;
				weight += 1;
				overlayIcons << "contact_away_overlay";
				description = i18n("Away");
			}
			weight <<= 1;
/*
			// Is Invisible
			if (status.mode_i)
			{
				statusType = OnlineStatus::Invisible;
				weight += 1;
			}
			weight <<= 1;
*/
		}
		else
		{
			statusType = OnlineStatus::Offline;
			description = i18n("Offline");
		}

		OnlineStatus onlineStatus(statusType, weight, this,
			0, overlayIcons, description, description, categories);

		m_statusMap.insert(status, onlineStatus);
	}

	return ret;
}

void IRCProtocol::slotViewCreated(KopeteView *view)
{
	if (view->msgManager()->protocol() == this)
		new IRCGUIClient(view->msgManager());
}

void IRCProtocol::slotMessageFilter(Message &msg)
{
	if (msg.from()->protocol() == this)
	{
		QString messageText = msg.escapedBody();

		//Add right click for channels, only replace text not in HTML tags
		messageText.replace(QRegExp( QString::fromLatin1("(?![^<]+>)(#[^#\\s]+)(?![^<]+>)")), QString::fromLatin1("<span class=\"KopeteLink\" type=\"IRCChannel\">\\1</span>") );

		msg.setBody( messageText, Message::RichText );
	}
}

QPtrList<KAction> *IRCProtocol::customChatWindowPopupActions(const Message &m, DOM::Node &n)
{
	DOM::HTMLElement e = n;

	//isNull checks that the cast was successful
	if (!e.isNull() && !m.to().isEmpty())
	{
		activeNode = n;
		activeAccount = static_cast<IRCAccount*>( m.from()->account() );
//		if (e.getAttribute(QString::fromLatin1("type")) == QString::fromLatin1("IRCChannel"))
//			return activeAccount->contactManager()->findChannel(
//				e.innerText().string() )->customContextMenuActions();
	}

	return 0;
}

AddContactPage *IRCProtocol::createAddContactWidget(QWidget *parent, Account *account)
{
	return new IRCAddContactPage(parent,static_cast<IRCAccount*>(account));
}

KopeteEditAccountWidget *IRCProtocol::createEditAccountWidget(Account *account, QWidget *parent)
{
	return new IRCEditAccountWidget(static_cast<IRCAccount*>(account), parent);
}

Account *IRCProtocol::createNewAccount(const QString &accountId)
{
	return new IRCAccount(accountId);
}

Contact *IRCProtocol::deserializeContact(MetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/*addressBookData*/)
{
	kdDebug(14120) << k_funcinfo << endl;

	QString contactId = serializedData[ "contactId" ];
	QString displayName = serializedData[ "displayName" ];

	if( displayName.isEmpty() )
		displayName = contactId;

	QDict<Account> accounts = AccountManager::self()->accounts( this );
	if( !accounts.isEmpty() )
	{
		Account *a = accounts[ serializedData[ "accountId" ] ];
		if( a )
		{
			a->addContact( contactId, metaContact );
			return a->contacts()[contactId];
		}
		else
			kdDebug(14120) << k_funcinfo << serializedData[ "accountId" ] << " was a contact's account,"
				" but we don't have it in the accounts list" << endl;
	}
	else
		kdDebug(14120) << k_funcinfo << "No accounts loaded!" << endl;

	return 0;
}

void IRCProtocol::slotRawCommand( const QString &args, ChatSession *manager )
{
	if (!args.isEmpty())
	{
		static_cast<IRCAccount*>(manager->account())->engine()->writeRawMessage(args);
	}
	else
	{
		static_cast<IRCAccount*>(manager->account())->appendErrorMessage(
			i18n("You must enter some text to send to the server.") );
	}
}

void IRCProtocol::slotQuoteCommand( const QString &args, ChatSession *manager )
{
	if (!args.isEmpty())
	{
		static_cast<IRCAccount *>(manager->account())->engine()->writeRawMessage(args);
	}
	else
	{
		static_cast<IRCAccount *>(manager->account())->appendErrorMessage(
			i18n("You must enter some text to send to the server.") );
	}
}

void IRCProtocol::slotCtcpCommand(const QString &args, ChatSession *manager)
{
	if (!args.isEmpty())
	{
		QString user = args.section( ' ', 0, 0 );
		QString message = args.section( ' ', 1 );
//		static_cast<IRCAccount*>(manager->account())->engine()->writeCtcpQueryMessage(
//			user, QString::null, message);
	}
}

void IRCProtocol::slotMotdCommand(const QString &args, ChatSession *manager)
{
	QStringList argsList = CommandHandler::parseArguments(args);
	static_cast<IRCAccount*>(manager->account())->engine()->motd(argsList.front());
}

void IRCProtocol::slotPingCommand(const QString &args, ChatSession *manager)
{
	QStringList argsList = CommandHandler::parseArguments(args);
	static_cast<IRCAccount*>(manager->account())->engine()->CtcpRequest_ping(argsList.front());
}

void IRCProtocol::slotListCommand(const QString &/*args*/, ChatSession *manager)
{
//	static_cast<IRCAccount*>(manager->account())->listChannels();
}

void IRCProtocol::slotTopicCommand(const QString &args, ChatSession *manager)
{/*
	ContactPtrList members = manager->members();
	IRCChannelContact *chan = dynamic_cast<IRCChannelContact*>( members.first() );
	if( chan )
	{
		if( !args.isEmpty() )
			chan->setTopic( args );
		else
		{
			static_cast<IRCAccount*>(manager->account())->engine()->
				writeRawMessage(QString::fromLatin1("TOPIC %1").arg(chan->nickName()));
		}
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendErrorMessage(
			i18n("You must be in a channel to use this command.") );
	}*/
}

void IRCProtocol::slotJoinCommand(const QString &arg, ChatSession *manager)
{/*
	QStringList args = CommandHandler::parseArguments( arg );
	if( KIRC::Entity::isChannel(args[0]) )
	{
		IRCChannelContact *chan = static_cast<IRCAccount*>( manager->account() )->contactManager()->findChannel( args[0] );
		if( args.count() == 2 )
			chan->setPassword( args[1] );
		static_cast<IRCAccount*>( manager->account() )->engine()->join(args[0], chan->password());
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendErrorMessage(
			i18n("\"%1\" is an invalid channel. Channels must start with '#', '!', '+', or '&'.")
			.arg(args[0]) );
	}*/
}

void IRCProtocol::slotInviteCommand(const QString &args, ChatSession *manager)
{/*
	IRCChannelContact *c = 0L;
	QStringList argsList = CommandHandler::parseArguments( args );

	if (argsList.count() > 1)
	{
		if( KIRC::Entity::isChannel(argsList[1]) )
		{
			c = static_cast<IRCAccount*>( manager->account() )->contactManager()->
				findChannel( argsList[1] );
		}
		else
		{
			static_cast<IRCAccount*>( manager->account() )->appendErrorMessage(
				i18n("\"%1\" is an invalid channel. Channels must start with '#', '!', '+', or '&'.")
				.arg(argsList[1]) );
		}
	}
	else
	{
		ContactPtrList members = manager->members();
		c = dynamic_cast<IRCChannelContact*>( members.first() );
	}

	if( c && c->manager()->contactOnlineStatus( manager->myself() ) == m_UserStatusOp )
	{
		static_cast<IRCAccount*>( manager->account() )->engine()->invite(argsList[0], c->nickName());
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendErrorMessage(
			i18n("You must be a channel operator to perform this operation.") );
	}*/
}

void IRCProtocol::slotQueryCommand(const QString &args, ChatSession *manager)
{/*
	QString user = args.section( ' ', 0, 0 );
	QString rest = args.section( ' ', 1 );

	if (!KIRC::Entity::isChannel(user))
	{
		IRCUserContact *c = static_cast<IRCAccount*>( manager->account() )->
			contactManager()->findUser( user );
		c->startChat();
		if( !rest.isEmpty() )
		{
			Message msg( c->manager()->myself(), c->manager()->members(), rest,
				Message::Outbound, Message::PlainText, CHAT_VIEW);
			c->manager()->sendMessage(msg);
		}
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendErrorMessage(
			i18n("\"%1\" is an invalid nickname. Nicknames must not start with '#','!','+', or '&'.").arg(user) );
	}*/
}

void IRCProtocol::slotWhoisCommand(const QString &args, ChatSession *manager)
{
	QStringList argsList = CommandHandler::parseArguments(args);
	static_cast<IRCAccount*>(manager->account())->engine()->whois(argsList.first());
	static_cast<IRCAccount*>(manager->account())->setCurrentCommandSource(manager);
}

void IRCProtocol::slotWhoCommand(const QString &args, ChatSession *manager)
{
	QStringList argsList = CommandHandler::parseArguments(args);
//	static_cast<IRCAccount*>(manager->account())->engine()->who(argsList.first());
	static_cast<IRCAccount*>(manager->account())->setCurrentCommandSource(manager);
}

void IRCProtocol::slotWhoWasCommand(const QString &args, ChatSession *manager)
{
	QStringList argsList = CommandHandler::parseArguments(args);
//	static_cast<IRCAccount*>(manager->account())->engine()->whowas(argsList.first());
	static_cast<IRCAccount*>(manager->account())->setCurrentCommandSource(manager);
}

void IRCProtocol::slotQuitCommand(const QString &args, ChatSession *manager)
{
	static_cast<IRCAccount*>(manager->account())->quit(args);
}

void IRCProtocol::slotNickCommand(const QString &args, ChatSession *manager)
{
	QStringList argsList = CommandHandler::parseArguments(args);
	static_cast<IRCAccount*>(manager->account())->engine()->nick(argsList.front());
}

void IRCProtocol::slotModeCommand(const QString &args, ChatSession *manager)
{
	QStringList argsList = CommandHandler::parseArguments(args);
	static_cast<IRCAccount*>( manager->account() )->engine()->mode( argsList.front(),
		args.section( QRegExp(QString::fromLatin1("\\s+")), 1 ) );
}

void IRCProtocol::slotMeCommand(const QString &args, ChatSession *manager)
{
	ContactPtrList members = manager->members();
	static_cast<IRCAccount*>( manager->account() )->engine()->CtcpRequest_action(
		static_cast<const IRCContact*>(members.first())->nickName(), args
	);
}

void IRCProtocol::slotAllMeCommand(const QString &args, ChatSession *)
{
	QValueList<ChatSession*> sessions = ChatSessionManager::self()->sessions();

	for (QValueList<ChatSession*>::iterator it = sessions.begin(); it != sessions.end(); ++it)
	{
		ChatSession *session = *it;
		if( session->protocol() == this )
			slotMeCommand(args, session);
	}
}

void IRCProtocol::slotKickCommand(const QString &args, ChatSession *manager)
{/*
	if (manager->contactOnlineStatus(manager->myself()) == m_UserStatusOp)
	{
		QRegExp spaces(QString::fromLatin1("\\s+"));
		QString nick = args.section(spaces, 0, 0);
		QString reason = args.section(spaces, 1);
		ContactPtrList members = manager->members();
		QString channel = static_cast<IRCContact*>(members.first())->nickName();
		if (KIRC::Entity::isChannel(channel))
			static_cast<IRCAccount*>(manager->account())->engine()->kick(nick, channel, reason);
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendErrorMessage(
			i18n("You must be a channel operator to perform this operation.") );
	} */
}

void IRCProtocol::slotBanCommand(const QString &args, ChatSession *manager)
{/*
	if( manager->contactOnlineStatus( manager->myself() ) == m_UserStatusOp )
	{
		QStringList argsList = CommandHandler::parseArguments( args );
		ContactPtrList members = manager->members();
		IRCChannelContact *chan = static_cast<IRCChannelContact*>( members.first() );
		if( chan && chan->locateUser( argsList.front() ) )
			chan->setMode( QString::fromLatin1("+b %1").arg( argsList.front() ) );
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendErrorMessage(
			i18n("You must be a channel operator to perform this operation.") );
	}*/
}

void IRCProtocol::slotPartCommand( const QString &args, ChatSession *manager )
{/*
	QStringList argsList = CommandHandler::parseArguments(args);
	ContactPtrList members = manager->members();
	IRCChannelContact *chan = static_cast<IRCChannelContact*>(members.first());

	if (chan)
	{
		if(!args.isEmpty())
			static_cast<IRCAccount*>(manager->account())->engine()->part(chan->nickName(), args);
		else
			chan->part();
		manager->view()->closeView();
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendErrorMessage(
			i18n("You must be in a channel to use this command.") );
	}*/
}

void IRCProtocol::slotOpCommand( const QString &args, ChatSession *manager )
{
	simpleModeChange( args, manager, QString::fromLatin1("+o") );
}

void IRCProtocol::slotDeopCommand( const QString &args, ChatSession *manager )
{
	simpleModeChange( args, manager, QString::fromLatin1("-o") );
}

void IRCProtocol::slotVoiceCommand( const QString &args, ChatSession *manager )
{
	simpleModeChange( args, manager, QString::fromLatin1("+v") );
}

void IRCProtocol::slotDevoiceCommand( const QString &args, ChatSession *manager )
{
	simpleModeChange( args, manager, QString::fromLatin1("-v") );
}

void IRCProtocol::simpleModeChange( const QString &args, ChatSession *manager, const QString &mode )
{/*
	if( manager->contactOnlineStatus( manager->myself() ) == m_UserStatusOp )
	{
		QStringList argsList = CommandHandler::parseArguments( args );
		ContactPtrList members = manager->members();
		IRCChannelContact *chan = static_cast<IRCChannelContact*>( members.first() );
		if( chan )
		{
			for( QStringList::iterator it = argsList.begin(); it != argsList.end(); ++it )
			{
				if( chan->locateUser( *it ) )
					chan->setMode( QString::fromLatin1("%1 %2").arg( mode ).arg( *it ) );
			}
		}
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendErrorMessage(
			i18n("You must be a channel operator to perform this operation.") );
	}*/
}

void IRCProtocol::editNetworks(const QString &networkName)
{
	IRCNetworkConfigWidget *netConf = new IRCNetworkConfigWidget(UI::Global::mainWidget(), Qt::WDestructiveClose);
	netConf->editNetworks(networkName);
	netConf->show();
}

#include "ircprotocol.moc"
