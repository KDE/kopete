/*
    ircprotocol - IRC Protocol

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

#include "ircaccount.h"
#include "ircprotocol.h"
#include "ksparser.h"

#include "ircaddcontactpage.h"
#include "ircchannelcontact.h"
#include "irccontactmanager.h"

#include "networkconfig.h"
#include "channellist.h"
#include "ircguiclient.h"
#include "ircusercontact.h"
#include "irceditaccountwidget.h"
#include "irctransferhandler.h"

#include "kircengine.h"

#include "kopeteaccountmanager.h"
#include "kopetecommandhandler.h"
#include "kopeteglobal.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteonlinestatus.h"
#include "kopeteview.h"
#include "kopeteuiglobal.h"

#undef KDE_NO_COMPAT
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

typedef KGenericFactory<IRCProtocol> IRCProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_irc, IRCProtocolFactory( "kopete_irc" )  )

IRCProtocol *IRCProtocol::s_protocol = 0L;

IRCProtocolHandler::IRCProtocolHandler() : Kopete::MimeTypeHandler( false )
{
	registerAsProtocolHandler( QString::fromLatin1("irc") );
}

void IRCProtocolHandler::handleURL( const KURL &url ) const
{
	kdDebug(14120) << url << endl;
	if( !url.isValid() )
		return;

	unsigned short port = url.port();
	if( port == 0 )
		port = 6667;

	QString chan = url.url().section('/',3);
	if( chan.isEmpty() )
		return;

	KUser user( getuid() );
	QString accountId = QString::fromLatin1("%1@%2:%3").arg(
		user.loginName(),
		url.host(),
		QString::number(port)
	);

	kdDebug(14120) << accountId << endl;

	IRCAccount *newAccount = new IRCAccount( IRCProtocol::protocol(), accountId, chan );
	newAccount->setNickName( user.loginName() );
	newAccount->setUserName( user.loginName() );
	newAccount->connect();
}

IRCProtocol::IRCProtocol( QObject *parent, const char *name, const QStringList & /* args */ )
: Kopete::Protocol( IRCProtocolFactory::instance(), parent, name ),

	m_ServerStatusOnline(Kopete::OnlineStatus::Online,
			100, this, OnlineServer, QString::null, i18n("Online")),
	m_ServerStatusOffline(Kopete::OnlineStatus::Offline,
			90, this, OfflineServer, QString::null, i18n("Offline")),

	m_ChannelStatusOnline(Kopete::OnlineStatus::Online,
			80, this, OnlineChannel, QString::null, i18n("Online")),
	m_ChannelStatusOffline(Kopete::OnlineStatus::Offline,
			70, this, OfflineChannel, QString::null, i18n("Offline")),

	m_UserStatusOpVoice(Kopete::OnlineStatus::Online,
			60, this, Online | Operator | Voiced, QStringList::split(' ',"irc_voice irc_op"), i18n("Op")),
	m_UserStatusOpVoiceAway(Kopete::OnlineStatus::Away,
			55, this, Online | Operator | Voiced | Away,
			QStringList::split(' ',"irc_voice irc_op contact_away_overlay"), i18n("Away")),

	m_UserStatusOp(Kopete::OnlineStatus::Online,
			50, this, Online | Operator, "irc_op", i18n("Op")),
	m_UserStatusOpAway(Kopete::OnlineStatus::Away,
			45, this, Online | Operator | Away,
			QStringList::split(' ',"irc_op contact_away_overlay"), i18n("Away")),

	m_UserStatusVoice(Kopete::OnlineStatus::Online,
			40, this, Online | Voiced, "irc_voice", i18n("Voice")),
	m_UserStatusVoiceAway(Kopete::OnlineStatus::Away,
			35, this, Online | Voiced | Away,
			QStringList::split(' ',"irc_voice contact_away_overlay"),  i18n("Away")),

	m_UserStatusOnline(Kopete::OnlineStatus::Online,
			25, this, Online, QString::null, i18n("Online"), i18n("Online"), Kopete::OnlineStatusManager::Online),

	m_UserStatusAway(Kopete::OnlineStatus::Away,
			2, this, Online | Away, "contact_away_overlay",
			i18n("Away"), i18n("Away"), Kopete::OnlineStatusManager::Away),
	m_UserStatusConnecting(Kopete::OnlineStatus::Connecting,
			1, this, Connecting, "irc_connecting", i18n("Connecting")),
	m_UserStatusOffline(Kopete::OnlineStatus::Offline,
			0, this, Offline, QString::null, i18n("Offline"), i18n("Offline"), Kopete::OnlineStatusManager::Offline),

	m_StatusUnknown(Kopete::OnlineStatus::Unknown,
			999, this, 999, "status_unknown", i18n("Status not available")),

	propChannelTopic(QString::fromLatin1("channelTopic"), i18n("Topic"), QString::null, false, true ),
	propChannelMembers(QString::fromLatin1("channelMembers"), i18n("Members")),
	propHomepage(QString::fromLatin1("homePage"), i18n("Home Page")),
	propLastSeen(Kopete::Global::Properties::self()->lastSeen()),
	propUserInfo(QString::fromLatin1("userInfo"), i18n("IRC User")),
	propServer(QString::fromLatin1("ircServer"), i18n("IRC Server")),
	propChannels( QString::fromLatin1("ircChannels"), i18n("IRC Channels")),
	propHops(QString::fromLatin1("ircHops"), i18n("IRC Hops")),
	propFullName(QString::fromLatin1("FormattedName"), i18n("Full Name")),
	propIsIdentified(QString::fromLatin1("identifiedUser"), i18n("User Is Authenticated"))
{
//	kdDebug(14120) << k_funcinfo << endl;

	s_protocol = this;

	//m_status = m_unknownStatus = m_Unknown;

	addAddressBookField("messaging/irc", Kopete::Plugin::MakeIndexField);

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("raw"),
		SLOT( slotRawCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /raw <text> - Sends the text in raw form to the server."), 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("quote"),
		SLOT( slotQuoteCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /quote <text> - Sends the text in quoted form to the server."), 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("ctcp"),
		SLOT( slotCtcpCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /ctcp <nick> <message> - Send the CTCP message to nick<action>."), 2 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("ping"),
		SLOT( slotPingCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /ping <nickname> - Alias for /CTCP <nickname> PING."), 1, 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("motd"),
		SLOT( slotMotdCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /motd [<server>] - Shows the message of the day for the current or the given server.") );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("list"),
		SLOT( slotListCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /list - List the public channels on the server.") );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("join"),
		SLOT( slotJoinCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /join <#channel 1> [<password>] - Joins the specified channel."), 1, 2 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("topic"),
		SLOT( slotTopicCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /topic [<topic>] - Sets and/or displays the topic for the active channel.") );

	//FIXME: Update help text
	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("whois"),
		SLOT( slotWhoisCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /whois <nickname> - Display whois info on this user."), 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("whowas"),
		SLOT( slotWhoWasCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /whowas <nickname> - Display whowas info on this user."), 1, 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("who"),
		SLOT( slotWhoCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /who <nickname|channel> - Display who info on this user/channel."), 1, 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("query"),
		SLOT( slotQueryCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /query <nickname> [<message>] - Open a private chat with this user."), 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("mode"),
		SLOT( slotModeCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /mode <channel> <modes> - Set modes on the given channel."), 2 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("nick"),
		SLOT( slotNickCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /nick <nickname> - Change your nickname to the given one."), 1, 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("me"),
		SLOT( slotMeCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /me <action> - Do something."), 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("ame"),
		SLOT( slotAllMeCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /ame <action> - Do something in every open chat."), 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("kick"),
		SLOT( slotKickCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /kick <nickname> [<reason>] - Kick someone from the channel (requires operator status).")
		, 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("ban"),
		SLOT( slotBanCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /ban <mask> - Add someone to this channel's ban list. (requires operator status)."),
		1, 1 );

	Kopete::CommandHandler::commandHandler()->registerAlias( this, QString::fromLatin1("bannick"),
		QString::fromLatin1("ban %1!*@*"),
		i18n("USAGE: /bannick <nickname> - Add someone to this channel's ban list. Uses the hostmask nickname!*@* (requires operator status)."), Kopete::CommandHandler::SystemAlias, 1, 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("op"),
		SLOT( slotOpCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /op <nickname 1> [<nickname 2> <...>] - Give channel operator status to someone (requires operator status)."),
		1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("deop"),
		SLOT( slotDeopCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /deop <nickname> [<nickname 2> <...>]- Remove channel operator status from someone (requires operator status)."), 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("voice"),
		SLOT( slotVoiceCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /voice <nickname> [<nickname 2> <...>]- Give channel voice status to someone (requires operator status)."),
		1);

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("devoice"),
		SLOT( slotDevoiceCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /devoice <nickname> [<nickname 2> <...>]- Remove channel voice status from someone (requires operator status)."), 1 );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("quit"),
		SLOT( slotQuitCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /quit [<reason>] - Disconnect from IRC, optionally leaving a message.") );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("part"),
		SLOT( slotPartCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /part [<reason>] - Part from a channel, optionally leaving a message.") );

	Kopete::CommandHandler::commandHandler()->registerCommand( this, QString::fromLatin1("invite"),
		SLOT( slotInviteCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /invite <nickname> [<channel>] - Invite a user to join a channel."), 1 );

	Kopete::CommandHandler::commandHandler()->registerAlias( this, QString::fromLatin1("j"),
		QString::fromLatin1("join %1"),
		i18n("USAGE: /j <#channel 1> [<password>] - Alias for JOIN."), Kopete::CommandHandler::SystemAlias,
		1, 2 );

	Kopete::CommandHandler::commandHandler()->registerAlias( this, QString::fromLatin1("msg"),
		QString::fromLatin1("query %s"),
		i18n("USAGE: /msg <nickname> [<message>] - Alias for QUERY <nickname> <message>."), Kopete::CommandHandler::SystemAlias, 1 );

	QObject::connect( Kopete::ChatSessionManager::self(), SIGNAL(aboutToDisplay(Kopete::Message &)),
		this, SLOT(slotMessageFilter(Kopete::Message &)) );

	QObject::connect( Kopete::ChatSessionManager::self(), SIGNAL( viewCreated( KopeteView* ) ),
		this, SLOT( slotViewCreated( KopeteView* ) ) );

	setCapabilities( Kopete::Protocol::RichBFormatting | Kopete::Protocol::RichUFormatting | Kopete::Protocol::RichColor );

	netConf = 0L;

	slotReadNetworks();

	m_protocolHandler = new IRCProtocolHandler();

	IRCTransferHandler::self(); // Initiate the transfer handling system.
}

IRCProtocol * IRCProtocol::protocol()
{
	return s_protocol;
}

IRCProtocol::~IRCProtocol()
{
	delete m_protocolHandler;
}

const Kopete::OnlineStatus IRCProtocol::statusLookup( IRCStatus status ) const
{
	kdDebug(14120) << k_funcinfo << "Looking up status for " << status << endl;

	switch( status )
	{
	case Offline:
		return m_UserStatusOffline;
	case Connecting:
		return m_UserStatusConnecting;

	// Regular user
	case Online:
		return m_UserStatusOnline;
	case Online | Away:
		return m_UserStatusAway;

	// Voiced
	case Online | Voiced:
		return m_UserStatusVoice;
	case Online | Away | Voiced:
		return m_UserStatusVoiceAway;

	// Operator
	case Online | Operator:
		return m_UserStatusOp;
	case Online | Away | Operator:
		return m_UserStatusOpAway;
	case Online | Operator | Voiced:
		return m_UserStatusOpVoice;
	case Online | Operator | Voiced | Away:
		return m_UserStatusOpVoiceAway;

	// Server
	case OnlineServer:
		return m_ServerStatusOnline;
	case OfflineServer:
		return m_ServerStatusOffline;

	// Channel
	case OnlineChannel:
		return m_ChannelStatusOnline;
	case OfflineChannel:
		return m_ChannelStatusOffline;

	default:
		return m_StatusUnknown;
	}
}

void IRCProtocol::slotViewCreated( KopeteView *view )
{
	if( view->msgManager()->protocol() == this )
		new IRCGUIClient( view->msgManager() );
}

void IRCProtocol::slotMessageFilter( Kopete::Message &msg )
{
	if( msg.from()->protocol() == this )
	{
		QString messageText = msg.escapedBody();

		//Add right click for channels, only replace text not in HTML tags
		messageText.replace( QRegExp( QString::fromLatin1("(?![^<]+>)(#[^#\\s]+)(?![^<]+>)") ), QString::fromLatin1("<span class=\"KopeteLink\" type=\"IRCChannel\">\\1</span>") );

		msg.setBody( messageText, Kopete::Message::RichText );
	}
}

QPtrList<KAction> *IRCProtocol::customChatWindowPopupActions( const Kopete::Message &m, DOM::Node &n )
{
	DOM::HTMLElement e = n;

	//isNull checks that the cast was successful
	if( !e.isNull() && !m.to().isEmpty() )
	{
		activeNode = n;
		activeAccount = static_cast<IRCAccount*>( m.from()->account() );
		if( e.getAttribute( QString::fromLatin1("type") ) == QString::fromLatin1("IRCChannel") )
			return activeAccount->contactManager()->findChannel(
				e.innerText().string() )->customContextMenuActions();
	}

	return 0L;
}

AddContactPage *IRCProtocol::createAddContactWidget(QWidget *parent, Kopete::Account *account)
{
	return new IRCAddContactPage(parent,static_cast<IRCAccount*>(account));
}

KopeteEditAccountWidget *IRCProtocol::createEditAccountWidget(Kopete::Account *account, QWidget *parent)
{
	return new IRCEditAccountWidget(this, static_cast<IRCAccount*>(account),parent);
}

Kopete::Account *IRCProtocol::createNewAccount(const QString &accountId)
{
	return new IRCAccount( this, accountId );
}

Kopete::Contact *IRCProtocol::deserializeContact( Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> & /* addressBookData */ )
{
	kdDebug(14120) << k_funcinfo << endl;

	QString contactId = serializedData[ "contactId" ];
	QString displayName = serializedData[ "displayName" ];

	if( displayName.isEmpty() )
		displayName = contactId;

	QDict<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts( this );
	if( !accounts.isEmpty() )
	{
		Kopete::Account *a = accounts[ serializedData[ "accountId" ] ];
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

void IRCProtocol::slotRawCommand( const QString &args, Kopete::ChatSession *manager )
{
	IRCAccount *account = static_cast<IRCAccount*>( manager->account() );

	if (!args.isEmpty())
	{
		account->engine()->writeRawMessage(args);
	}
	else
	{
		account->appendMessage(i18n("You must enter some text to send to the server."),
				IRCAccount::ErrorReply );
	}
}

void IRCProtocol::slotQuoteCommand( const QString &args, Kopete::ChatSession *manager )
{
	IRCAccount *account = static_cast<IRCAccount*>( manager->account() );

	if( !args.isEmpty() )
	{
		account->engine()->writeMessage( args );
	}
	else
	{
		account->appendMessage(i18n("You must enter some text to send to the server."),
				IRCAccount::ErrorReply );
	}
}

void IRCProtocol::slotCtcpCommand( const QString &args, Kopete::ChatSession *manager )
{
	if( !args.isEmpty() )
	{
		QString user = args.section( ' ', 0, 0 );
		QString message = args.section( ' ', 1 );
		static_cast<IRCAccount*>( manager->account() )->engine()->writeCtcpQueryMessage( user, QString::null, message );
	}
}

void IRCProtocol::slotMotdCommand( const QString &args, Kopete::ChatSession *manager )
{
	QStringList argsList = Kopete::CommandHandler::parseArguments( args );
	static_cast<IRCAccount*>( manager->account() )->engine()->motd(argsList.front());
}

void IRCProtocol::slotPingCommand( const QString &args, Kopete::ChatSession *manager )
{
	QStringList argsList = Kopete::CommandHandler::parseArguments(args);
	static_cast<IRCAccount*>( manager->account() )->engine()->CtcpRequest_ping(argsList.front());
}

void IRCProtocol::slotListCommand( const QString &/*args*/, Kopete::ChatSession *manager )
{
	static_cast<IRCAccount*>( manager->account() )->listChannels();
}

void IRCProtocol::slotTopicCommand( const QString &args, Kopete::ChatSession *manager )
{
	Kopete::ContactPtrList members = manager->members();
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
		static_cast<IRCAccount*>( manager->account() )->appendMessage(
			i18n("You must be in a channel to use this command."), IRCAccount::ErrorReply );
	}
}

void IRCProtocol::slotJoinCommand( const QString &arg, Kopete::ChatSession *manager )
{
	QStringList args = Kopete::CommandHandler::parseArguments( arg );
	if( KIRC::Entity::isChannel(args[0]) )
	{
		IRCChannelContact *chan = static_cast<IRCAccount*>( manager->account() )->contactManager()->findChannel( args[0] );
		if( args.count() == 2 )
			chan->setPassword( args[1] );
		static_cast<IRCAccount*>( manager->account() )->engine()->join(args[0], chan->password());
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendMessage(
			i18n("\"%1\" is an invalid channel. Channels must start with '#', '!', '+', or '&'.")
			.arg(args[0]), IRCAccount::ErrorReply );
	}
}

void IRCProtocol::slotInviteCommand( const QString &args, Kopete::ChatSession *manager )
{
	IRCChannelContact *c = 0L;
	QStringList argsList = Kopete::CommandHandler::parseArguments( args );

	if( argsList.count() > 1 )
	{
		if( KIRC::Entity::isChannel(argsList[1]) )
		{
			c = static_cast<IRCAccount*>( manager->account() )->contactManager()->
				findChannel( argsList[1] );
		}
		else
		{
			static_cast<IRCAccount*>( manager->account() )->appendMessage(
				i18n("\"%1\" is an invalid channel. Channels must start with '#', '!', '+', or '&'.")
				.arg(argsList[1]), IRCAccount::ErrorReply );
		}
	}
	else
	{
		Kopete::ContactPtrList members = manager->members();
		c = dynamic_cast<IRCChannelContact*>( members.first() );
	}

	if( c && c->manager()->contactOnlineStatus( manager->myself() ) == m_UserStatusOp )
	{
		static_cast<IRCAccount*>( manager->account() )->engine()->writeMessage(
			QString::fromLatin1("INVITE %1 %2").arg( argsList[0] ).
			arg( c->nickName() )
		);
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendMessage(
			i18n("You must be a channel operator to perform this operation."), IRCAccount::ErrorReply );
	}
}

void IRCProtocol::slotQueryCommand( const QString &args, Kopete::ChatSession *manager )
{
	QString user = args.section( ' ', 0, 0 );
	QString rest = args.section( ' ', 1 );

	if( !KIRC::Entity::isChannel(user) )
	{
		IRCUserContact *c = static_cast<IRCAccount*>( manager->account() )->
			contactManager()->findUser( user );
		c->startChat();
		if( !rest.isEmpty() )
		{
			Kopete::Message msg( c->manager()->myself(), c->manager()->members(), rest,
				Kopete::Message::Outbound, Kopete::Message::PlainText, CHAT_VIEW);
			c->manager()->sendMessage(msg);
		}
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendMessage(
			i18n("\"%1\" is an invalid nickname. Nicknames must not start with '#','!','+', or '&'.").arg(user),
			IRCAccount::ErrorReply );
	}
}

void IRCProtocol::slotWhoisCommand( const QString &args, Kopete::ChatSession *manager )
{
	static_cast<IRCAccount*>( manager->account() )->engine()->whois( args );
	static_cast<IRCAccount*>( manager->account() )->setCurrentCommandSource( manager );
}

void IRCProtocol::slotWhoCommand( const QString &args, Kopete::ChatSession *manager )
{
	QStringList argsList = Kopete::CommandHandler::parseArguments( args );
	static_cast<IRCAccount*>( manager->account() )->engine()->writeMessage(
		QString::fromLatin1("WHO %1").arg( argsList.first() ) );
	static_cast<IRCAccount*>( manager->account() )->setCurrentCommandSource( manager );
}

void IRCProtocol::slotWhoWasCommand( const QString &args, Kopete::ChatSession *manager )
{
	QStringList argsList = Kopete::CommandHandler::parseArguments( args );
	static_cast<IRCAccount*>( manager->account() )->engine()->writeMessage(
		QString::fromLatin1("WHOWAS %1").arg( argsList.first() ) );
	static_cast<IRCAccount*>( manager->account() )->setCurrentCommandSource( manager );
}

void IRCProtocol::slotQuitCommand( const QString &args, Kopete::ChatSession *manager )
{
	static_cast<IRCAccount*>( manager->account() )->quit( args );
}

void IRCProtocol::slotNickCommand( const QString &args, Kopete::ChatSession *manager )
{
	QStringList argsList = Kopete::CommandHandler::parseArguments( args );
	static_cast<IRCAccount*>( manager->account() )->engine()->nick( argsList.front() );
}

void IRCProtocol::slotModeCommand(const QString &args, Kopete::ChatSession *manager)
{
	QStringList argsList = Kopete::CommandHandler::parseArguments( args );
	static_cast<IRCAccount*>( manager->account() )->engine()->mode( argsList.front(),
		args.section( QRegExp(QString::fromLatin1("\\s+")), 1 ) );
}

void IRCProtocol::slotMeCommand(const QString &args, Kopete::ChatSession *manager)
{
	Kopete::ContactPtrList members = manager->members();
	static_cast<IRCAccount*>( manager->account() )->engine()->CtcpRequest_action(
		static_cast<const IRCContact*>(members.first())->nickName(), args
	);
}

void IRCProtocol::slotAllMeCommand(const QString &args, Kopete::ChatSession *)
{
	QValueList<Kopete::ChatSession*> sessions = Kopete::ChatSessionManager::self()->sessions();

	for( QValueList<Kopete::ChatSession*>::iterator it = sessions.begin(); it != sessions.end(); ++it )
	{
		Kopete::ChatSession *session = *it;
		if( session->protocol() == this )
			slotMeCommand(args, session);
	}
}

void IRCProtocol::slotKickCommand(const QString &args, Kopete::ChatSession *manager)
{
	if (manager->contactOnlineStatus( manager->myself() ) == m_UserStatusOp)
	{
		QRegExp spaces(QString::fromLatin1("\\s+"));
		QString nick = args.section( spaces, 0, 0);
		QString reason = args.section( spaces, 1);
		Kopete::ContactPtrList members = manager->members();
		QString channel = static_cast<IRCContact*>( members.first() )->nickName();
		if (KIRC::Entity::isChannel(channel))
			static_cast<IRCAccount*>(manager->account())->engine()->kick(nick, channel, reason);
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendMessage(
			i18n("You must be a channel operator to perform this operation."), IRCAccount::ErrorReply );
	}
}

void IRCProtocol::slotBanCommand( const QString &args, Kopete::ChatSession *manager )
{
	if( manager->contactOnlineStatus( manager->myself() ) == m_UserStatusOp )
	{
		QStringList argsList = Kopete::CommandHandler::parseArguments( args );
		Kopete::ContactPtrList members = manager->members();
		IRCChannelContact *chan = static_cast<IRCChannelContact*>( members.first() );
		if( chan && chan->locateUser( argsList.front() ) )
			chan->setMode( QString::fromLatin1("+b %1").arg( argsList.front() ) );
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendMessage(
			i18n("You must be a channel operator to perform this operation."), IRCAccount::ErrorReply );
	}
}

void IRCProtocol::slotPartCommand( const QString &args, Kopete::ChatSession *manager )
{
	QStringList argsList = Kopete::CommandHandler::parseArguments(args);
	Kopete::ContactPtrList members = manager->members();
	IRCChannelContact *chan = static_cast<IRCChannelContact*>(members.first());

	if (chan)
	{
		if(!args.isEmpty())
			static_cast<IRCAccount*>(manager->account())->engine()->part(chan->nickName(), args);
		else
			chan->part();
		if( manager->view() )
			manager->view()->closeView(true);
	}
	else
	{
		static_cast<IRCAccount*>( manager->account() )->appendMessage(
			i18n("You must be in a channel to use this command."), IRCAccount::ErrorReply );
	}
}

void IRCProtocol::slotOpCommand( const QString &args, Kopete::ChatSession *manager )
{
	simpleModeChange( args, manager, QString::fromLatin1("+o") );
}

void IRCProtocol::slotDeopCommand( const QString &args, Kopete::ChatSession *manager )
{
	simpleModeChange( args, manager, QString::fromLatin1("-o") );
}

void IRCProtocol::slotVoiceCommand( const QString &args, Kopete::ChatSession *manager )
{
	simpleModeChange( args, manager, QString::fromLatin1("+v") );
}

void IRCProtocol::slotDevoiceCommand( const QString &args, Kopete::ChatSession *manager )
{
	simpleModeChange( args, manager, QString::fromLatin1("-v") );
}

void IRCProtocol::simpleModeChange( const QString &args, Kopete::ChatSession *manager, const QString &mode )
{
	if( manager->contactOnlineStatus( manager->myself() ) == m_UserStatusOp )
	{
		QStringList argsList = Kopete::CommandHandler::parseArguments( args );
		Kopete::ContactPtrList members = manager->members();
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
		static_cast<IRCAccount*>( manager->account() )->appendMessage(
			i18n("You must be a channel operator to perform this operation."), IRCAccount::ErrorReply );
	}
}

void IRCProtocol::editNetworks( const QString &networkName )
{
	if( !netConf )
	{
		netConf = new NetworkConfig( Kopete::UI::Global::mainWidget(), "network_config", true );
		netConf->host->setValidator( new QRegExpValidator( QString::fromLatin1("^[\\w-\\.]*$"), netConf ) );
		netConf->upButton->setIconSet( SmallIconSet( "up" )  );
		netConf->downButton->setIconSet( SmallIconSet( "down" ) );

		connect( netConf->networkList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkConfig() ) );
		connect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );
		connect( netConf, SIGNAL( accepted() ), this, SLOT( slotSaveNetworkConfig() ) );
		connect( netConf, SIGNAL( rejected() ), this, SLOT( slotReadNetworks() ) );
		connect( netConf->upButton, SIGNAL( clicked() ), this, SLOT( slotMoveServerUp() ) );
		connect( netConf->downButton, SIGNAL( clicked() ), this, SLOT( slotMoveServerDown() ) );
		connect( netConf->removeNetwork, SIGNAL( clicked() ), this, SLOT( slotDeleteNetwork() ) );
		connect( netConf->removeHost, SIGNAL( clicked() ), this, SLOT( slotDeleteHost() ) );
		connect( netConf->newHost, SIGNAL( clicked() ), this, SLOT( slotNewHost() ) );
		connect( netConf->newNetwork, SIGNAL( clicked() ), this, SLOT( slotNewNetwork() ) );
		connect( netConf->renameNetwork, SIGNAL( clicked() ), this, SLOT( slotRenameNetwork() ) );
		connect( netConf->port, SIGNAL( valueChanged( int ) ), this, SLOT( slotHostPortChanged( int ) ) );
		connect( netConf->networkList, SIGNAL( doubleClicked ( QListBoxItem * )), SLOT(slotRenameNetwork()));
				
	}

	disconnect( netConf->networkList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkConfig() ) );
	disconnect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );

	netConf->networkList->clear();

	for( QDictIterator<IRCNetwork> it( m_networks ); it.current(); ++it )
	{
		IRCNetwork *net = it.current();
		netConf->networkList->insertItem( net->name );
	}

	netConf->networkList->sort();

	connect( netConf->networkList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkConfig() ) );
	connect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );

	if( !networkName.isEmpty() )
		netConf->networkList->setSelected( netConf->networkList->findItem( networkName ), true );

	//slotUpdateNetworkConfig(); // unnecessary, setSelected emits selectionChanged

	netConf->show();
}

void IRCProtocol::slotUpdateNetworkConfig()
{
	// update the data structure of the previous selection from the UI
	storeCurrentNetwork();

	// update the UI from the data for the current selection
	IRCNetwork *net = m_networks[ netConf->networkList->currentText() ];
	if( net )
	{
		netConf->description->setText( net->description );
		netConf->hostList->clear();

		for( QValueList<IRCHost*>::iterator it = net->hosts.begin(); it != net->hosts.end(); ++it )
			netConf->hostList->insertItem( (*it)->host + QString::fromLatin1(":") + QString::number((*it)->port) );

		// prevent nested event loop crash
		disconnect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );
		netConf->hostList->setSelected( 0, true );
		slotUpdateNetworkHostConfig();
		connect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );
	}

	// record the current selection
	m_uiCurrentNetworkSelection = netConf->networkList->currentText();
}

void IRCProtocol::storeCurrentNetwork()
{
	if ( !m_uiCurrentNetworkSelection.isEmpty() )
	{
		IRCNetwork *net = m_networks[ m_uiCurrentNetworkSelection ];
		if ( net )
		{
			net->description = netConf->description->text(); // crash on 2nd dialog show here!
		}
		else
			kdDebug( 14120 ) << m_uiCurrentNetworkSelection << " was already gone from the cache!" << endl;
	}
}

void IRCProtocol::storeCurrentHost()
{
	if ( !m_uiCurrentHostSelection.isEmpty()  )
	{
		IRCHost *host = m_hosts[ m_uiCurrentHostSelection ];
		if ( host )
		{
			host->host = netConf->host->text();
			host->password = netConf->password->text();
			host->port = netConf->port->text().toInt();
			host->ssl = netConf->useSSL->isChecked();
		}
	}
}

void IRCProtocol::slotHostPortChanged( int value )
{
	QString entryText = m_uiCurrentHostSelection + QString::fromLatin1(":") + QString::number( value );
	// changeItem causes a take() and insert, and we don't want a selectionChanged() signal that sets all this off again.
	disconnect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );
	netConf->hostList->changeItem( entryText, netConf->hostList->currentItem() );
	connect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );
}

void IRCProtocol::slotUpdateNetworkHostConfig()
{
	storeCurrentHost();

	if ( netConf->hostList->selectedItem() )
	{
		m_uiCurrentHostSelection = netConf->hostList->currentText().section(':', 0, 0);
		IRCHost *host = m_hosts[ m_uiCurrentHostSelection ];

		if( host )
		{
			netConf->host->setText( host->host );
			netConf->password->setText( host->password );
			disconnect( netConf->port, SIGNAL( valueChanged( int ) ), this, SLOT( slotHostPortChanged( int ) ) );
			netConf->port->setValue( host->port );
			connect( netConf->port, SIGNAL( valueChanged( int ) ), this, SLOT( slotHostPortChanged( int ) ) );
			netConf->useSSL->setChecked( host->ssl );

			netConf->upButton->setEnabled( netConf->hostList->currentItem() > 0 );
			netConf->downButton->setEnabled( netConf->hostList->currentItem() < (int)( netConf->hostList->count() - 1 ) );
		}
	}
	else
	{
		m_uiCurrentHostSelection = QString();
		disconnect( netConf->port, SIGNAL( valueChanged( int ) ), this, SLOT( slotHostPortChanged( int ) ) );
		netConf->host->clear();
		netConf->password->clear();
		netConf->port->setValue( 6667 );
		netConf->useSSL->setChecked( false );
		connect( netConf->port, SIGNAL( valueChanged( int ) ), this, SLOT( slotHostPortChanged( int ) ) );
	}
}

void IRCProtocol::slotDeleteNetwork()
{
	QString network = netConf->networkList->currentText();
	if( KMessageBox::warningContinueCancel(
		Kopete::UI::Global::mainWidget(), i18n("<qt>Are you sure you want to delete the network <b>%1</b>?<br>"
		"Any accounts which use this network will have to be modified.</qt>")
		.arg(network), i18n("Deleting Network"),
		KGuiItem(i18n("&Delete Network"),"editdelete"), QString::fromLatin1("AskIRCDeleteNetwork") ) == KMessageBox::Continue )
	{
		disconnect( netConf->networkList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkConfig() ) );
		disconnect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );
		IRCNetwork *net = m_networks[ network ];
		for( QValueList<IRCHost*>::iterator it = net->hosts.begin(); it != net->hosts.end(); ++it )
		{
			m_hosts.remove( (*it)->host );
			delete (*it);
		}
		m_networks.remove( network );
		delete net;
		netConf->networkList->removeItem( netConf->networkList->currentItem() );
 		connect( netConf->networkList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkConfig() ) );
		connect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );
		slotUpdateNetworkHostConfig();

	}
}

void IRCProtocol::slotDeleteHost()
{
	QString hostName = netConf->host->text();
	if ( KMessageBox::warningContinueCancel(
		Kopete::UI::Global::mainWidget(), i18n("<qt>Are you sure you want to delete the host <b>%1</b>?</qt>")
		.arg(hostName), i18n("Deleting Host"),
		KGuiItem(i18n("&Delete Host"),"editdelete"), QString::fromLatin1("AskIRCDeleteHost")) == KMessageBox::Continue )
	{
		IRCHost *host = m_hosts[ hostName ];
		if ( host )
		{
			disconnect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );
			QString entryText = host->host + QString::fromLatin1(":") + QString::number(host->port);
			QListBoxItem * justAdded = netConf->hostList->findItem( entryText );
			netConf->hostList->removeItem( netConf->hostList->index( justAdded ) );
			connect( netConf->hostList, SIGNAL( selectionChanged() ), this, SLOT( slotUpdateNetworkHostConfig() ) );

			// remove from network as well
			IRCNetwork *net = m_networks[ m_uiCurrentNetworkSelection ];
			net->hosts.remove( host );

			m_hosts.remove( host->host );
			delete host;
		}
	}
}

void IRCProtocol::slotNewNetwork()
{
	// create a new network struct
	IRCNetwork *net = new IRCNetwork;
	// give it the name of 'New Network' (incrementing number if needed)
	QString netName = QString::fromLatin1( "New Network" );
	if ( m_networks.find( netName ) )
	{
		int newIdx = 1;
		do {
			netName = QString::fromLatin1( "New Network #%1" ).arg( newIdx++ );
		}
		while ( m_networks.find( netName ) && newIdx < 100 );
		if ( newIdx == 100 ) // pathological case
			return;
	}
	net->name = netName;
	// and add it to the networks dict and list
	m_networks.insert( net->name, net );
	netConf->networkList->insertItem( net->name );
	QListBoxItem * justAdded = netConf->networkList->findItem( net->name );
	netConf->networkList->setSelected( justAdded, true );
	netConf->networkList->setBottomItem( netConf->networkList->index( justAdded ) );
}

void IRCProtocol::slotNewHost()
{
	// create a new host
	IRCHost *host = new IRCHost;
	// prompt for a name
	bool ok;
	QString name = KInputDialog::getText(
			i18n("New Host"),
			i18n("Enter the hostname of the new server:"),
			QString::null, &ok, Kopete::UI::Global::mainWidget() );
	if ( ok )
	{
		// dupe check
		if ( m_hosts[ name ] )
		{
			KMessageBox::sorry(netConf, i18n( "A host already exists with that name" ) );
			return;
		}
		// set defaults on others
		host->host = name;
		host->port = 6667;
		host->ssl = false;
		// add it to the dict
		m_hosts.insert( host->host, host );
		// add it to the network!
		IRCNetwork *net = m_networks[ netConf->networkList->currentText() ];
		net->hosts.append( host );
		// add it to the gui
		QString entryText = host->host + QString::fromLatin1(":") + QString::number(host->port);
		netConf->hostList->insertItem( entryText );
		// select it in the gui
		QListBoxItem * justAdded = netConf->hostList->findItem( entryText );
		netConf->hostList->setSelected( justAdded, true );
		//netConf->hostList->setBottomItem( netConf->hostList->index( justAdded ) );
	}
}

void IRCProtocol::slotRenameNetwork()
{
	IRCNetwork *net = m_networks[ m_uiCurrentNetworkSelection ];
	if ( net )
	{
		bool ok;
		// popup up a dialog containing the current name
		QString name = KInputDialog::getText(
				i18n("Rename Network"),
				i18n("Enter the new name for this network:"),
				m_uiCurrentNetworkSelection, &ok,
				Kopete::UI::Global::mainWidget() );
		if ( ok )
		{
			if ( m_uiCurrentNetworkSelection != name )
			{
				// dupe check
				if ( m_networks[ name ] )
				{
					KMessageBox::sorry(netConf, i18n( "A network already exists with that name" ) );
					return;
				}

				net->name = name;
				// dict
				m_networks.remove( m_uiCurrentNetworkSelection );
				m_networks.insert( net->name, net );
				// ui
				int idx = netConf->networkList->index( netConf->networkList->findItem( m_uiCurrentNetworkSelection ) );
				m_uiCurrentNetworkSelection = net->name;
				netConf->networkList->changeItem( net->name, idx ); // changes the selection!!!
				netConf->networkList->sort();
			}
		}
	}
}

void IRCProtocol::addNetwork( IRCNetwork *network )
{
	m_networks.insert( network->name, network );
	slotSaveNetworkConfig();
}

void IRCProtocol::slotSaveNetworkConfig()
{
	// store any changes in the UI
	storeCurrentNetwork();
	kdDebug( 14120 ) <<  k_funcinfo << m_uiCurrentHostSelection << endl;
	storeCurrentHost();

	QDomDocument doc("irc-networks");
	QDomNode root = doc.appendChild( doc.createElement("networks") );

	for( QDictIterator<IRCNetwork> it( m_networks ); it.current(); ++it )
	{
		IRCNetwork *net = it.current();

		QDomNode networkNode = root.appendChild( doc.createElement("network") );
		QDomNode nameNode = networkNode.appendChild( doc.createElement("name") );
		nameNode.appendChild( doc.createTextNode( net->name ) );

		QDomNode descNode = networkNode.appendChild( doc.createElement("description") );
		descNode.appendChild( doc.createTextNode( net->description ) );

		QDomNode serversNode = networkNode.appendChild( doc.createElement("servers") );

		for( QValueList<IRCHost*>::iterator it2 = net->hosts.begin(); it2 != net->hosts.end(); ++it2 )
		{
			QDomNode serverNode = serversNode.appendChild( doc.createElement("server") );

			QDomNode hostNode = serverNode.appendChild( doc.createElement("host") );
			hostNode.appendChild( doc.createTextNode( (*it2)->host ) );

			QDomNode portNode = serverNode.appendChild( doc.createElement("port" ) );
			portNode.appendChild( doc.createTextNode( QString::number( (*it2)->port ) ) );

			QDomNode sslNode = serverNode.appendChild( doc.createElement("useSSL") );
			sslNode.appendChild( doc.createTextNode( (*it2)->ssl ? "true" : "false" ) );
		}
	}

//	kdDebug(14121) << k_funcinfo << doc.toString(4) << endl;
	QFile xmlFile( locateLocal( "appdata", "ircnetworks.xml" ) );

	if (xmlFile.open(IO_WriteOnly))
	{
		QTextStream stream(&xmlFile);
		stream << doc.toString(4);
		xmlFile.close();
	}
	else
		kdDebug(14121) << k_funcinfo << "Failed to save the Networks definition file" << endl;

	if (netConf)
		emit networkConfigUpdated( netConf->networkList->currentText() );
}

void IRCProtocol::slotReadNetworks()
{
	m_networks.clear();
	m_hosts.clear();

	QFile xmlFile( locate( "appdata", "ircnetworks.xml" ) );
	xmlFile.open( IO_ReadOnly );

	QDomDocument doc;
	doc.setContent( &xmlFile );
	QDomElement networkNode = doc.documentElement().firstChild().toElement();
	while( !networkNode.isNull () )
	{
		IRCNetwork *net = new IRCNetwork;

		QDomElement networkChild = networkNode.firstChild().toElement();
		while( !networkChild.isNull() )
		{
			if( networkChild.tagName() == "name" )
				net->name = networkChild.text();
			else if( networkChild.tagName() == "description" )
				net->description = networkChild.text();
			else if( networkChild.tagName() == "servers" )
			{
				QDomElement server = networkChild.firstChild().toElement();
				while( !server.isNull() )
				{
					IRCHost *host = new IRCHost;

					QDomElement serverChild = server.firstChild().toElement();
					while( !serverChild.isNull() )
					{
						if( serverChild.tagName() == "host" )
							host->host = serverChild.text();
						else if( serverChild.tagName() == "port" )
							host->port = serverChild.text().toInt();
						else if( serverChild.tagName() == "useSSL" )
							host->ssl = ( serverChild.text() == "true" );

						serverChild = serverChild.nextSibling().toElement();
					}

					net->hosts.append( host );
					m_hosts.insert( host->host, host );
					server = server.nextSibling().toElement();
				}
			}
			networkChild = networkChild.nextSibling().toElement();
		}

		m_networks.insert( net->name, net );
		networkNode = networkNode.nextSibling().toElement();
	}

	xmlFile.close();
}

void IRCProtocol::slotMoveServerUp()
{
	IRCHost *selectedHost = m_hosts[ netConf->hostList->currentText().section(':', 0, 0) ];
	IRCNetwork *selectedNetwork = m_networks[ netConf->networkList->currentText() ];

	if( !selectedNetwork || !selectedHost )
		return;

	QValueList<IRCHost*>::iterator pos = selectedNetwork->hosts.find( selectedHost );
	if( pos != selectedNetwork->hosts.begin() )
	{
		QValueList<IRCHost*>::iterator lastPos = pos;
		lastPos--;
		selectedNetwork->hosts.insert( lastPos, selectedHost );
		selectedNetwork->hosts.remove( pos );
	}

	unsigned int currentPos = netConf->hostList->currentItem();
	if( currentPos > 0 )
	{
		netConf->hostList->removeItem( currentPos );
		QString entryText = selectedHost->host + QString::fromLatin1(":") + QString::number( selectedHost->port );
		netConf->hostList->insertItem( entryText, --currentPos );
		netConf->hostList->setSelected( currentPos, true );
	}
}

void IRCProtocol::slotMoveServerDown()
{
	IRCHost *selectedHost = m_hosts[ netConf->hostList->currentText().section(':', 0, 0) ];
	IRCNetwork *selectedNetwork = m_networks[ netConf->networkList->currentText() ];

	if( !selectedNetwork || !selectedHost )
		return;

	QValueList<IRCHost*>::iterator pos = selectedNetwork->hosts.find( selectedHost );
	if( *pos != selectedNetwork->hosts.back() )
	{
		QValueList<IRCHost*>::iterator nextPos = selectedNetwork->hosts.remove( pos );
		selectedNetwork->hosts.insert( ++nextPos, selectedHost );
	}

	unsigned int currentPos = netConf->hostList->currentItem();
	if( currentPos < ( netConf->hostList->count() - 1 ) )
	{
		netConf->hostList->removeItem( currentPos );
		QString entryText = selectedHost->host + QString::fromLatin1(":") + QString::number( selectedHost->port );
		netConf->hostList->insertItem( entryText, ++currentPos );
		netConf->hostList->setSelected( currentPos, true );
	}
}



#include "ircprotocol.moc"

// vim: set noet ts=4 sts=4 sw=4:
