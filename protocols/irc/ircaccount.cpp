/*
    ircaccount.cpp - IRC Account

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2004 by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2003-2004 by Michel Hermier <michel.hermier@wanadoo.fr>

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
#include "irccontact.h"
#include "irccontactmanager.h"
#include "ircprotocol.h"
#include "channellist.h"

#include "ircservercontact.h"
#include "ircchannelcontact.h"
#include "ircusercontact.h"

#include "kopeteaway.h"
#include "kopeteawayaction.h"
#include "kopeteuiglobal.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopeteaccountmanager.h"
#include "kopetecommandhandler.h"
#include "kopeteview.h"

#include <kaction.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klineedit.h>
#include <klineeditdlg.h>
#include <kcompletionbox.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kconfig.h>
#include <kglobal.h>
#include <knotifyclient.h>

#include <qlayout.h>
#include <qtimer.h>

ChannelListDialog::ChannelListDialog(KIRC::Engine *engine, const QString &caption, QObject *target, const char* slotJoinChan)
	: KDialogBase(Kopete::UI::Global::mainWidget(), "channel_list_widget", false, caption, Close)
{
	m_engine = engine;
	m_list = new ChannelList( this, engine );

	connect( m_list, SIGNAL( channelDoubleClicked( const QString & ) ),
		target, slotJoinChan );

	connect( m_list, SIGNAL( channelDoubleClicked( const QString & ) ),
		this, SLOT( slotChannelDoubleClicked( const QString & ) ) );

	new QHBoxLayout( m_list, 0, spacingHint() );

	setInitialSize( QSize( 500, 400 ) );
	setMainWidget( m_list );
	show();
}

void ChannelListDialog::clear()
{
	m_list->clear();
}

void ChannelListDialog::search()
{
	m_list->search();
}

void ChannelListDialog::slotChannelDoubleClicked( const QString & )
{
	close();
}

const QString IRCAccount::CONFIG_CODECMIB = QString::fromLatin1("Codec");
const QString IRCAccount::CONFIG_NETWORKNAME = QString::fromLatin1("NetworkName");
const QString IRCAccount::CONFIG_NICKNAME = QString::fromLatin1("NickName");
const QString IRCAccount::CONFIG_USERNAME = QString::fromLatin1("UserName");

IRCAccount::IRCAccount(IRCProtocol *protocol, const QString &accountId, const QString &autoChan )
	: Kopete::Account(protocol, accountId), autoConnect( autoChan )
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

	QObject::connect(m_engine, SIGNAL(connectedToServer()),
		this, SLOT(slotConnectedToServer()));

	QObject::connect(m_engine, SIGNAL(connectionTimeout()),
		this, SLOT(connect()));

	QObject::connect(m_engine, SIGNAL(successfulQuit()),
		this, SLOT(slotDisconnected()));

	QObject::connect(m_engine, SIGNAL(disconnected()),
		this, SLOT(slotDisconnected()));

	QObject::connect(m_engine, SIGNAL(incomingServerLoadTooHigh()),
		this, SLOT(slotServerBusy()));

	mAwayAction = new Kopete::AwayAction ( i18n("Set Away"),
		m_protocol->m_UserStatusAway.iconFor( this ), 0, this,
		SLOT(slotGoAway( const QString & )), this );

	currentHost = 0;
}

IRCAccount::~IRCAccount()
{
	if (m_engine->isConnected())
		m_engine->quit(i18n("Plugin Unloaded"), true);
}

void IRCAccount::loaded()
{
	KConfigGroup *config = configGroup();
	QString networkName = config->readEntry(CONFIG_NETWORKNAME);
	mNickName = config->readEntry(CONFIG_NICKNAME);
	QString codecMib = config->readEntry(CONFIG_CODECMIB);
	//	int codecMib = config->readNumEntry(CONFIG_CODECMIB, UTF-8);

	if( !codecMib.isEmpty() )
	{
		mCodec = QTextCodec::codecForMib( codecMib.toInt() );
		m_engine->setDefaultCodec( mCodec );
	}
	else
		mCodec = 0;

	QString m_accountId = accountId();
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
			if( rememberPassword() )
				host->password = password();
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

	m_contactManager = new IRCContactManager(mNickName, this);
	setMyself( m_contactManager->mySelf() );
	m_myServer = m_contactManager->myServer();
}

void IRCAccount::slotNickInUse( const QString &nick )
{
	QString altNickName = altNick();
	if( triedAltNick || altNickName.isEmpty() )
	{
		QString newNick = KLineEditDlg::getText(i18n( "IRC Plugin"),
			i18n("The nickname %1 is already in use. Please enter an alternate nickname:").arg(nick), nick);

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

void IRCAccount::setNetwork( const QString &network )
{
	IRCNetwork *net = m_protocol->networks()[ network ];
	if( net )
	{
		m_network = net;
		configGroup()->writeEntry(CONFIG_NETWORKNAME, network);
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

	KConfigGroup *config = configGroup();
	config->writeEntry( "CustomCtcp", val );
	config->sync();
}

const QMap< QString, QString > IRCAccount::customCtcpReplies() const
{
	QMap< QString, QString > replies;
	QStringList replyList;

	KConfigGroup *config = configGroup() ;
	replyList = config->readListEntry( "CustomCtcp" );

	for( QStringList::Iterator it = replyList.begin(); it != replyList.end(); ++it )
		replies[ (*it).section('=', 0, 0 ) ] = (*it).section('=', 1 );

	return replies;
}

void IRCAccount::setConnectCommands( const QStringList &commands ) const
{
	KConfigGroup *config = configGroup() ;
	config->writeEntry( "ConnectCommands", commands );
	config->sync();
}

const QStringList IRCAccount::connectCommands() const
{
	KConfigGroup *config =  configGroup() ;
	return config->readListEntry( "ConnectCommands" );
}

KActionMenu *IRCAccount::actionMenu()
{
	QString menuTitle = QString::fromLatin1( " %1 <%2> " ).arg( accountId() ).arg( myself()->onlineStatus().description() );

	KActionMenu *mActionMenu = new KActionMenu( accountId(),myself()->onlineStatus().iconFor(this), this, "IRCAccount::mActionMenu" );
	mActionMenu->popupMenu()->insertTitle( myself()->onlineStatus().iconFor( myself() ), menuTitle );

	mActionMenu->insert( new KAction ( i18n("Go Online"), m_protocol->m_UserStatusOnline.iconFor( this ), 0, this, SLOT(connect()), mActionMenu ) );
	mActionMenu->insert( mAwayAction );
	mActionMenu->insert( new KAction ( i18n("Go Offline"), m_protocol->m_UserStatusOffline.iconFor( this ), 0, this, SLOT(disconnect()), mActionMenu ) );
	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert( new KAction ( i18n("Join Channel..."), "", 0, this, SLOT(slotJoinChannel()), mActionMenu ) );
	mActionMenu->insert( new KAction ( i18n("Search Channels..."), "", 0, this, SLOT(slotSearchChannels()), mActionMenu ) );
	mActionMenu->insert( new KAction ( i18n("Show Server Window"), "", 0, this, SLOT(slotShowServerWindow()), mActionMenu ) );

	if( m_engine->isConnected() && m_engine->useSSL() )
	{
		mActionMenu->insert( new KAction ( i18n("Show Security Information"), "", 0, m_engine,
			SLOT(showInfoDialog()), mActionMenu ) );
	}

	return mActionMenu;
}

void IRCAccount::connect()
{
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

				if( currentHost == hosts.count() )
					currentHost = 0;

				IRCHost *host = hosts[ currentHost++ ];
				kdDebug( 0 ) << k_funcinfo << "connecting to " << host->host << ", SSL= " << host->ssl << endl;
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
		// Do nothing.
		break;
	case KIRC::Engine::Authentifying:
		break;
	case KIRC::Engine::Connected:
		{
			m_contactManager->addToNotifyList( m_engine->nickName() );

			Kopete::MessageManager *manager = myServer()->manager();
			if (!autoConnect.isEmpty())
				Kopete::CommandHandler::commandHandler()->processMessage( QString::fromLatin1("/join %1").arg(autoConnect), manager);

			QStringList commands(connectCommands());
			for (QStringList::Iterator it=commands.begin(); it != commands.end(); ++it)
				Kopete::CommandHandler::commandHandler()->processMessage(*it, manager);
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
		break;
	case KIRC::Engine::Disconnected:
		break;
	}
}
/*
void IRCAccount::slotConnectedToServer()
{
	kdDebug(14120) << k_funcinfo << autoConnect << endl;

	m_contactManager->addToNotifyList( m_engine->nickName() );

	Kopete::MessageManager *manager = myServer()->manager();
	if( !autoConnect.isEmpty() )
		Kopete::CommandHandler::commandHandler()->processMessage( QString::fromLatin1("/join %1").arg(autoConnect), manager );

	QStringList m_connectCommands = connectCommands();
	for( QStringList::Iterator it = m_connectCommands.begin(); it != m_connectCommands.end(); ++it )
		Kopete::CommandHandler::commandHandler()->processMessage( *it, manager );
}
*/
void IRCAccount::slotJoinedUnknownChannel( const QString &channel, const QString &nick )
{
	if ( nick.lower() == m_contactManager->mySelf()->nickName().lower() )
	{
		m_contactManager->findChannel( channel )->join();
	}
}
/*
void IRCAccount::slotDisconnected()
{
	triedAltNick = false;
	mySelf()->setOnlineStatus( m_protocol->m_UserStatusOffline );
	m_contactManager->removeFromNotifyList( m_engine->nickName() );

//	if (m_contactManager && !autoConnect.isNull())
//		Kopete::AccountManager::self()->removeAccount( this );
}
*/
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
	QString servPass = Kopete::Account::password();
	m_engine->setPassword(servPass);
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
	return ( myself()->onlineStatus().status() != Kopete::OnlineStatus::Offline );
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

	if ( contactId.startsWith( QString::fromLatin1("#") ) )
		c = static_cast<IRCContact*>( contactManager()->findChannel(contactId, m) );
	else
	{
		m_contactManager->addToNotifyList( contactId );
		c = static_cast<IRCContact*>( contactManager()->findUser(contactId, m) );
	}

	if( c->metaContact() != m )
	{//This should NEVER happen
		Kopete::MetaContact *old = c->metaContact();
		c->setMetaContact( m );
		Kopete::ContactPtrList children = old->contacts();
		if( children.isEmpty() )
			Kopete::ContactList::self()->removeMetaContact( old );
	}
	else if( c->metaContact()->isTemporary() )
		m->setTemporary(false);

	return true;
}

void IRCAccount::slotJoinNamedChannel( const QString &chan )
{
	contactManager()->findChannel( chan )->startChat();
}

void IRCAccount::slotJoinChannel()
{
	if(!isConnected())
		return;

	KConfig *config = kapp->config();
	config->setGroup( QString::fromLatin1("Account_IRCProtocol_") + accountId() );
	QStringList chans = config->readListEntry( "Recent Channel list" );

	KLineEditDlg dlg(
		i18n( "Please enter name of the channel you want to join:" ),
		QString::null,
		Kopete::UI::Global::mainWidget()
	);

	if( !chans.isEmpty() )
	{
		dlg.lineEdit()->setCompletedItems( chans );
		dlg.lineEdit()->setCompletionMode( KGlobalSettings::CompletionPopupAuto );
	}

	if( dlg.exec() == QDialog::Accepted )
	{
		QString chan = dlg.text();
		chans = dlg.lineEdit()->completionBox()->items();
		chans.append( chan );

		if( !chan.isNull() )
		{
			if( KIRC::Entity::isChannel( chan ) )
				contactManager()->findChannel( chan )->startChat();
			else
				KMessageBox::error( Kopete::UI::Global::mainWidget(),
					i18n("\"%1\" is an invalid channel. Channels must start with '#', '!', '+', or '&'.")
					.arg(chan), i18n("IRC Plugin")
				);
		}

		if( !chans.isEmpty() )
		{
			config->writeEntry( "Recent Channel list", chans );
			config->sync();
		}
	}
}

void IRCAccount::slotNewCtcpReply(const QString &type, const QString &target, const QString &messageReceived)
{
	appendMessage( i18n("CTCP %1 REPLY: %2").arg(type).arg(messageReceived), InfoReply );
}

void IRCAccount::appendMessage( const QString &message, MessageType type )
{
	MessageDestination destination;
	//if( !manager )
	//{
		//No manager was passed. Use current active manager
		destination = ActiveWindow;
	//}
	/*else
	{
		//FIXME: Implement this!
		destination = type;
	} */

	if( destination & ActiveWindow )
	{
		KopeteView *activeView = Kopete::MessageManagerFactory::self()->activeView();
		if( activeView && activeView->msgManager()->account() == this )
		{
			Kopete::MessageManager *manager = activeView->msgManager();
			Kopete::Message msg( manager->user(), manager->members(), message,
				Kopete::Message::Internal, Kopete::Message::RichText, Kopete::Message::Chat );
			activeView->appendMessage(msg);
		}
	}

	if( destination & AnonymousWindow )
	{
		//TODO: Create an anonymous window??? What will this mean...
	}

	if( destination & ServerWindow )
	{
		myServer()->appendMessage(message);
	}

	if( destination & KNotify )
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

#include "ircaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

