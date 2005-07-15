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

#include "ircconst.h"
#include "irccontact.h"
#include "ircprotocol.h"

#include "kircengine.h"

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
#include <klineedit.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kpopupmenu.h>

#include <qtextcodec.h>

using namespace IRC;
using namespace Kopete;

class IRCAccountPrivate
{
public:
	IRCAccountPrivate()
		: manager(0), engine(0),
		  server(0), self(0),
		  commandSource(0),
		  awayAction(0), joinChannelAction(0), searchChannelAction(0)
	{ }

	Kopete::ChatSession *manager;
	QString autoConnect;

	KIRC::Engine *engine;
	IRCNetwork network;
	uint currentHost;

	QValueList<IRCContact *> contacts;
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
	: PasswordedAccount(IRCProtocol::self(), accountId, 0, true),
	  d( new IRCAccountPrivate )
{
	d->engine = new KIRC::Engine(this);
	d->autoConnect = autoChan;
	d->currentHost = 0;

	QObject::connect(d->engine, SIGNAL(connectionStateChanged(KIRC::ConnectionState)),
			 this, SLOT(engineConnectionStateChanged(KIRC::ConnectionState)));

	QObject::connect(d->engine, SIGNAL(receivedMessage(KIRC::MessageType, const KIRC::EntityPtr &, const KIRC::EntityPtrList &, const QString &)),
			 this, SLOT(receivedMessage(KIRC::MessageType, const KIRC::EntityPtr &, const KIRC::EntityPtrList &, const QString &)));

//	loadProperties();

	d->server = new IRCContact(this, d->engine->server());
	d->self = new IRCContact(this, d->engine->self());
	setMyself(d->self);
/*
	QString accountId = this->accountId();
	if (networkName.isEmpty() && QRegExp( "[^#+&\\s]+@[\\w-\\.]+:\\d+" ).exactMatch(accountId))
	{
		kdDebug(14120) << "Creating account from " << accountId << endl;

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
		kdError() << "No network name defined, and could not import network information from ID" << endl;
	}
*/

//	setAccountLabel( QString::fromLatin1("%1@%2").arg(mNickName,networkName) );

	#warning spurus slot calls for now
	d->joinChannelAction = new KAction ( i18n("Join Channel..."), QString::null, 0, this,
				SLOT(slotJoinChannel()), this);
	d->searchChannelAction = new KAction ( i18n("Search Channels..."), QString::null, 0, this,
				SLOT(slotSearchChannels()), this);
}

IRCAccount::~IRCAccount()
{
	if (d->engine->isConnected())
		d->engine->quit(i18n("Plugin Unloaded"), true);

	delete d;
}


void IRCAccount::setupEngine()
{
	d->engine->setDefaultCodec(codec());

	d->engine->setUserName(userName());
	d->engine->setRealName(realName());

//	d->engine->setNickName(nickName());

	d->engine->setVersionString(IRC::Version);
/*
	QMap<QString, QString> replies = customCtcpReplies();
	for (QMap<QString, QString>::ConstIterator it = replies.begin(); it != replies.end(); ++it)
		d->engine->addCustomCtcp(it.key(), it.data());
*/
}

int IRCAccount::codecMib() const
{
	return configGroup()->readNumEntry(Config::CODECMIB);
}

void IRCAccount::setCodecFromMib(int mib)
{
	configGroup()->writeEntry(Config::CODECMIB, mib);
	d->engine->setDefaultCodec(QTextCodec::codecForMib(mib));
}

QTextCodec *IRCAccount::codec() const
{
	return QTextCodec::codecForMib(codecMib());
}

void IRCAccount::setCodec( QTextCodec *codec )
{
	if (codec)
		setCodecFromMib(codec->mibEnum());
	else
		setCodecFromMib(-1); // MIBenum are >= 0  so we inforce an error value
}

const QString IRCAccount::networkName() const
{
	return configGroup()->readEntry(Config::NETWORKNAME);
}

void IRCAccount::setNetworkByName(const QString &networkName)
{
	configGroup()->writeEntry(Config::NETWORKNAME, networkName);
//	setAccountLabel(network.name);
}
/*
const IRCNetwork &network() const
{
	return d->network;
}

void IRCAccount::setNetwork(const IRCNetwork &network)
{
//	configGroup()->writeEntry(Config::NETWORKNAME, network.name);
	d->network = network;
//	setAccountLabel(network.name);
}
*/

const QString IRCAccount::userName() const
{
	return configGroup()->readEntry(Config::USERNAME);
}

void IRCAccount::setUserName(const QString &userName)
{
	configGroup()->writeEntry(Config::USERNAME, userName);
	d->engine->setUserName(userName);
}

const QString IRCAccount::realName() const
{
	return configGroup()->readEntry(Config::REALNAME);
}

void IRCAccount::setRealName( const QString &userName )
{
	configGroup()->writeEntry(Config::REALNAME, userName);
	d->engine->setRealName(userName);
}

const QString IRCAccount::nickName() const
{
	return configGroup()->readEntry(Config::NICKNAME);
}

void IRCAccount::setNickName(const QString &nickName)
{
	configGroup()->writeEntry(Config::NICKNAME, nickName);
//	d->self->setNickName(nickName);
}
/*
const QString IRCAccount::altNick() const
{
	return configGroup()->readEntry(QString::fromLatin1("altNick"));
}

void IRCAccount::setAltNick( const QString &altNick )
{
	configGroup()->writeEntry(QString::fromLatin1( "altNick" ), altNick);
}
*/
const QString IRCAccount::defaultPartMessage() const
{
	QString partMsg = configGroup()->readEntry(QString::fromLatin1("defaultPart"));
	if( partMsg.isEmpty() )
		return IRC::Version;
	return partMsg;
}

void IRCAccount::setDefaultPartMessage( const QString &defaultPart )
{
	configGroup()->writeEntry( QString::fromLatin1( "defaultPart" ), defaultPart );
}

const QString IRCAccount::defaultQuitMessage() const
{
	QString quitMsg = configGroup()->readEntry(QString::fromLatin1("defaultQuit"));
	if( quitMsg.isEmpty() )
		return IRC::Version;
	return quitMsg;
}

void IRCAccount::setDefaultQuitMessage( const QString &defaultQuit )
{
	configGroup()->writeEntry( QString::fromLatin1( "defaultQuit" ), defaultQuit );
}

bool IRCAccount::autoShowServerWindow() const
{
	return configGroup()->readBoolEntry(QString::fromLatin1("AutoShowServerWindow"));
}

void IRCAccount::setAutoShowServerWindow(bool autoShow)
{
	configGroup()->writeEntry(QString::fromLatin1("AutoShowServerWindow"), autoShow);
}

KIRC::Engine *IRCAccount::engine() const
{
	return d->engine;
}

void IRCAccount::setCustomCtcpReplies( const QMap< QString, QString > &replies ) const
{
	QStringList val;
	for( QMap< QString, QString >::ConstIterator it = replies.begin(); it != replies.end(); ++it )
	{
		d->engine->addCustomCtcp( it.key(), it.data() );
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

KActionMenu *IRCAccount::actionMenu()
{
	QString menuTitle = QString::fromLatin1( " %1 <%2> " ).arg( accountId() ).arg( myself()->onlineStatus().description() );

	KActionMenu *mActionMenu = Account::actionMenu();

	d->joinChannelAction->setEnabled( isConnected() );
	d->searchChannelAction->setEnabled( isConnected() );

	mActionMenu->popupMenu()->insertSeparator();
	mActionMenu->insert(d->joinChannelAction);
	mActionMenu->insert(d->searchChannelAction);
	mActionMenu->insert( new KAction ( i18n("Show Server Window"), QString::null, 0, this, SLOT(slotShowServerWindow()), mActionMenu ) );

//	if (d->engine->isConnected() && d->engine->useSSL())
	{
		mActionMenu->insert( new KAction ( i18n("Show Security Information"), "", 0, d->engine,
			SLOT(showInfoDialog()), mActionMenu ) );
	}

	return mActionMenu;
}

void IRCAccount::connectWithPassword(const QString &password)
{
	//TODO:  honor the initial status

	if ( d->engine->isConnected() )
	{
		if( isAway() )
			setAway( false );
	}
	else if ( d->engine->isDisconnected() )
	{
//		if( d->network )
		{
/*
			QValueList<IRCHost*> &hosts = d->network->hosts;
			if( hosts.count() == 0 )
			{
				KMessageBox::queuedMessageBox(
					UI::Global::mainWidget(), KMessageBox::Error,
					i18n("<qt>The network associated with this account, <b>%1</b>, has no valid hosts. Please ensure that the account has a valid network.</qt>").arg(d->network->name),
					i18n("Network is Empty"), 0 );
			}
			else if( currentHost == hosts.count() )
			{
			    KMessageBox::queuedMessageBox(
				    UI::Global::mainWidget(), KMessageBox::Error,
			    i18n("<qt>Kopete could not connect to any of the servers in the network associated with this account (<b>%1</b>). Please try again later.</qt>").arg(d->network->name),
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
				appendInternalMessage( i18n("Connecting to %1...").arg( host->host ) );
				if( host->ssl )
					appendInternalMessage( i18n("Using SSL") );

				d->engine->setPassword(password);
//				d->engine->connectToServer( host->host, host->port, mNickName, host->ssl );
			}
*/
		}
//		else
//		{
//			kdWarning() << "No network defined!" << endl;
//		}
	}
}

void IRCAccount::engineConnectionStateChanged(KIRC::ConnectionState newstate)
{
	kdDebug(14120) << k_funcinfo << endl;

	mySelf()->updateStatus();

	switch (newstate)
	{
	case KIRC::Idle:
		// Do nothing.
		break;
	case KIRC::Connecting:
	{
		// d->expectedOnlineStatus check and use it
		if (autoShowServerWindow())
			myServer()->startChat();
		break;
	}
	case KIRC::Authentifying:
		break;
	case KIRC::Connected:
		{
			//Reset the host so re-connection will start over at first server
			d->currentHost = 0;
//			d->contactManager->addToNotifyList( d->engine->nickName() );

			// HACK! See bug #85200 for details. Some servers still cannot accept commands
			// after the 001 is sent, you need to wait until all the init junk is done.
			// Unfortunatly, there is no way for us to know when it is done (it could be
			// spewing out any number of replies), so just try delaying it
//			QTimer::singleShot( 250, this, SLOT( slotPerformOnConnectCommands() ) );
		}
		break;
	case KIRC::Closing:
//		mySelf()->setOnlineStatus( protocol->m_UserStatusOffline );
//		d->contactManager->removeFromNotifyList( d->engine->nickName() );

//		if (d->contactManager && !autoConnect.isNull())
//			AccountManager::self()->removeAccount( this );
		break;
//	case KIRC::Timeout:
		//Try next server
//		connect();
//		break;
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
	ChatSession *manager = myServer()->manager(Contact::CanCreate);
	if (!manager)
		return;

	if (!d->autoConnect.isEmpty())
		CommandHandler::commandHandler()->processMessage( QString::fromLatin1("/join %1").arg(d->autoConnect), manager);

	QStringList commands(connectCommands());
	for (QStringList::Iterator it=commands.begin(); it != commands.end(); ++it)
		CommandHandler::commandHandler()->processMessage(*it, manager);
}

void IRCAccount::quit( const QString &quitMessage )
{
	kdDebug(14120) << "Quitting IRC: " << quitMessage << endl;

	if (quitMessage.isNull() || quitMessage.isEmpty())
		d->engine->quit( defaultQuitMessage() );
	else
		d->engine->quit( quitMessage );
}

void IRCAccount::setAway(bool isAway, const QString &awayMessage)
{
	kdDebug(14120) << k_funcinfo << isAway << " " << awayMessage << endl;
	if (d->engine->isConnected())
	{
//		static_cast<IRCUserContact *>( myself() )->setAway( isAway );
		engine()->away(isAway, awayMessage);
	}
}

void IRCAccount::slotShowServerWindow()
{
	d->server->startChat();
}

bool IRCAccount::isConnected()
{
	return d->engine->isConnected();
}

void IRCAccount::setOnlineStatus(const OnlineStatus& status , const QString &reason)
{
	kdDebug(14120) << k_funcinfo << endl;
	d->expectedOnlineStatus = status;
	d->expectedReason = reason;

	OnlineStatus::StatusType current = myself()->onlineStatus().status();
	OnlineStatus::StatusType expected = d->expectedOnlineStatus.status();

	if ( expected != OnlineStatus::Offline && current == OnlineStatus::Offline )
	{
		setupEngine();
		connect();
	}

	if ( expected == OnlineStatus::Offline && current != OnlineStatus::Offline )
		quit(reason);
}

bool IRCAccount::createContact(const QString &contactId, MetaContact *metac)
{
/*	if (contactId == mNickName)
	{
		KMessageBox::error( UI::Global::mainWidget(),
			i18n("\"You are not allowed to add yourself to your contact list."), i18n("IRC Plugin")
		);

		return false;
	}*/
	IRCContact *contact = getContact(contactId, metac);
/*
	if (contact->metaContact() != metac )
	{//This should NEVER happen
		MetaContact *old = contact->metaContact();
		contact->setMetaContact(metac);
		ContactPtrList children = old->contacts();
		if (children.isEmpty())
			ContactList::self()->removeMetaContact( old );
	}
	else */ if (contact->metaContact()->isTemporary())
		metac->setTemporary(false);

	return true;
}

void IRCAccount::setCurrentCommandSource( ChatSession *session )
{
	d->commandSource = session;
}

ChatSession *IRCAccount::currentCommandSource()
{
	return d->commandSource;
}

void IRCAccount::appendErrorMessage(const QString &message)
{
	#warning FIXME make a reall message.
	appendInternalMessage(message);
}

void IRCAccount::appendInternalMessage(const QString &message)
{
	ContactPtrList members;
	members.append(d->server);
	Message msg(d->server, members, message, Message::Internal, Message::RichText, CHAT_VIEW);
	d->server->appendMessage(msg);
}

IRCContact *IRCAccount::myServer() const
{
	return d->server;
}

IRCContact *IRCAccount::mySelf() const
{
	return d->self;
}

IRCContact *IRCAccount::getContact(const QString &name, MetaContact *metac)
{
	kdDebug(14120) << k_funcinfo << name << endl;
	return getContact(d->engine->getEntity(name), metac);
}

IRCContact *IRCAccount::getContact(KIRC::EntityPtr entity, MetaContact *metac)
{
	IRCContact *contact = 0;

	#warning Do the search code here.

	if (!contact)
	{
		#warning Make a temporary meta contact if metac is null
		contact = new IRCContact(this, entity, metac);
		d->contacts.append(contact);
	}

	QObject::connect(contact, SIGNAL(destroyed(IRCContact *)), SLOT(destroyed(IRCContact *)));
	return contact;
}

void IRCAccount::destroyed(IRCContact *contact)
{
	d->contacts.remove(contact);
}

void IRCAccount::receivedMessage(
		KIRC::MessageType type,
		const KIRC::EntityPtr &from,
		const KIRC::EntityPtrList &to,
		const QString &message)
{
	IRCContact *fromContact = getContact(from);
/*
	Message msg(fromContact, manager()->members(), message, Kopete::Message::Inbound,
		    Kopete::Message::RichText, CHAT_VIEW);

		toContact->appendMessage(msg);
*/
}
/*
void IRCChannelContact::newAction(const QString &from, const QString &action)
{
	IRCAccount *account = ircAccount();

	IRCUserContact *f = account->contactManager()->findUser(from);
	Kopete::Message::MessageDirection dir =
		(f == account->mySelf()) ? Kopete::Message::Outbound : Kopete::Message::Inbound;
	Kopete::Message msg(f, manager()->members(), action, dir, Kopete::Message::RichText,
			    CHAT_VIEW, Kopete::Message::TypeAction);
	appendMessage(msg);
}

void IRCUserContact::newAction(const QString &to, const QString &action)
{
	IRCAccount *account = ircAccount();

	IRCContact *t = account->contactManager()->findUser(to);

	Kopete::Message::MessageDirection dir =
		(this == account->mySelf()) ? Kopete::Message::Outbound : Kopete::Message::Inbound;
	Kopete::Message msg(this, t, action, dir, Kopete::Message::RichText,
			CHAT_VIEW, Kopete::Message::TypeAction);

	//Either this is from me to a guy, or from a guy to me. Either way its a PM
	if (dir == Kopete::Message::Outbound)
		t->appendMessage(msg);
	else
		appendMessage(msg);
}

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
//	kdDebug(14120) << k_funcinfo << "Changing nick to " << newnick << endl;
//	mNickName = newnick;
//	mySelf()->setNickName( mNickName );
//	d->contactManager->removeFromNotifyList( oldnick );
//	d->contactManager->addToNotifyList( newnick );
}
*/
#include "ircaccount.moc"
