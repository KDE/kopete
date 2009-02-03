/*
    ircprotocol - IRC Protocol

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "ircprotocol.moc"

#include "ircaccount.h"
#include "irccontact.h"
#include "irctransferhandler.h"
#include "ksparser.h"

#include "networkconfigwidget.h"
//#include "channellist.h"
#include "ircaddcontactpage.h"
//#include "ircguiclient.h"
#include "irceditaccountwidget.h"

#include "kircclientsocket.h"

#include "kopeteaccountmanager.h"
#include "kopetechatsessionmanager.h"
#include "kopetecommandhandler.h"
#include "kopeteglobal.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteonlinestatus.h"
#include "kopeteview.h"
#include "kopeteuiglobal.h"

#include <kaboutdata.h>
#include <kaction.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
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
#include <qtextcodec.h>

#include <dom/html_element.h>
#include <unistd.h>

using namespace Kopete;

K_PLUGIN_FACTORY( IRCProtocolFactory, registerPlugin<IRCProtocol>(); )
K_EXPORT_PLUGIN( IRCProtocolFactory( "kopete_irc" ) )

static IRCProtocol *s_protocol = 0L;

IRCProtocolHandler::IRCProtocolHandler()
	: MimeTypeHandler(false)
{
	registerAsProtocolHandler(QString::fromLatin1("irc"));
}

void IRCProtocolHandler::handleURL(const KUrl &url) const
{
	kDebug(14120) << url;
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

	kDebug(14120) << accountId;

	IRCAccount *newAccount = new IRCAccount( accountId, chan );
	newAccount->setNickName( user.loginName() );
	newAccount->setUserName( user.loginName() );
	newAccount->connect();
}

IRCProtocol::IRCProtocol( QObject *parent, const QVariantList & /* args */ )
	: Protocol(IRCProtocolFactory::componentData(), parent)
//	, m_StatusUnknown(OnlineStatus::Unknown, 999, this, 999, "status_unknown", i18n("Status not available"))
{
	kDebug(14120) ;

	s_protocol = this;

	initOnlineStatus();

	//m_status = m_unknownStatus = m_Unknown;

	addAddressBookField("messaging/irc", Plugin::MakeIndexField);

	CommandHandler *commandHandler = CommandHandler::commandHandler();

	// Statically implemented commands
	commandHandler->registerCommand(this, QString::fromLatin1("all"),
		SLOT( slotAllCommand(const QString &, Kopete::ChatSession*)),
		i18n("USAGE: /all <command> - Exectute the given command in all the chats."), 2);

	commandHandler->registerCommand(this, QString::fromLatin1("ctcp"),
		SLOT( slotCtcpCommand(const QString &, Kopete::ChatSession *)),
		i18n("USAGE: /ctcp <nick> <message> - Send the CTCP message to nick."), 2);

	commandHandler->registerCommand(this, QString::fromLatin1("quote"),
		SLOT( slotQuoteCommand(const QString &, Kopete::ChatSession*)),
		i18n("USAGE: /quote <text> - Sends the text in quoted form to the server."), 1);

	commandHandler->registerCommand(this, QString::fromLatin1("raw"),
		SLOT( slotRawCommand( const QString &, Kopete::ChatSession*)),
		i18n("USAGE: /raw <text> - Sends the text in raw form to the server."), 1);

	//Add a command to execute the natively implemented commands in libkirc
	//TODO: hide this command from the user / use 1 slot per command
	commandHandler->registerCommand(this, QString::fromLatin1("kirc" ),
		SLOT( slotNativeKIrcCommand( const QString &, Kopete::ChatSession* ) ),
		QString(), 1 );

	// Alias implemented commands
/*	commandHandler->registerAlias(this, QString::fromLatin1("ame"),
		QString::fromLatin1("all ME"),
		i18n("USAGE: /ame <action> - Do something in every open chat."),
		CommandHandler::SystemAlias, 1);
*/

	commandHandler->registerAlias(this, QString::fromLatin1("join"),
		QString::fromLatin1("kirc join %s"),
		i18n("USAGE: /join <#channel 1> [<password>] - Joins the specified channel."),
		CommandHandler::SystemAlias, 1, 2);

/*	commandHandler->registerAlias(this, QString::fromLatin1("quit"),
		QString::fromLatin1("kirc quit %s"),
		i18n("USAGE: /quit [<reason>] - Disconnect from IRC, optionally leaving a message."),
		CommandHandler::SystemAlias);
*/
	commandHandler->registerAlias(this, QString::fromLatin1("topic"),
		QString::fromLatin1("kirc TOPIC %s"),
		i18n("USAGE: /topic [<topic>] - Sets and/or displays the topic for the active channel."),
		CommandHandler::SystemAlias);

	commandHandler->registerAlias(this, QString::fromLatin1("part"),
		QString::fromLatin1( "kirc part %s" ),
		i18n("USAGE: /part [<reason>] - Part from a channel, optionally leaving a message."),
		CommandHandler::SystemAlias);


	/*
	commandHandler->registerAlias(this, QString::fromLatin1("ban"),
		SLOT(slotBanCommand(const QString &, Kopete::ChatSession*)),
		i18n("USAGE: /ban <mask> - Add someone to this channel's ban list. (requires operator status)."),
		CommandHandler::SystemAlias, 1, 1);
*/
/*	commandHandler->registerAlias(this, QString::fromLatin1("bannick"),
		QString::fromLatin1("ban %1!*@*"),
		i18n("USAGE: /bannick <nickname> - Add someone to this channel's ban list. Uses the hostmask nickname!*@* (requires operator status)."),
		CommandHandler::SystemAlias, 1, 1);

	commandHandler->registerAlias(this, QString::fromLatin1("deop"),
		SLOT(slotDeopCommand(const QString &, Kopete::ChatSession*)),
		i18n("USAGE: /deop <nickname> [<nickname 2> <...>] - Remove channel operator status from someone (requires operator status)."),
		CommandHandler::SystemAlias, 1);

	commandHandler->registerAlias(this, QString::fromLatin1("devoice"),
		SLOT( slotDevoiceCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /devoice <nickname> [<nickname 2> <...>] - Remove channel voice status from someone (requires operator status)."),
		CommandHandler::SystemAlias, 1);

	commandHandler->registerAlias(this, QString::fromLatin1("invite"),
		QString::fromLatin1("raw invite %s"),
		i18n("USAGE: /invite <nickname> [<channel>] - Invite a user to join a channel."),
		CommandHandler::SystemAlias, 1);

	commandHandler->registerAlias( this, QString::fromLatin1("kick"),
		SLOT( slotKickCommand( const QString &, Kopete::ChatSession*) ),
		i18n("USAGE: /kick <nickname> [<reason>] - Kick someone from the channel (requires operator status)."),
		CommandHandler::SystemAlias, 1);

	commandHandler->registerAlias(this, QString::fromLatin1("list"),
		QString::fromLatin1( "raw list %s"),
		i18n("USAGE: /list - List the public channels on the server."),
		CommandHandler::SystemAlias );

	commandHandler->registerAlias(this, QString::fromLatin1("me"),
		QString::fromLatin1("ctcp me"),
		i18n("USAGE: /me <action> - Do something."),
		CommandHandler::SystemAlias, 1 );

	// FIX help string, MODE is also available for user
	commandHandler->registerAlias( this, QString::fromLatin1("mode"),
		QString::fromLatin1("raw mode %s"),
		i18n("USAGE: /mode <channel> <modes> - Set modes on the given channel."),
		CommandHandler::SystemAlias, 2);

	commandHandler->registerAlias(this, QString::fromLatin1("motd"),
		QString::fromLatin1("raw motd %s"),
		i18n("USAGE: /motd [<server>] - Shows the message of the day for the current or the given server."),
		CommandHandler::SystemAlias);

	commandHandler->registerAlias(this, QString::fromLatin1("nick"),
		QString::fromLatin1 ("raw nick %s"),
		i18n("USAGE: /nick <nickname> - Change your nickname to the given one."),
		CommandHandler::SystemAlias, 1, 1);

	commandHandler->registerAlias(this, QString::fromLatin1("op"),
		SLOT( slotOpCommand(const QString &, Kopete::ChatSession*)),
		i18n("USAGE: /op <nickname 1> [<nickname 2> <...>] - Give channel operator status to someone (requires operator status)."),
		CommandHandler::SystemAlias, 1);


	commandHandler->registerAlias(this, QString::fromLatin1("ping"),
		QString::fromLatin1( "ctcp %1 PING" ),
		i18n("USAGE: /ping <nickname> - Alias for /CTCP <nickname> PING."),
		CommandHandler::SystemAlias, 1, 1);*/
/*
	commandHandler->registerCommand(this, QString::fromLatin1("query"),
		SLOT(slotQueryCommand(const QString &, Kopete::ChatSession*)),
		i18n("USAGE: /query <nickname> [<message>] - Open a private chat with this user."),
		CommandHandler::SystemAlias, 1);
*/
/*
	commandHandler->registerAlias(this, QString::fromLatin1("voice"),
		SLOT(slotVoiceCommand( const QString &, Kopete::ChatSession*)),
		i18n("USAGE: /voice <nickname> [<nickname 2> <...>] - Give channel voice status to someone (requires operator status)."),
		CommandHandler::SystemAlias, 1);
*/
/*	commandHandler->registerAlias( this, QString::fromLatin1("who"),
		QString::fromLatin1( "raw WHO %1" ),
		i18n("USAGE: /who <nickname|channel> - Display who info on this user/channel."),
		CommandHandler::SystemAlias, 1, 1 );

	commandHandler->registerAlias( this, QString::fromLatin1("whois"),
		QString::fromLatin1("raw WHOIS %1"),
		i18n("USAGE: /whois <nickname> - Display whois info on this user."),
		CommandHandler::SystemAlias, 1, 1);

	commandHandler->registerAlias( this, QString::fromLatin1("whowas"),
		QString::fromLatin1( "raw WHOWAS %1" ),
		i18n("USAGE: /whowas <nickname> - Display whowas info on this user."),
		CommandHandler::SystemAlias, 1, 1);

	// Alias of alias
	commandHandler->registerAlias( this, QString::fromLatin1("j"),
		QString::fromLatin1("join %1"),
		i18n("USAGE: /j <#channel 1> [<password>] - Alias for JOIN."),
		CommandHandler::SystemAlias, 1, 2);
/*
	commandHandler->registerAlias( this, QString::fromLatin1("msg"),
		QString::fromLatin1("query %s"),
		i18n("USAGE: /msg <nickname> [<message>] - Alias for QUERY <nickname> <message>."),
		CommandHandler::SystemAlias, 1);
*/

	// Utility alias
/*
	commandHandler->registerAlias( this, QString::fromLatin1("ns"),
		QString::fromLatin1("msg nickserv %s"),
		i18n("USAGE: /ns <message> - Alias for MSG NickServ <message>."),
		CommandHandler::SystemAlias, 1);
*/
	QObject::connect( ChatSessionManager::self(), SIGNAL(aboutToDisplay(Kopete::Message &)),
		this, SLOT(slotMessageFilter(Kopete::Message &)) );

	QObject::connect( ChatSessionManager::self(), SIGNAL( viewCreated( KopeteView* ) ),
		this, SLOT( slotViewCreated( KopeteView* ) ) );

	setCapabilities(Protocol::RichBFormatting | Kopete::Protocol::RichUFormatting | Kopete::Protocol::RichColor);

	m_protocolHandler = new IRCProtocolHandler();
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
/*
	OnlineStatus ServerOnline(OnlineStatus::Online, 100, this, 0,
		QString(), i18n("Online"));

	OnlineStatus ServerOffline(OnlineStatus::Offline, 90, this, 0,
		QString(), i18n("Offline"));

	m_statusMap.insert(ServerOnline.internalStatus(), ServerOnline);
	m_statusMap.insert(ServerOffline.internalStatus(), ServerOffline);

	ChannelOnline(OnlineStatus::Online, 80, this, EntityType::Channel|EntityType::Online,
		QString(), i18n("Online")),
	m_statusMap.insert(ChannelOnline.internalStatus(), ChannelOnline);

	ChannelOffline(OnlineStatus::Offline, 70, this, EntityType::Channel|EntityType::OfflineChannel,
		QString(), i18n("Offline")),
	m_statusMap.insert(ChannelOffline.internalStatus(), ChannelOffline);

	KIrc::EntityStatus status;
	status.type = KIrc::User;

	onlineStatusFor(status, OnlineStatusManager::Offline);

	status.online = true;
	onlineStatusFor(status,  OnlineStatusManager::Online);

	status.mode_a = true;
	onlineStatusFor(status,  OnlineStatusManager::Away);

	OnlineStatus UserOnline(OnlineStatus::Online, 25, this, 0,
		QString(), i18n("Online"), i18n("Online"), OnlineStatusManager::Online);

	OnlineStatus UserAway(OnlineStatus::Away, 2, this, 0,
		"contact_away_overlay", i18n("Away"), i18n("Away"), OnlineStatusManager::Away);

	OnlineStatus UserConnecting(OnlineStatus::Connecting, 1, this, 0,
		"irc_connecting", i18n("Connecting"));

	OnlineStatus UserOffline(OnlineStatus::Offline, 0, this, 0,
		QString(), i18n("Offline"), i18n("Offline"), OnlineStatusManager::Offline);
*/

	onlineStatusFor(KIrc::Unknown,OnlineStatusManager::Offline);
	onlineStatusFor(KIrc::Online,OnlineStatusManager::Online);
	onlineStatusFor((KIrc::Online|KIrc::Away), OnlineStatusManager::Away);
}

OnlineStatus IRCProtocol::onlineStatusFor(KIrc::EntityPtr entity, KIrc::Context* relativeTo )
{
	return onlineStatusFor(entity,0,relativeTo);
}

OnlineStatus IRCProtocol::onlineStatusFor(KIrc::EntityPtr entity, OnlineStatusManager::Categories categories, KIrc::Context* relativeTo)
{

	// Only copy the needed status
	KIrc::EntityStatus status;
	if(relativeTo)
		status=relativeTo->statusOf(entity);
	else
		status=entity->status();

	return onlineStatusFor(status,categories);
}

OnlineStatus IRCProtocol::onlineStatusFor(KIrc::EntityStatus status,OnlineStatusManager::Categories categories)
{
	//status.online = _status.online;
	//status.mode_a = _status.mode_a;
	//status.mode_i = _status.mode_i;
	//status.mode_o = _status.mode_o;
	//status.mode_v = _status.mode_v;
	//status.mode_O = _status.mode_O;

	OnlineStatus ret = m_statusMap[status];
	if (ret.status() == OnlineStatus::Unknown)
	{
		OnlineStatus::StatusType statusType;
		unsigned weight = 0;
		QStringList overlayIcons;
		QString description;

		if (status.testFlag(KIrc::Online))
		{
			statusType = OnlineStatus::Online;

			weight += 1;
			description = i18n("Online");
			weight <<= 1;

			//Is a channel
			if(status.testFlag(KIrc::Channel))
			{
				weight+=1;
				description= i18n("Channel");
			}
			weight <<=1;

			// Is operator
			if (status.testFlag(KIrc::Operator)) //mode_o || status.mode_O)
			{
				weight += 1;
				overlayIcons << "irc_op";
				description = i18n("Operator");
			}
			weight <<= 1;

			// Is Voiced
			if (status.testFlag(KIrc::Voiced))
			{
				weight += 1;
				overlayIcons << "irc_voice";
				description = i18n("Voiced");
			}
			weight <<= 1;

			// Is away
			if (status.testFlag(KIrc::Away))
			{
				statusType = OnlineStatus::Away;
				weight += 1;
				overlayIcons << "contact_away_overlay";
				description = i18n("Away");
			}
			weight <<= 1;

			// Is Invisible
			if (status.testFlag(KIrc::Invisible))
			{
				statusType = OnlineStatus::Invisible;
				weight += 1;
			}
			weight <<= 1;
		}
		else
		{
			statusType = OnlineStatus::Offline;
			description = i18n("Offline");
		}

		ret=OnlineStatus(statusType, weight, this,
						 status, overlayIcons, description, description, categories);

		m_statusMap.insert(status, ret);
	}

	return ret;
}

void IRCProtocol::slotViewCreated(KopeteView *view)
{
//	if (view->msgManager()->protocol() == this)
//		new IRCGUIClient(view->msgManager());
}

void IRCProtocol::slotMessageFilter(Message &msg)
{
	if (msg.from()->protocol() == this)
	{
		QString messageText = msg.escapedBody();

		//Add right click for channels, only replace text not in HTML tags
		messageText.replace(QRegExp( QString::fromLatin1("(?![^<]+>)(#[^#\\s]+)(?![^<]+>)")), QString::fromLatin1("<span class=\"KopeteLink\" type=\"IRCChannel\">\\1</span>") );

		msg.setHtmlBody(messageText);
	}
}
/*
QList<KAction *> *IRCProtocol::customChatWindowPopupActions(const Message &m, DOM::Node &n)
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
*/
AddContactPage *IRCProtocol::createAddContactWidget(QWidget *parent, Account *account)
{
	return new IRCAddContactPage(parent,static_cast<IRCAccount*>(account));
}

KopeteEditAccountWidget *IRCProtocol::createEditAccountWidget(Account *account, QWidget *parent)
{
	return new IRCEditAccountWidget(qobject_cast<IRCAccount*>(account), parent);
}

Account *IRCProtocol::createNewAccount(const QString &accountId)
{
	return new IRCAccount(accountId);
}

Contact *IRCProtocol::deserializeContact(MetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/*addressBookData*/)
{
	kDebug(14120) ;

	QString contactId = serializedData[ "contactId" ];
	QString displayName = serializedData[ "displayName" ];
	QString accountId = serializedData[ "accountId" ];

	if( displayName.isEmpty() )
		displayName = contactId;

	QList<Account*> accounts = AccountManager::self()->accounts( this );
	Kopete::Account* account = 0;
	foreach( Kopete::Account* acct, accounts )
	{
		if ( acct->accountId() == accountId )
			account = acct;
	}

	if( account )
	{
		account->addContact( contactId, metaContact );
		return account->contacts()[contactId];
	}
	else
	{
		kDebug(14120) << serializedData[ "accountId" ] << " was a contact's account,"
			" but we don't have it in the accounts list" << endl;
	}

	return 0;
}

void IRCProtocol::slotAllCommand(const QString &args, ChatSession *manager)
{
#ifdef __GNUC__
#warning IMPLEMENT ME
#endif
}

void IRCProtocol::slotCtcpCommand(const QString &args, ChatSession *manager)
{
	if (!args.isEmpty())
	{
		QString user = args.section( ' ', 0, 0 );
		QString message = args.section( ' ', 1 );
//		static_cast<IRCAccount*>(manager->account())->client()->writeCtcpQueryMessage(
//			user, QString(), message);
	}
}

void IRCProtocol::slotQuoteCommand( const QString &args, ChatSession *manager )
{
//	static_cast<IRCAccount *>(manager->account())->client()->writeRawMessage(args);
}

void IRCProtocol::slotRawCommand( const QString &args, ChatSession *manager )
{
//	static_cast<IRCAccount*>(manager->account())->client()->writeRawMessage(args);
}

void IRCProtocol::slotNativeKIrcCommand( const QString& args, ChatSession* manager )
{
	IRCAccount *account=static_cast<IRCAccount*>( manager->account() );
	account->client()->onCommand( static_cast<IRCContact*> ( manager->members().first() )->entity()->context(),
								  account->codec()->fromUnicode( args )
								);
}

void IRCProtocol::editNetworks(const QString &networkName)
{

	IRCNetworkConfigWidget *netConf = new IRCNetworkConfigWidget(UI::Global::mainWidget() );
	netConf->setAttribute( Qt::WA_DeleteOnClose );
	netConf->editNetworks(networkName);
	netConf->exec();
}

