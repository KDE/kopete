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

#include "kirccontext.h"
#include "kircclientsocket.h"
#include "kircstdmessages.h"
#include "kircconst.h"
#include "kircevent.h"

#include "kopeteaccountmanager.h"
#include "kopetechatsessionmanager.h"
#include "kopetecommandhandler.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include "kopeteview.h"
#include "kopetepassword.h"

#include <kactionmenu.h>
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
#include <QTimer>

using namespace IRC;
using namespace Kopete;

class IRCAccount::Private
{
public:
	Private()
		: manager(0)
		, client(0)
		, server(0)
		, self(0)
		, commandSource(0)
		, joinChannelAction(0)
		, searchChannelAction(0)
	{ }

	Kopete::ChatSession *manager;
	QString autoConnect;

	KIrc::Context *clientContext;
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

	KAction *joinChannelAction;
	KAction *searchChannelAction;

	QString motd;
};

IRCAccount::IRCAccount(const QString &accountId, const QString &autoChan, const QString& netName, const QString &nickName)
	: PasswordedAccount(IRCProtocol::self(), accountId, true),
	  d( new Private )
{
	d->clientContext = new KIrc::Context(this);
	d->client = new KIrc::ClientSocket(d->clientContext);
	d->autoConnect = autoChan;
	d->currentHost = 0;

	QObject::connect(d->client, SIGNAL(connectionStateChanged(KIrc::Socket::ConnectionState)),
			 this, SLOT(clientConnectionStateChanged(KIrc::Socket::ConnectionState)));

	QObject::connect(d->clientContext,SIGNAL( ircEvent( QEvent* ) ),
			 this, SLOT( receivedEvent( QEvent* ) ) );

//	loadProperties();

	d->server = new IRCContact(this, d->client->server());
	d->self = new IRCContact(this, d->client->owner());
	d->contacts.append( d->server );
	d->contacts.append( d->self );

	setMyself(d->self);

	kDebug()<<"accId="<<accountId<<" autoChan="<<autoChan<<" netName="<<netName<<" nickname="<<nickName;

	QString networkName=netName;
	if(networkName.isEmpty())
		networkName=this->networkName();

	if ( networkName.isEmpty() && QRegExp( "[^#+&\\s]+@[\\w-\\.]+:\\d+" ).exactMatch( accountId ) )
	{
		kDebug(14120) << "Creating account from " << accountId;

//		mNickName = accountId.section('@',0,0);
		QString serverInfo = accountId.section('@',1);
		QString hostName = serverInfo.section(':',0,0);

		IRC::NetworkList networks = IRC::Networks::self()->networks();
		foreach(const IRC::Network &net, networks)
		{
			foreach(const IRC::Host& host, net.hosts)
			{
				if( host.host == hostName )
				{
					setNetworkByName(net.name);
					break;
				}
			}

			if( !networkName.isEmpty() )
				break;
		}

		if( networkName.isEmpty() )
		{
			// Could not find this host. Add it to the networks structure

			d->network.name = i18n("Temporary Network - %1", hostName );
			d->network.description = i18n("Network imported from previous version of Kopete, or an IRC URI");

			IRC::Host host;
			host.host = hostName;
			host.port = serverInfo.section(':',1).toInt();
			if (!password().cachedValue().isEmpty())
				host.password = password().cachedValue();
			host.ssl = false;

			d->network.hosts.append( host );
//			d->protocol->addNetwork( d->network );

			setNetworkByName(networkName);
//			config->writeEntry(Config::NICKNAME, mNickName);
		}
	}
	else if( !networkName.isEmpty() )
	{
		setNetworkByName(networkName);
	}
	else
	{
		kError() << "No network name defined, and could not import network information from ID" << endl;
	}


//	setAccountLabel( QString::fromLatin1("%1@%2").arg(mNickName,networkName) );


#ifdef __GNUC__
	#warning spurus slot calls for now
#endif
	d->joinChannelAction = new KAction(i18n("Join Channel..."), this);
	QObject::connect(d->joinChannelAction, SIGNAL(triggered()), this, SLOT(slotJoinChannel()));
	d->searchChannelAction = new KAction(i18n("Search Channels..."), this);
	QObject::connect(d->searchChannelAction, SIGNAL(triggered()), this, SLOT(slotSearchChannels()));
}

IRCAccount::~IRCAccount()
{
	kDebug(14120) ;
//	KIrc::StdMessage::quit(d->client, i18n("Plugin Unloaded"));

	delete d;
}


void IRCAccount::clientSetup()
{
	//d->client->setDefaultCodec(codec());
/*
	// Build the URL instead
	KUrl url;
	url.setUser(userName());
//	url.setPass(password());
*/
/*
	d->client->setNickName(nickName());
	url.addQuery(URL_REALNAME, realName());
	d->client->setVersionString(IRC::Version);

	QMap<QString, QString> replies = customCtcpReplies();
	for (QMap<QString, QString>::ConstIterator it = replies.begin(); it != replies.end(); ++it)
		d->client->addCustomCtcp(it.key(), it.data());
*/

	d->network = IRC::Networks::self()->network(networkName());

	// if prefer SSL is set, sort by SSL first
	if (configGroup()->readEntry("PreferSSL",false))
	{
		QList<IRC::Host> sslFirst;
		QList<IRC::Host> noSSL;

		foreach(const IRC::Host &host,d->network.hosts)
		{
			if ( host.ssl == true )
			{
				sslFirst.append( host );
			}else
			{
				noSSL.append( host );
			}
		}
		//Now append the non ssl servers
		foreach(const IRC::Host &host,noSSL)
		{
			sslFirst.append(host);
		}

		d->network.hosts = sslFirst;
	}

}

void IRCAccount::clientConnect()
{
	kDebug(14120) ;

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
		//appendInternalMessage( i18n("Connecting to %1...", host.host ) );

		QString urlString;

		if (host.ssl) {
			//appendInternalMessage( i18n("Using SSL") );
			urlString = "ircs://";
		} else {
			urlString = "irc://";
		}

		urlString += nickName() + "@" + host.host+":"+QString::number(host.port);

		KUrl url(urlString);
		//TODO use the constants in kircconst.h
		url.addQueryItem("realname",realName());
		url.addQueryItem("nickname",nickName());
		//d->client->connectToServer( host->host, host->port, mNickName, host->ssl );
		d->client->connectToServer(url);
	}
}

int IRCAccount::codecMib() const
{
	kDebug(14120) ;
	return configGroup()->readEntry(Config::CODECMIB, 0);
}

void IRCAccount::setCodecFromMib(int mib)
{
	kDebug(14120) ;
	configGroup()->writeEntry(Config::CODECMIB, mib);
	d->clientContext->setDefaultCodec(QTextCodec::codecForMib(mib));
}

QTextCodec *IRCAccount::codec() const
{
	kDebug(14120) ;
	return QTextCodec::codecForMib(codecMib());
}

void IRCAccount::setCodec( QTextCodec *codec )
{
	kDebug(14120) ;
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
	kDebug(14120) ;
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
	kDebug(14120) ;
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
	configGroup()->writeEntry("ConnectCommands", commands);
}

const QStringList IRCAccount::connectCommands() const
{
	return configGroup()->readEntry("ConnectCommands", QStringList());
}

void IRCAccount::fillActionMenu( KActionMenu *actionMenu )
{
	kDebug(14120) ;
	QString menuTitle = QString::fromLatin1( " %1 <%2> " ).arg( accountId() ).arg( myself()->onlineStatus().description() );

	Account::fillActionMenu( actionMenu );

	d->joinChannelAction->setEnabled( isConnected() );
	d->searchChannelAction->setEnabled( isConnected() );

	actionMenu->addSeparator();
	actionMenu->addAction(d->joinChannelAction);
	actionMenu->addAction(d->searchChannelAction);
/*
	actionMenu->insert( new KAction ( i18n("Show Server Window"), QString(), 0, this, SLOT(slotShowServerWindow()), actionMenu ) );

//	if (d->client->isConnected() && d->client->useSSL())
	{
		actionMenu->insert( new KAction ( i18n("Show Security Information"), "", 0, d->client,
			SLOT(showInfoDialog()), actionMenu ) );
	}
*/
}

void IRCAccount::connectWithPassword(const QString &password)
{
	//	d->client->setPassword(password);

	kDebug(14120) << "Connecting with password.";
	clientConnect();
}

void IRCAccount::clientConnectionStateChanged(KIrc::Socket::ConnectionState newstate)
{
	kDebug(14120) ;

	mySelf()->updateStatus();

	switch (newstate)
	{
	case KIrc::Socket::HostLookup:
	case KIrc::Socket::HostFound:
	case KIrc::Socket::Connecting:
		// d->expectedOnlineStatus check and use it
		mySelf()->setOnlineStatus(Kopete::OnlineStatus::Connecting);

		if (autoShowServerWindow())
			myServer()->startChat();
		break;

	case KIrc::Socket::Authentified:
		mySelf()->setOnlineStatus(Kopete::OnlineStatus::Online);

		//Reset the host so re-connection will start over at first server
		d->currentHost = 0;
//		d->contactManager->addToNotifyList( d->client->nickName() );

		// HACK! See bug #85200 for details. Some servers still cannot accept commands
		// after the 001 is sent, you need to wait until all the init junk is done.
		// Unfortunately, there is no way for us to know when it is done (it could be
		// spewing out any number of replies), so just try delaying it
		QTimer::singleShot( 250, this, SLOT( slotPerformOnConnectCommands() ) );
		break;
/*
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
		kDebug(14120) << "Doing nothing on state" << newstate;
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
	kDebug(14120) ;
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
	kDebug(14120) << "Quitting IRC: " << quitMessage;

//	KIrc::StdCommands::quit(d->client, quitMessage.isEmpty() ? defaultQuitMessage() : quitMessage);
}

void IRCAccount::setAway(bool isAway, const QString &awayMessage)
{
	kDebug(14120) << isAway << " " << awayMessage;
//	KIrc::StdCommands::away(d->client, awayMessage);
}

void IRCAccount::slotShowServerWindow()
{
	d->server->startChat();
}

void IRCAccount::slotJoinChannel()
{
	if (!isConnected())
		return;

	QStringList chans = configGroup()->readEntry( "Recent Channel list", QStringList() );
	//kdDebug(14120) << "Recent channel list from config: " << chans << endl;
	QString channelName=KInputDialog::getText( i18n( "Join Channel" ),
			i18n("Please enter the name of the channel you want to join:"),
			QString(), 0,
			Kopete::UI::Global::mainWidget(),
			0, QString(), 0, chans
		);

	if ( !channelName.isNull() )
	{
		kDebug( 14120 )<<"joining channel"<<channelName;
		chans.prepend( channelName );
		configGroup()->writeEntry( "Recent Channel list", chans );

		KIrc::EntityPtr channel=d->client->joinChannel( channelName.toUtf8() );
		getContact( channel )->startChat();
	}
}

void IRCAccount::setOnlineStatus(const OnlineStatus& status , const StatusMessage &messageStatus, const OnlineStatusOptions& options)
{
	kDebug(14120) ;
	d->expectedOnlineStatus = status;
	//d->expectedReason = reason;

	OnlineStatus::StatusType current = myself()->onlineStatus().status();
	OnlineStatus::StatusType expected = d->expectedOnlineStatus.status();

	if ( expected != OnlineStatus::Offline && (current == OnlineStatus::Offline || current == OnlineStatus::Unknown) )
	{
		kDebug(14120) << "Connecting.";
		clientSetup();
//		clientConnect();
		connect();
	}

	if ( expected == OnlineStatus::Offline && current != OnlineStatus::Offline )
	{
		kDebug(14120) << "Disconnecting.";
		//quit(reason);
	}
}

void IRCAccount::setStatusMessage(const StatusMessage &messageStatus)
{
	kDebug(14120) ;
}

bool IRCAccount::createContact(const QString &contactId, MetaContact *metac)
{
	kDebug(14120) ;
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
	kDebug(14120) ;
	d->commandSource = session;
}

ChatSession *IRCAccount::currentCommandSource()
{
	kDebug(14120) ;
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
	kDebug(14120) << name;
//	return getContact(d->client->entityManager()->entityByName(name), metac);
	return 0;
}

IRCContact *IRCAccount::getContact(const KIrc::EntityPtr &entity, MetaContact *metac)
{
	IRCContact *contact = 0;

	kDebug( 14120)<<"finding contact for name "<<entity->name();

	//TODO: use hash or something to speed up searching?
	foreach( IRCContact *tmp, d->contacts )
	{
		if ( tmp->entity()==entity )
		{
			contact=tmp;
			break;
		}
	}

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

QList<Kopete::Contact*> IRCAccount::getContacts( const KIrc::EntityList &entities )
{
	QList<Kopete::Contact*> contacts;
	foreach( const KIrc::EntityPtr &e, entities )
		contacts<<getContact( e );

	return contacts;
}

void IRCAccount::destroyed(IRCContact *contact)
{
	kDebug(14120) ;
	d->contacts.removeAll(contact);
}

void IRCAccount::receivedEvent(QEvent *event)
{
	kDebug(14120)<<"received event";
	if ( event->type()==KIrc::TextEvent::Type )
	{
		KIrc::TextEvent* txtEvent=static_cast< KIrc::TextEvent* >( event );
		kDebug(14120)<<"type: " << txtEvent->eventId();
		kDebug(14120)<<"from: " << txtEvent->from()->name();
		kDebug(14120)<<"message:" << txtEvent->text();

		IRCContact *from = getContact( txtEvent->from() );
		QList<Kopete::Contact*> to = getContacts( txtEvent->to() );
		Kopete::Message::MessageType msgType = Kopete::Message::TypeNormal;
		Kopete::Message::MessageImportance msgImportance = Kopete::Message::Low;

		if ( txtEvent->eventId()=="PRIVMSG" )
		{
//			if ( !to->isChannel() )
//				importance = Kopete::Message::Normal;
		}
		else if ( txtEvent->eventId() == "DCC_ACTION" )
		{
			msgType = Kopete::Message::TypeAction;
		}
#if 0 
		else if ( txtEvent->eventId().startWith("ERR_") )
		{
			msgImportance = Kopete::Message::Highlight;
		}
#endif
		appendMessage( from, to, txtEvent->text(), msgType );
	}
   	/*
	QList<Kopete::Contact*> to = getContacts(txtEvent->to());
	//QList<IRCContact*> cc = getContacts(txtEvent->cc());

	Kopete::Message::MessageType msgType;
	if ( txtEvent->eventId()=="ServerMessage" )
	{
		msgType = Kopete::Message::TypeAction;
	}

	switch ( type)
	{
	case KIrc::????: // Action
		msgType = Kopete::Message::TypeAction;
		break;
	default:
		msgType = Kopete::Message::????;
	}*/

	//	make a notification if needed, istead of posting the message to the toContact.
	//	toContact may be the wrong contact where to post in case of private user chat

	/*
	foreach
		postContact->appendMessage(msg);
*/
}

void IRCAccount::appendMessage(IRCContact* from, QList<Contact*> to,const QString& text, Kopete::Message::MessageType type)
{
	Kopete::Message::MessageDirection msgDirection =
		from == mySelf() ? Kopete::Message::Outbound : Kopete::Message::Inbound;

	Kopete::Message msg(from, to);
	msg.setDirection( msgDirection );
	msg.setPlainBody( text );
	msg.setType( type );

	foreach( Kopete::Contact* c, to )
	{
		if ( c==myself() ) //If we are the target of the message, append it to the chatsession of the origin
			from->appendMessage( msg );
		else
			dynamic_cast<IRCContact*> ( c )->appendMessage( msg );
	}
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
			d->chatSession->removeContact(c, i18n("Quit: \"%1\" ",reason), Message::RichText);
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
//	kDebug(14120) << "Changing nick to " << newnick;
//	mNickName = newnick;
//	mySelf()->setNickName( mNickName );
//	d->contactManager->removeFromNotifyList( oldnick );
//	d->contactManager->addToNotifyList( newnick );
}
*/
#include "ircaccount.moc"

