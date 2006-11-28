/*
    ircaccount.cpp - IRC Account

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2004 by Jason Keirstead <jason@keirstead.org>
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
#include "irccontactmanager.h"
#include "ircprotocol.h"

#include "ircservercontact.h"
#include "ircchannelcontact.h"
#include "ircusercontact.h"

#include "channellistdialog.h"

#include "kircengine.h"

#include "kopeteaccountmanager.h"
#include "kopeteaway.h"
#include "kopeteawayaction.h"
#include "kopetecommandhandler.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include "kopeteview.h"
#include "kopetepassword.h"

#include <kaction.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcompletionbox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kpopupmenu.h>

#include <qlayout.h>
#include <qtimer.h>

const QString IRCAccount::CONFIG_CODECMIB = QString::fromLatin1("Codec");
const QString IRCAccount::CONFIG_NETWORKNAME = QString::fromLatin1("NetworkName");
const QString IRCAccount::CONFIG_NICKNAME = QString::fromLatin1("NickName");
const QString IRCAccount::CONFIG_USERNAME = QString::fromLatin1("UserName");
const QString IRCAccount::CONFIG_REALNAME = QString::fromLatin1("RealName");

IRCAccount::IRCAccount(IRCProtocol *protocol, const QString &accountId, const QString &autoChan, const QString& netName, const QString &nickName)
	: Kopete::PasswordedAccount(protocol, accountId, 0, true), autoConnect( autoChan ), commandSource(0)
{
	m_manager = 0L;
	m_channelList = 0L;
	m_network = 0L;

	triedAltNick = false;

	m_contactManager = 0;
	m_engine = new KIRC::Engine(this);

	QMap< QString, QString> replies = customCtcpReplies();
	for( QMap< QString, QString >::ConstIterator it = replies.begin(); it != replies.end(); ++it )
		m_engine->addCustomCtcp( it.key(), it.data() );

	QString version=i18n("Kopete IRC Plugin %1 [http://kopete.kde.org]").arg(kapp->aboutData()->version());
	m_engine->setVersionString( version  );

	QObject::connect(m_engine, SIGNAL(successfullyChangedNick(const QString &, const QString &)),
			this, SLOT(successfullyChangedNick(const QString &, const QString &)));

	QObject::connect(m_engine, SIGNAL(incomingFailedServerPassword()),
			this, SLOT(slotFailedServerPassword()));

	QObject::connect(m_engine, SIGNAL(incomingNickInUse(const QString &)),
			this, SLOT(slotNickInUseAlert( const QString &)) );

	QObject::connect(m_engine, SIGNAL(incomingFailedNickOnLogin(const QString &)),
			this, SLOT(slotNickInUse( const QString &)) );

	QObject::connect(m_engine, SIGNAL(incomingJoinedChannel(const QString &, const QString &)),
		this, SLOT(slotJoinedUnknownChannel(const QString &, const QString &)));

	QObject::connect(m_engine, SIGNAL(incomingCtcpReply(const QString &, const QString &, const QString &)),
			this, SLOT( slotNewCtcpReply(const QString&, const QString &, const QString &)));

	QObject::connect(m_engine, SIGNAL(statusChanged(KIRC::Engine::Status)),
			 this, SLOT(engineStatusChanged(KIRC::Engine::Status)));

	QObject::connect(m_engine, SIGNAL(incomingServerLoadTooHigh()),
		this, SLOT(slotServerBusy()));

	QObject::connect(m_engine, SIGNAL(incomingNoSuchNickname(const QString &)),
			 this, SLOT(slotNoSuchNickname(const QString &)));

	mAwayAction = new Kopete::AwayAction ( i18n("Set Away"),
		m_protocol->m_UserStatusAway.iconFor( this ), 0, this,
		SLOT(slotGoAway( const QString & )), this );

	currentHost = 0;

	KConfigGroup *config = configGroup();

	QString networkName = netName;
	if (networkName.isNull())
		networkName = config->readEntry(CONFIG_NETWORKNAME);

	if (!nickName.isNull())
		setNickName(nickName);
	else
		mNickName = config->readEntry(CONFIG_NICKNAME);

	QString codecMib = config->readEntry(CONFIG_CODECMIB);
	//	int codecMib = config->readNumEntry(CONFIG_CODECMIB, UTF-8);

	m_serverNotices = (MessageDestination)config->readNumEntry( "ServerNotices", ServerWindow );
	m_serverMessages = (MessageDestination)config->readNumEntry( "ServerMessages", ServerWindow );
	m_informationReplies = (MessageDestination)config->readNumEntry( "InformationReplies", ActiveWindow );
	m_errorMessages = (MessageDestination)config->readNumEntry( "ErrorMessages", ActiveWindow );
	autoShowServerWindow = config->readBoolEntry( "AutoShowServerWindow", false );

	if( !codecMib.isEmpty() )
	{
		mCodec = QTextCodec::codecForMib( codecMib.toInt() );
		m_engine->setDefaultCodec( mCodec );
	}
	else
		mCodec = 0;

	QString m_accountId = this->accountId();
	if( networkName.isEmpty() && QRegExp( "[^#+&\\s]+@[\\w-\\.]+:\\d+" ).exactMatch( m_accountId ) )
	{
		kdDebug(14120) << "Creating account from " << m_accountId << endl;

		mNickName = m_accountId.section('@',0,0);
		QString serverInfo = m_accountId.section('@',1);
		QString hostName = serverInfo.section(':',0,0);

		for( QDictIterator<IRCNetwork> it( m_protocol->networks() ); it.current(); ++it )
		{
			IRCNetwork *net = it.current();
			for( QValueList<IRCHost*>::iterator it2 = net->hosts.begin(); it2 != net->hosts.end(); ++it2 )
			{
				if( (*it2)->host == hostName )
				{
					setNetwork(net->name);
					break;
				}
			}

			if( !networkName.isEmpty() )
				break;
		}

		if( networkName.isEmpty() )
		{
			/* Could not find this host. Add it to the networks structure */

			m_network = new IRCNetwork;
			m_network->name = i18n("Temporary Network - %1").arg( hostName );
			m_network->description = i18n("Network imported from previous version of Kopete, or an IRC URI");

			IRCHost *host = new IRCHost;
			host->host = hostName;
			host->port = serverInfo.section(':',1).toInt();
			if( !password().cachedValue().isEmpty() )
				host->password = password().cachedValue();
			host->ssl = false;

			m_network->hosts.append( host );
			m_protocol->addNetwork( m_network );

			config->writeEntry(CONFIG_NETWORKNAME, m_network->name);
			config->writeEntry(CONFIG_NICKNAME, mNickName);
		}
	}
	else if( !networkName.isEmpty() )
	{
		setNetwork(networkName);
	}
	else
	{
		kdError() << "No network name defined, and could not import network information from ID" << endl;
	}

	m_engine->setUserName(userName());
	m_engine->setRealName(realName());

	m_contactManager = new IRCContactManager(mNickName, this);
	setMyself( m_contactManager->mySelf() );
	setAccountLabel( QString::fromLatin1("%1@%2").arg(mNickName,networkName) );
	m_myServer = m_contactManager->myServer();

	m_joinChannelAction = new KAction ( i18n("Join Channel..."), QString::null, 0, this,
				SLOT(slotJoinChannel()), this);
	m_searchChannelAction = new KAction ( i18n("Search Channels..."), QString::null, 0, this,
				SLOT(slotSearchChannels()), this);
}

IRCAccount::~IRCAccount()
{
	if (m_engine->isConnected())
		m_engine->quit(i18n("Plugin Unloaded"), true);
}

void IRCAccount::slotNickInUse( const QString &nick )
{
	QString altNickName = altNick();
	if( triedAltNick || altNickName.isEmpty() )
	{
		QString newNick = KInputDialog::getText(
				i18n("IRC Plugin"),
				i18n("The nickname %1 is already in use. Please enter an alternate nickname:").arg(nick),
				nick);

		if (newNick.isNull())
			disconnect();
		else
			m_engine->nick(newNick);
	}
	else
	{
		triedAltNick = true;
		m_engine->nick(altNickName);
	}
}

void IRCAccount::slotNickInUseAlert( const QString &nick )
{
	KMessageBox::error(Kopete::UI::Global::mainWidget(), i18n("The nickname %1 is already in use").arg(nick), i18n("IRC Plugin"));
}

void IRCAccount::setAltNick( const QString &altNick )
{
	configGroup()->writeEntry(QString::fromLatin1( "altNick" ), altNick);
}

const QString IRCAccount::altNick() const
{
	return configGroup()->readEntry(QString::fromLatin1("altNick"));
}

void IRCAccount::setAutoShowServerWindow( bool show )
{
	autoShowServerWindow = show;
	configGroup()->writeEntry(QString::fromLatin1( "AutoShowServerWindow" ), autoShowServerWindow);
}

const QString IRCAccount::networkName() const
{
	if( m_network )
		return m_network->name;
	else
		return i18n("Unknown");
}

void IRCAccount::setUserName( const QString &userName )
{
	m_engine->setUserName(userName);
	configGroup()->writeEntry(CONFIG_USERNAME, userName);
}

const QString IRCAccount::userName() const
{
	return configGroup()->readEntry(CONFIG_USERNAME);
}

void IRCAccount::setRealName( const QString &userName )
{
	m_engine->setRealName(userName);
	configGroup()->writeEntry(CONFIG_REALNAME, userName);
}

const QString IRCAccount::realName() const
{
	return configGroup()->readEntry(CONFIG_REALNAME);
}

void IRCAccount::setNetwork( const QString &network )
{
	IRCNetwork *net = m_protocol->networks()[ network ];
	if( net )
	{
		m_network = net;
		configGroup()->writeEntry(CONFIG_NETWORKNAME, network);
		setAccountLabel(network);
	}
	else
	{
		KMessageBox::queuedMessageBox(
		Kopete::UI::Global::mainWidget(), KMessageBox::Error,
		i18n("<qt>The network associated with this account, <b>%1</b>, no longer exists. Please"
		" ensure that the account has a valid network. The account will not be enabled until you do so.</qt>").arg(network),
		i18n("Problem Loading %1").arg( accountId() ), 0 );
	}
}

void IRCAccount::setNickName( const QString &nick )
{
	mNickName = nick;
	configGroup()->writeEntry(CONFIG_NICKNAME, mNickName);

	if( mySelf() )
		mySelf()->setNickName( mNickName );
}

// FIXME: Possible null pointer usage here
void IRCAccount::setCodec( QTextCodec *codec )
{
	mCodec = codec;
	configGroup()->writeEntry(CONFIG_CODECMIB, codec->mibEnum());

	if( mCodec )
		m_engine->setDefaultCodec( mCodec );
}

QTextCodec *IRCAccount::codec() const
{
	return mCodec;
}

// FIXME: Move this to a dictionnary
void IRCAccount::setDefaultPart( const QString &defaultPart )
{
	configGroup()->writeEntry( QString::fromLatin1( "defaultPart" ), defaultPart );
}

// FIXME: Move this to a dictionnary
void IRCAccount::setDefaultQuit( const QString &defaultQuit )
{
	configGroup()->writeEntry( QString::fromLatin1( "defaultQuit" ), defaultQuit );
}

// FIXME: Move this to a dictionnary
const QString IRCAccount::defaultPart() const
{
	QString partMsg = configGroup()->readEntry(QString::fromLatin1("defaultPart"));
	if( partMsg.isEmpty() )
		return QString::fromLatin1("Kopete %1 : http://kopete.kde.org").arg( kapp->aboutData()->version() );
	return partMsg;
}

const QString IRCAccount::defaultQuit() const
{
	QString quitMsg = configGroup()->readEntry(QString::fromLatin1("defaultQuit"));
	if( quitMsg.isEmpty() )
		return QString::fromLatin1("Kopete %1 : http://kopete.kde.org").arg(kapp->aboutData()->version());
	return quitMsg;
}

void IRCAccount::setCustomCtcpReplies( const QMap< QString, QString > &replies ) const
{
	QStringList val;
	for( QMap< QString, QString >::ConstIterator it = replies.begin(); it != replies.end(); ++it )
	{
		m_engine->addCustomCtcp( it.key(), it.data() );
		val.append( QString::fromLatin1("%1=%2").arg( it.key() ).arg( it.data() ) );
	}

	configGroup()->writeEntry( "CustomCtcp", val );
}

const QMap< QString, QString > IRCAccount::customCtcpReplies() const
{
	QMap< QString, QString > replies;
	QStringList replyList;

	replyList = configGroup()->readListEntry( "CustomCtcp" );

	for( QStringList::Iterator it = replyList.begin(); it != replyList.end(); ++it )
		replies[ (*it).section('=', 0, 0 ) ] = (*it).section('=', 1 );

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

void IRCAccount::setMessageDestinations( int serverNotices, int serverMessages,
			     int informationReplies, int errorMessages )
{
	KConfigGroup *config =  configGroup();
	config->writeEntry( "ServerNotices", serverNotices );
	config->writeEntry( "ServerMessages", serverMessages );
	config->writeEntry( "InformationReplies", informationReplies );
	config->writeEntry( "ErrorMessages", errorMessages );

	m_serverNotices = (MessageDestination)serverNotices;
	m_serverMessages = (MessageDestination)serverMessages;
	m_informationReplies = (MessageDestination)informationReplies;
	m_errorMessages = (MessageDestination)errorMessages;
}

KActionMenu *IRCAccount::actionMenu()
{
	QString menuTitle = QString::fromLatin1( " %1 <%2> " ).arg( accountId() ).arg( myself()->onlineStatus().description() );

	KActionMenu *mActionMenu = Kopete::Account::actionMenu();

	m_joinChannelAction->setEnabled( isConnected() );
	m_searchChannelAction->setEnabled( isConnected() );

	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(m_joinChannelAction);
	mActionMenu->insert(m_searchChannelAction);
	mActionMenu->insert( new KAction ( i18n("Show Server Window"), QString::null, 0, this, SLOT(slotShowServerWindow()), mActionMenu ) );

	if( m_engine->isConnected() && m_engine->useSSL() )
	{
		mActionMenu->insert( new KAction ( i18n("Show Security Information"), "", 0, m_engine,
			SLOT(showInfoDialog()), mActionMenu ) );
	}

	return mActionMenu;
}

void IRCAccount::connectWithPassword(const QString &password)
{
	//TODO:  honor the initial status

	if( m_engine->isConnected() )
	{
		if( isAway() )
			setAway( false );
	}
	else if( m_engine->isDisconnected() )
	{
		if( m_network )
		{
			QValueList<IRCHost*> &hosts = m_network->hosts;
			if( hosts.count() == 0 )
			{
				KMessageBox::queuedMessageBox(
					Kopete::UI::Global::mainWidget(), KMessageBox::Error,
					i18n("<qt>The network associated with this account, <b>%1</b>, has no valid hosts. Please ensure that the account has a valid network.</qt>").arg(m_network->name),
					i18n("Network is Empty"), 0 );
			}
			else if( currentHost == hosts.count() )
			{
			    KMessageBox::queuedMessageBox(
				    Kopete::UI::Global::mainWidget(), KMessageBox::Error,
			    i18n("<qt>Kopete could not connect to any of the servers in the network associated with this account (<b>%1</b>). Please try again later.</qt>").arg(m_network->name),
			    i18n("Network is Unavailable"), 0 );

			    currentHost = 0;
			}
			else
			{
				// if prefer SSL is set, sort by SSL first
				if (configGroup()->readBoolEntry("PreferSSL"))
				{
					typedef QValueList<IRCHost*> IRCHostList;
					IRCHostList sslFirst;
					IRCHostList::iterator it;
					for ( it = hosts.begin(); it != hosts.end(); ++it )
					{
						if ( (*it)->ssl == true )
						{
							sslFirst.append( *it );
							it = hosts.remove( it );
						}
					}
					for ( it = hosts.begin(); it != hosts.end(); ++it )
						sslFirst.append( *it );

					hosts = sslFirst;
				}

				IRCHost *host = hosts[ currentHost++ ];
				myServer()->appendMessage( i18n("Connecting to %1...").arg( host->host ) );
				if( host->ssl )
					myServer()->appendMessage( i18n("Using SSL") );

				m_engine->setPassword(password);
				m_engine->connectToServer( host->host, host->port, mNickName, host->ssl );
			}
		}
		else
		{
			kdWarning() << "No network defined!" << endl;
		}
	}
}

void IRCAccount::engineStatusChanged(KIRC::Engine::Status newStatus)
{
	kdDebug(14120) << k_funcinfo << endl;

	mySelf()->updateStatus();

	switch (newStatus)
	{
	case KIRC::Engine::Idle:
		// Do nothing.
		break;
	case KIRC::Engine::Connecting:
	{
		if( autoShowServerWindow )
		    myServer()->startChat();
		break;
	}
	case KIRC::Engine::Authentifying:
		break;
	case KIRC::Engine::Connected:
		{
			//Reset the host so re-connection will start over at first server
			currentHost = 0;
			m_contactManager->addToNotifyList( m_engine->nickName() );

			// HACK! See bug #85200 for details. Some servers still cannot accept commands
			// after the 001 is sent, you need to wait until all the init junk is done.
			// Unfortunatly, there is no way for us to know when it is done (it could be
			// spewing out any number of replies), so just try delaying it
			QTimer::singleShot( 250, this, SLOT( slotPerformOnConnectCommands() ) );
		}
		break;
	case KIRC::Engine::Closing:
		triedAltNick = false;
//		mySelf()->setOnlineStatus( m_protocol->m_UserStatusOffline );
		m_contactManager->removeFromNotifyList( m_engine->nickName() );

//		if (m_contactManager && !autoConnect.isNull())
//			Kopete::AccountManager::self()->removeAccount( this );
		break;
	case KIRC::Engine::AuthentifyingFailed:
		break;
	case KIRC::Engine::Timeout:
		//Try next server
		connect();
		break;
	case KIRC::Engine::Disconnected:
		break;
	}
}

void IRCAccount::slotPerformOnConnectCommands()
{
	Kopete::ChatSession *manager = myServer()->manager(Kopete::Contact::CanCreate);
	if (!manager)
		return;

	if (!autoConnect.isEmpty())
		Kopete::CommandHandler::commandHandler()->processMessage( QString::fromLatin1("/join %1").arg(autoConnect), manager);

	QStringList commands(connectCommands());
	for (QStringList::Iterator it=commands.begin(); it != commands.end(); ++it)
		Kopete::CommandHandler::commandHandler()->processMessage(*it, manager);
}

void IRCAccount::slotJoinedUnknownChannel(const QString &channel, const QString &nick)
{
	if ( nick.lower() == m_contactManager->mySelf()->nickName().lower() )
	{
		m_contactManager->findChannel(channel)->join();
	}
}

void IRCAccount::disconnect()
{
	quit();
}

void IRCAccount::slotServerBusy()
{
	KMessageBox::queuedMessageBox(
		Kopete::UI::Global::mainWidget(), KMessageBox::Error,
		i18n("The IRC server is currently too busy to respond to this request."),
		i18n("Server is Busy"), 0
	);
}

void IRCAccount::slotSearchChannels()
{
	if( !m_channelList )
	{
		m_channelList = new ChannelListDialog( m_engine,
			i18n("Channel List for %1").arg( m_engine->currentHost() ), this,
			SLOT( slotJoinNamedChannel( const QString & ) ) );
	}
	else
		m_channelList->clear();

	m_channelList->show();
}

void IRCAccount::listChannels()
{
	slotSearchChannels();
	m_channelList->search();
}

void IRCAccount::quit( const QString &quitMessage )
{
	kdDebug(14120) << "Quitting IRC: " << quitMessage << endl;

	if( quitMessage.isNull() || quitMessage.isEmpty() )
		m_engine->quit( defaultQuit() );
	else
		m_engine->quit( quitMessage );
}

void IRCAccount::setAway(bool isAway, const QString &awayMessage)
{
	kdDebug(14120) << k_funcinfo << isAway << " " << awayMessage << endl;
	if(m_engine->isConnected())
	{
		static_cast<IRCUserContact *>( myself() )->setAway( isAway );
		engine()->away(isAway, awayMessage);
	}
}

/*
 * Ask for server password, and reconnect
 */
void IRCAccount::slotFailedServerPassword()
{
	// JLN
	password().setWrong();
	connect();
}
void IRCAccount::slotGoAway( const QString &reason )
{
	setAway( true, reason );
}

void IRCAccount::slotShowServerWindow()
{
	m_myServer->startChat();
}

bool IRCAccount::isConnected()
{
//	return ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline );
	return m_engine->isConnected();
}

void IRCAccount::setOnlineStatus( const Kopete::OnlineStatus& status , const QString &reason )
{
	if ( status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
		connect();
	else if (status.status() == Kopete::OnlineStatus::Online &&
			myself()->onlineStatus().status() == Kopete::OnlineStatus::Away )
		setAway( false );
	else if ( status.status() == Kopete::OnlineStatus::Offline )
		disconnect();
	else if ( status.status() == Kopete::OnlineStatus::Away )
		slotGoAway( reason );
}


void IRCAccount::successfullyChangedNick(const QString &oldnick, const QString &newnick)
{
	kdDebug(14120) << k_funcinfo << "Changing nick to " << newnick << endl;
	mNickName = newnick;
	mySelf()->setNickName( mNickName );
	m_contactManager->removeFromNotifyList( oldnick );
	m_contactManager->addToNotifyList( newnick );
}

bool IRCAccount::createContact( const QString &contactId, Kopete::MetaContact *m )
{
	kdDebug(14120) << k_funcinfo << contactManager() << endl;
	IRCContact *c;

	if( !m )
	{//This should NEVER happen
		m = new Kopete::MetaContact();
		Kopete::ContactList::self()->addMetaContact(m);
	}

	if( contactId == mNickName )
	{
		KMessageBox::error( Kopete::UI::Global::mainWidget(),
			i18n("\"You are not allowed to add yourself to your contact list."), i18n("IRC Plugin")
		);

		return false;
	}
	else if (contactId.startsWith(QString::fromLatin1("#")))
	{
		c = static_cast<IRCContact*>(contactManager()->findChannel(contactId, m));
	}
	else
	{
		m_contactManager->addToNotifyList( contactId );
		c = static_cast<IRCContact*>(contactManager()->findUser(contactId, m));
	}

	if( c->metaContact() != m )
	{//This should NEVER happen
		Kopete::MetaContact *old = c->metaContact();
		c->setMetaContact( m );
		Kopete::ContactPtrList children = old->contacts();
		if (children.isEmpty())
			Kopete::ContactList::self()->removeMetaContact( old );
	}
	else if( c->metaContact()->isTemporary() )
		m->setTemporary(false);

	return true;
}

void IRCAccount::slotJoinNamedChannel(const QString &chan)
{
	contactManager()->findChannel(chan)->startChat();
}

void IRCAccount::setCurrentCommandSource( Kopete::ChatSession *session )
{
	commandSource = session;
}

Kopete::ChatSession *IRCAccount::currentCommandSource()
{
	return commandSource;
}

void IRCAccount::slotJoinChannel()
{
	if (!isConnected())
		return;

	QStringList chans = configGroup()->readListEntry( "Recent Channel list" );
	//kdDebug(14120) << "Recent channel list from config: " << chans << endl;

	KLineEditDlg dlg(
		i18n("Please enter name of the channel you want to join:"),
		QString::null,
		Kopete::UI::Global::mainWidget()
	);

	KCompletion comp;
	comp.insertItems( chans );

	dlg.lineEdit()->setCompletionObject( &comp );
	dlg.lineEdit()->setCompletionMode( KGlobalSettings::CompletionPopup );

	while( true )
	{
		if( dlg.exec() != QDialog::Accepted )
			break;

		QString chan = dlg.text();
		if( chan.isNull() )
			break;

		if( KIRC::Entity::isChannel( chan ) )
		{
			contactManager()->findChannel( chan )->startChat();

			// push the joined channel to first in list
			chans.remove( chan );
			chans.prepend( chan );

			configGroup()->writeEntry( "Recent Channel list", chans );
			break;
		}

		KMessageBox::error( Kopete::UI::Global::mainWidget(),
			i18n("\"%1\" is an invalid channel. Channels must start with '#', '!', '+', or '&'.").arg(chan),
			i18n("IRC Plugin")
			);
	}
}

void IRCAccount::slotNewCtcpReply(const QString &type, const QString &/*target*/, const QString &messageReceived)
{
	appendMessage( i18n("CTCP %1 REPLY: %2").arg(type).arg(messageReceived), InfoReply );
}

void IRCAccount::slotNoSuchNickname( const QString &nick )
{
	if( KIRC::Entity::isChannel(nick) )
		appendMessage( i18n("The channel \"%1\" does not exist").arg(nick), UnknownReply );
	else
		appendMessage( i18n("The nickname \"%1\" does not exist").arg(nick), UnknownReply );
}

void IRCAccount::appendMessage( const QString &message, MessageType type )
{
	// TODO: Impliment a UI where people can pick  multiple destinations
	// for a message type, and make codethis handle it

	MessageDestination destination;

	switch( type )
	{
		case ConnectReply:
			destination = m_serverMessages;
			break;
		case InfoReply:
			destination = m_informationReplies;
			break;
		case NoticeReply:
			destination = m_serverNotices;
			break;
		case ErrorReply:
			destination = m_errorMessages;
			break;
		case UnknownReply:
		default:
			destination = ActiveWindow;
			break;
	}

	if( destination == ActiveWindow )
	{
		KopeteView *activeView = Kopete::ChatSessionManager::self()->activeView();
		if( activeView && activeView->msgManager()->account() == this )
		{
			Kopete::ChatSession *manager = activeView->msgManager();
			Kopete::Message msg( manager->myself(), manager->members(), message,
				Kopete::Message::Internal, Kopete::Message::RichText, CHAT_VIEW );
			activeView->appendMessage(msg);
		}
	}

	if( destination == AnonymousWindow )
	{
		//TODO: Create an anonymous window??? What will this mean...
	}

	if( destination == ServerWindow )
	{
		myServer()->appendMessage(message);
	}

	if( destination == KNotify )
	{
		KNotifyClient::event(
			Kopete::UI::Global::mainWidget()->winId(), QString::fromLatin1("irc_event"), message
		);
	}
}

IRCUserContact *IRCAccount::mySelf() const
{
	return static_cast<IRCUserContact *>( myself() );
}

IRCServerContact *IRCAccount::myServer() const
{
	return m_myServer;
}

IRCContact *IRCAccount::getContact(const QString &name, Kopete::MetaContact *metac)
{
	return getContact(m_engine->getEntity(name), metac);
}

IRCContact *IRCAccount::getContact(KIRC::EntityPtr entity, Kopete::MetaContact *metac)
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
		m_contacts.append(contact);
	}

	QObject::connect(contact, SIGNAL(destroyed(IRCContact *)), SLOT(destroyed(IRCContact *)));
	return contact;
}

void IRCAccount::destroyed(IRCContact *contact)
{
	m_contacts.remove(contact);
}

#include "ircaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

