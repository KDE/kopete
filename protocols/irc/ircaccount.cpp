/*
    ircaccount.cpp - IRC Account

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2004 by Jason Keirstead <jason@keirstead.org>
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

#include "ircaccount.h"

#include "irccontact.h"
#include "ircprotocol.h"

#include "kircclientsocket.h"
#include "kircentitymanager.h"
#include "kircevent.h"
#include "kircstdmessages.h"

#include "kopeteaccountmanager.h"
#include "kopeteaway.h"
#include "kopeteawayaction.h"
#include "kopetechatsessionmanager.h"
#include "kopetecommandhandler.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include "kopeteview.h"
#include "kopetepassword.h"

#include <kaction.h>
#include <kconfig.h>
#include <kcompletionbox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinputdialog.h>
//#include <klineedit.h>
//#include <klineeditdlg.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>


#include <qtextcodec.h>

using namespace IRC;
using namespace Kopete;

class IRCAccount::Private
{
public:
	Private()
		: manager(0), client(0),
		  server(0), self(0),
		  commandSource(0),
		  awayAction(0), joinChannelAction(0), searchChannelAction(0)
	{ }

	Kopete::ChatSession *manager;
	QString autoConnect;

	KIrc::ClientSocket *client;
	IRC::Network network;
	int currentHost;

	QList<IRCContact *> contacts;
	IRCContact *server;
	IRCContact *self;

	Kopete::OnlineStatus expectedOnlineStatus;
	QString expectedReason;

	QMap<QString, QString> customCtcp;
	Kopete::ChatSession *commandSource;

	Kopete::AwayAction *awayAction;

	KAction *joinChannelAction;
	KAction *searchChannelAction;
};

IRCAccount::IRCAccount(const QString &accountId, const QString &autoChan, const QString& netName, const QString &nickName)
	: PasswordedAccount(IRCProtocol::self(), accountId, true),
	  d( new Private )
{
	d->client = new KIrc::ClientSocket(this);
	d->autoConnect = autoChan;
	d->currentHost = 0;

	QObject::connect(d->client, SIGNAL(connectionStateChanged(KIrc::ConnectionState)),
			 this, SLOT(clientConnectionStateChanged(KIrc::ConnectionState)));

	QObject::connect(d->client, SIGNAL(receivedMessage(KIrc::MessageType, const KIrc::Entity::Ptr &, const KIrc::Entity::List &, const QString &)),
			 this, SLOT(receivedMessage(KIrc::MessageType, const KIrc::Entity::Ptr &, const KIrc::Entity::List &, const QString &)));

//	loadProperties();

	d->server = new IRCContact(this, d->client->server());
	d->self = new IRCContact(this, d->client->owner());
	setMyself(d->self);
/*
	QString accountId = this->accountId();
	if (networkName.isEmpty() && QRegExp( "[^#+&\\s]+@[\\w-\\.]+:\\d+" ).exactMatch(accountId))
	{
		kDebug(14120) << "Creating account from " << accountId << endl;

//		mNickName = accountId.section('@',0,0);
		QString serverInfo = accountId.section('@',1);
		QString hostName = serverInfo.section(':',0,0);

		QValueList<IRCNetwork> networks = IRCNetworkList::self()->networks();
		for (QValueList<IRCNetwork>::Iterator it = networks.begin(); it != networks.end(); ++it)
		{
			IRCNetwork net = *it;
			for (QValueList<IRCHost>::iterator it2 = net.hosts.begin(); it2 != net.hosts.end(); ++it2)
			{
				if( (*it2).host == hostName )
				{
					setNetwork(net.name);
					break;
				}
			}

			if( !networkName.isEmpty() )
				break;
		}

		if( networkName.isEmpty() )
		{
			// Could not find this host. Add it to the networks structure

			d->network = IRCNetwork();
			d->network.name = i18n("Temporary Network - %1").arg( hostName );
			d->network.description = i18n("Network imported from previous version of Kopete, or an IRC URI");

			IRCHost host;
			host.host = hostName;
			host.port = serverInfo.section(':',1).toInt();
			if (!password().cachedValue().isEmpty())
				host.password = password().cachedValue();
			host.ssl = false;

			d->network.hosts.append( host );
//			d->protocol->addNetwork( d->network );

			config->writeEntry(Config::NETWORKNAME, d->network.name);
//			config->writeEntry(Config::NICKNAME, mNickName);
		}
	}
	else if( !networkName.isEmpty() )
	{
		setNetwork(networkName);
	}
	else
	{
		kError() << "No network name defined, and could not import network information from ID" << endl;
	}
*/

//	setAccountLabel( QString::fromLatin1("%1@%2").arg(mNickName,networkName) );

#ifdef __GNUC__
	#warning spurus slot calls for now
#endif
	d->joinChannelAction = new KAction ( i18n("Join Channel..."), QString::null, KShortcut(),
		this, SLOT(slotJoinChannel()), 0/*actiongroup*/, 0);
	d->searchChannelAction = new KAction ( i18n("Search Channels..."), QString::null, KShortcut(),
		this, SLOT(slotSearchChannels()), 0/*actionGroup*/, 0);
}

IRCAccount::~IRCAccount()
{
	kDebug(14120) << k_funcinfo << endl;
//	KIrc::StdMessage::quit(d->client, i18n("Plugin Unloaded"));

	delete d;
}


void IRCAccount::clientSetup()
{
/*
	d->client->setDefaultCodec(codec());

	// Build the URL instead
	KUrl url;
	url.setUser(userName());
//	url.setPass(password());

	d->client->setNickName(nickName());
	url.addQuery(URL_REALNAME, realName());
	d->client->setVersionString(IRC::Version);

	QMap<QString, QString> replies = customCtcpReplies();
	for (QMap<QString, QString>::ConstIterator it = replies.begin(); it != replies.end(); ++it)
		d->client->addCustomCtcp(it.key(), it.data());
*/

//	d->network = IRCNetworkList::self()->network(networkName());
/*
	// if prefer SSL is set, sort by SSL first
	if (configGroup()->readEntry("PreferSSL",false))
	{
		IRCHostList sslFirst;

		IRCHostList::iterator it = host.begin();
		IRCHostList::iterator end = host.end();
		for ( it = host.begin(); it != end; ++it )
		{
			if ( (*it)->ssl == true )
			{
				sslFirst.append( *it );
				it = hosts.remove( it );
			}
		}
		for ( it = hosts.begin(); it != hosts.end(); ++it )
			sslFirst.append( *it );

		d->network.hosts = sslFirst;
	}
*/
}

void IRCAccount::clientConnect()
{
	kDebug(14120) << k_funcinfo << endl;

	if (d->network.name.isEmpty())
	{
		KMessageBox::queuedMessageBox(
			UI::Global::mainWidget(), KMessageBox::Error,
			i18n("<qt>The network associated with this account has no valid hosts. "
				"Please ensure that the account has a valid network.</qt>"),
			i18n("Network is Empty"), 0 );
	}
	else if (d->network.hosts.isEmpty())
	{
		KMessageBox::queuedMessageBox(
			UI::Global::mainWidget(), KMessageBox::Error,
			i18n("<qt>The network associated with this account, <b>%1</b>, has no valid hosts. "
				"Please ensure that the account has a valid network.</qt>", d->network.name),
			i18n("Network is Empty"), 0 );
	}
	else if( d->currentHost == d->network.hosts.count() )
	{
		KMessageBox::queuedMessageBox(
			UI::Global::mainWidget(), KMessageBox::Error,
			i18n("<qt>Kopete could not connect to any of the servers in the network "
				"associated with this account (<b>%1</b>). Please try again later.</qt>",
				d->network.name),
			i18n("Network is Unavailable"), 0 );

			d->currentHost = 0;
	}
	else
	{
		const IRC::Host& host = d->network.hosts[ d->currentHost++ ];
		//appendInternalMessage( i18n("Connecting to %1...").arg( host.host ) );

		QString url;

		if (host.ssl) {
			//appendInternalMessage( i18n("Using SSL") );
			url = "ircs://";
		} else {
			url = "irc://";
		}

		url += nickName() + '@' + host.host + ':' + host.port;

		//d->client->connectToServer( host->host, host->port, mNickName, host->ssl );
		d->client->connectToServer(KUrl(url));
	}
}

int IRCAccount::codecMib() const
{
	kDebug(14120) << k_funcinfo << endl;
	return configGroup()->readNumEntry(Config::CODECMIB);
}

void IRCAccount::setCodecFromMib(int mib)
{
	kDebug(14120) << k_funcinfo << endl;
	configGroup()->writeEntry(Config::CODECMIB, mib);
	d->client->setDefaultCodec(QTextCodec::codecForMib(mib));
}

QTextCodec *IRCAccount::codec() const
{
	kDebug(14120) << k_funcinfo << endl;
	return QTextCodec::codecForMib(codecMib());
}

void IRCAccount::setCodec( QTextCodec *codec )
{
	kDebug(14120) << k_funcinfo << endl;
	if (codec)
		setCodecFromMib(codec->mibEnum());
	else
		setCodecFromMib(-1); // MIBenum are >= 0  so we inforce an error value
}

const QString IRCAccount::networkName() const
{
	return configGroup()->readEntry(Config::NETWORKNAME, QString());
}

void IRCAccount::setNetworkByName(const QString &networkName)
{
	configGroup()->writeEntry(Config::NETWORKNAME, networkName);
//	setAccountLabel(network.name);
}
/*
IRCNetwork network() const
{
	return d->network;
}
*/
const QString IRCAccount::userName() const
{
	return configGroup()->readEntry(Config::USERNAME, QString());
}

void IRCAccount::setUserName(const QString &userName)
{
	configGroup()->writeEntry(Config::USERNAME, userName);
}

const QString IRCAccount::realName() const
{
	return configGroup()->readEntry(Config::REALNAME, QString());
}

void IRCAccount::setRealName( const QString &userName )
{
	configGroup()->writeEntry(Config::REALNAME, userName);
}

const QString IRCAccount::nickName() const
{
	return configGroup()->readEntry(Config::NICKNAME, QString());
}

void IRCAccount::setNickName(const QString &nickName)
{
	configGroup()->writeEntry(Config::NICKNAME, nickName);
//	d->self->setNickName(nickName);
}

const QString IRCAccount::partMessage() const
{
	return configGroup()->readEntry(QLatin1String("defaultPart"), QString());
}

void IRCAccount::setPartMessage( const QString &partMessage )
{
	configGroup()->writeEntry(QLatin1String("defaultPart"), partMessage);
}

const QString IRCAccount::quitMessage() const
{
	return configGroup()->readEntry(QLatin1String("defaultQuit"), QString());
}

void IRCAccount::setQuitMessage(const QString &quitMessage)
{
	configGroup()->writeEntry( QLatin1String("defaultQuit"), quitMessage );
}

bool IRCAccount::autoShowServerWindow() const
{
	return configGroup()->readEntry(QString::fromLatin1("AutoShowServerWindow"), false);
}

void IRCAccount::setAutoShowServerWindow(bool autoShow)
{
	configGroup()->writeEntry(QString::fromLatin1("AutoShowServerWindow"), autoShow);
}

KIrc::ClientSocket *IRCAccount::client() const
{
	kDebug(14120) << k_funcinfo << endl;
	return d->client;
}

void IRCAccount::setCustomCtcpReplies(const QMap<QString, QString> &replies)
{
/*
	QStringList val;
	for( QMap< QString, QString >::ConstIterator it = replies.begin(); it != replies.end(); ++it )
	{
		d->client->addCustomCtcp( it.key(), it.data() );
		val.append( QString::fromLatin1("%1=%2").arg( it.key() ).arg( it.data() ) );
	}

	configGroup()->writeEntry( "CustomCtcp", val );
*/
}

const QMap<QString, QString> IRCAccount::customCtcpReplies() const
{
	kDebug(14120) << k_funcinfo << endl;
	QMap< QString, QString > replies;
/*
	QStringList replyList;

	replyList = configGroup()->readListEntry( "CustomCtcp" );

	for( QStringList::Iterator it = replyList.begin(); it != replyList.end(); ++it )
		replies[ (*it).section('=', 0, 0 ) ] = (*it).section('=', 1 );
*/
	return replies;
}

void IRCAccount::setConnectCommands( const QStringList &commands ) const
{
	configGroup()->writeEntry( "ConnectCommands", commands );
}

const QStringList IRCAccount::connectCommands() const
{
	return configGroup()->readListEntry( "ConnectCommands" );
}

KActionMenu *IRCAccount::actionMenu()
{
	kDebug(14120) << k_funcinfo << endl;
	QString menuTitle = QString::fromLatin1( " %1 <%2> " ).arg( accountId() ).arg( myself()->onlineStatus().description() );

	KActionMenu *mActionMenu = Account::actionMenu();

	d->joinChannelAction->setEnabled( isConnected() );
	d->searchChannelAction->setEnabled( isConnected() );

	mActionMenu->addSeparator();
	mActionMenu->addAction(d->joinChannelAction);
	mActionMenu->addAction(d->searchChannelAction);
/*
	mActionMenu->insert( new KAction ( i18n("Show Server Window"), QString::null, 0, this, SLOT(slotShowServerWindow()), mActionMenu ) );

//	if (d->client->isConnected() && d->client->useSSL())
	{
		mActionMenu->insert( new KAction ( i18n("Show Security Information"), "", 0, d->client,
			SLOT(showInfoDialog()), mActionMenu ) );
	}
*/
	return mActionMenu;
}

void IRCAccount::connectWithPassword(const QString &password)
{
	kDebug(14120) << k_funcinfo << endl;
//	d->client->setPassword(password);
	clientConnect();
}

void IRCAccount::clientConnectionStateChanged(KIrc::Socket::ConnectionState newstate)
{
	kDebug(14120) << k_funcinfo << endl;

	mySelf()->updateStatus();

	switch (newstate)
	{
	case KIrc::Socket::Connecting:
		// d->expectedOnlineStatus check and use it
		if (autoShowServerWindow())
			myServer()->startChat();
		break;
/*
	case KIrc::Socket::Open:
		//Reset the host so re-connection will start over at first server
		d->currentHost = 0;
//		d->contactManager->addToNotifyList( d->client->nickName() );

		// HACK! See bug #85200 for details. Some servers still cannot accept commands
		// after the 001 is sent, you need to wait until all the init junk is done.
		// Unfortunately, there is no way for us to know when it is done (it could be
		// spewing out any number of replies), so just try delaying it
//		QTimer::singleShot( 250, this, SLOT( slotPerformOnConnectCommands() ) );
		break;
	case KIrc::Socket::Closing:
//		mySelf()->setOnlineStatus( protocol->m_UserStatusOffline );
//		d->contactManager->removeFromNotifyList( d->client->nickName() );

//		if (d->contactManager && !autoConnect.isNull())
//			AccountManager::self()->removeAccount( this );
		break;
//	case KIrc::Socket::Timeout:
		//Try next server
//		connect();
//		break;
*/
	default:
		kDebug(14120) << k_funcinfo << "Doing nothing on state" << newstate << endl;
	}
}
/*
// Put that in error handling
void IRCAccount::slotFailedServerPassword()
{
	// JLN
	password().setWrong();
	connect();
}
*/
void IRCAccount::slotPerformOnConnectCommands()
{
	kDebug(14120) << k_funcinfo << endl;
	ChatSession *manager = myServer()->manager(Contact::CanCreate);
	if (!manager)
		return;

//	if (!d->autoConnect.isEmpty())
//		CommandHandler::commandHandler()->processMessage( QString::fromLatin1("/join %1").arg(d->autoConnect), manager);

	QStringList commands(connectCommands());
//	for (QStringList::Iterator it=commands.begin(); it != commands.end(); ++it)
//		CommandHandler::commandHandler()->processMessage(*it, manager);
}

void IRCAccount::quit( const QString &quitMessage )
{
	kDebug(14120) << "Quitting IRC: " << quitMessage << endl;

//	KIrc::StdCommands::quit(d->client, quitMessage.isEmpty() ? defaultQuitMessage() : quitMessage);
}

void IRCAccount::setAway(bool isAway, const QString &awayMessage)
{
	kDebug(14120) << k_funcinfo << isAway << " " << awayMessage << endl;
//	KIrc::StdCommands::away(d->client, awayMessage);
}

void IRCAccount::slotShowServerWindow()
{
	d->server->startChat();
}

bool IRCAccount::isConnected()
{
	kDebug(14120) << k_funcinfo << endl;
	return d->client->isConnected();
}

void IRCAccount::setOnlineStatus(const OnlineStatus& status , const StatusMessage &messageStatus)
{
	kDebug(14120) << k_funcinfo << endl;
	d->expectedOnlineStatus = status;
	//d->expectedReason = reason;

	OnlineStatus::StatusType current = myself()->onlineStatus().status();
	OnlineStatus::StatusType expected = d->expectedOnlineStatus.status();

	if ( expected != OnlineStatus::Offline && (current == OnlineStatus::Offline || current == OnlineStatus::Unknown) )
	{
		kDebug(14120) << k_funcinfo << "Connecting." << endl;
		clientSetup();
//		clientConnect();
		connect();
	}

	if ( expected == OnlineStatus::Offline && current != OnlineStatus::Offline )
	{
		kDebug(14120) << k_funcinfo << "Disconnecting." << endl;
		//quit(reason);
	}
}

void IRCAccount::setStatusMessage(const StatusMessage &messageStatus)
{
	kDebug(14120) << k_funcinfo << endl;
}

bool IRCAccount::createContact(const QString &contactId, MetaContact *metac)
{
	kDebug(14120) << k_funcinfo << endl;
/*	if (contactId == mNickName)
	{
		KMessageBox::error( UI::Global::mainWidget(),
			i18n("\"You are not allowed to add yourself to your contact list."), i18n("IRC Plugin")
		);

		return false;
	}
	IRCContact *contact = getContact(contactId, metac);

	if (contact->metaContact() != metac )
	{//This should NEVER happen
		MetaContact *old = contact->metaContact();
		contact->setMetaContact(metac);
		ContactPtrList children = old->contacts();
		if (children.isEmpty())
			ContactList::self()->removeMetaContact( old );
	}
	else if (contact->metaContact()->isTemporary())
		metac->setTemporary(false);
*/
	return true;
}

void IRCAccount::setCurrentCommandSource( ChatSession *session )
{
	kDebug(14120) << k_funcinfo << endl;
	d->commandSource = session;
}

ChatSession *IRCAccount::currentCommandSource()
{
	kDebug(14120) << k_funcinfo << endl;
	return d->commandSource;
}

IRCContact *IRCAccount::myServer() const
{
	return d->server;
}

IRCContact *IRCAccount::mySelf() const
{
	return d->self;
}

IRCContact *IRCAccount::getContact(const QByteArray &name, MetaContact *metac)
{
	kDebug(14120) << k_funcinfo << name << endl;
//	return getContact(d->client->entityManager()->entityByName(name), metac);
	return 0;
}

IRCContact *IRCAccount::getContact(const KIrc::Entity::Ptr &entity, MetaContact *metac)
{
	IRCContact *contact = 0;

#ifdef __GNUC__
	#warning Do the search code here.
#endif

	if (!contact)
	{
#ifdef __GNUC__
		#warning Make a temporary meta contact if metac is null
#endif
		contact = new IRCContact(this, entity, metac);
		d->contacts.append(contact);
	}

	QObject::connect(contact, SIGNAL(destroyed(IRCContact *)), SLOT(destroyed(IRCContact *)));
	return contact;
}

void IRCAccount::destroyed(IRCContact *contact)
{
	kDebug(14120) << k_funcinfo << endl;
	d->contacts.remove(contact);
}

void IRCAccount::receivedEvent(KIrc::Event *event)
{
/*
	IRCContact *from = getContact(event->from());
	QList<IRCContact*> to = getContacts(event->to());
	QList<IRCContact*> cc = getContacts(event->cc());

	Kopete::Message::MessageDirection msgDirection =
		event->from() == mySelf ? Kopete::Message::OutBound : Kopete::Message::Indound;

	Kopete::Message::MessageType msgType;
	switch (type)
	{
	case KIrc::????: // Action
		msgType = Kopete::Message::TypeAction;
		break;
	default:
		msgType = Kopete::Message::????;
	}

//	make a notification if needed, istead of posting the message to the toContact.
//	toContact may be the wrong contact where to post in case of private user chat

	Message msg(event->from(), manager()->members(), message, msgDirection,
		    Kopete::Message::RichText, CHAT_VIEW, msgType);

	foreach
		postContact->appendMessage(msg);
*/
}
/*
void IRCContact::slotUserDisconnected(const QString &user, const QString &reason)
{
	if (d->chatSession)
	{
		QString nickname = user.section('!', 0, 0);
		Contact *c = locateUser( nickname );
		if ( c )
		{
			d->chatSession->removeContact(c, i18n("Quit: \"%1\" ").arg(reason), Message::RichText);
//			c->setOnlineStatus(IRCProtocol::self()->m_UserStatusOffline);
		}
	}
}

void IRCContact::slotNewNickChange(const QString &oldnickname, const QString &newnickname)
{
	IRCAccount *account = ircAccount();

	IRCContact *user = static_cast<IRCContact*>( locateUser(oldnickname) );
	if( user )
	{
		user->setNickName( newnickname );

		//If the user is in our contact list, then change the notify list nickname
//		if (!user->metaContact()->isTemporary())
//		{
//			account->contactManager()->removeFromNotifyList( oldnickname );
//			account->contactManager()->addToNotifyList( newnickname );
//		}
	}
}

void IRCAccount::successfullyChangedNick(const QString &oldnick, const QString &newnick)
{
//	kDebug(14120) << k_funcinfo << "Changing nick to " << newnick << endl;
//	mNickName = newnick;
//	mySelf()->setNickName( mNickName );
//	d->contactManager->removeFromNotifyList( oldnick );
//	d->contactManager->addToNotifyList( newnick );
}
*/
#include "ircaccount.moc"

