/*
 * messengeraccount.cpp - Windows Live Messenger Kopete Account.
 *
 * Copyright (c) 2007 by Zhang Panyong <pyzhang@gmail.com>
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#include "messengeraccount.h"

// KDE includes
#include <kaction.h>
#include <kactionmenu.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kcodecs.h>
#include <klocale.h>
#include <kicon.h>
#include <kconfiggroup.h>

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QValidator>
#include <QtGui/QImage>
#include <QtCore/QList>
#include <QtCore/QCryptographicHash>
#include <QMessageBox>
#include <QStringListModel>
#include <QListView>

#include "messengercontact.h"
// Kopete includes
#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopetepassword.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopetechatsessionmanager.h"

MessengerAccount::MessengerAccount(MessengerProtocol *protocol, const QString &accountId)
 : Kopete::PasswordedAccount(protocol, accountId.toLower(), false)
{
	

	m_openInboxAction = new KAction( KIcon("mail"), i18n( "Open Inbo&x..." ), this );
        //, "m_openInboxAction" );
	QObject::connect( m_openInboxAction, SIGNAL(triggered(bool)), this, SLOT(slotOpenInbox()) );

	m_changeDNAction = new KAction( KIcon("help-contents"),i18n( "&Change Display Name..." ), this );
        //, "renameAction" );
	QObject::connect( m_changeDNAction, SIGNAL(triggered(bool)), this, SLOT(slotChangePublicName()) );

	m_editUserInfoAction = new KAction( KIcon("help-contents"),i18n( "&Edit User Info" ), this );
        //, "editUserInfoAction" );
	QObject::connect( m_editUserInfoAction, SIGNAL(triggered(bool)), this, SLOT(slotUserInfo()) );

	m_startChatAction = new KAction( KIcon("mail"), i18n( "&Start Chat..." ), this );
        //, "startChatAction" );
	QObject::connect( m_startChatAction, SIGNAL(triggered(bool)), this, SLOT(slotStartChat()) );

// 	//create a MessengerClient
// 	m_messengerClient = new Papillon::Client(new Papillon::QtConnector(this),this);

// 	KConfigGroup *config= configGroup();
// 	m_blockList   = config->readEntry(  "blockList", QStringList() ) ;
// 	m_allowList   = config->readEntry(  "allowList", QStringList() ) ;
// 	m_reverseList = config->readEntry(  "reverseList", QStringList()  ) ;

	// getting list of contacts order by type
// 	Papillon::ContactList *cl = new Papillon::ContactList(m_messengerClient);
// 	m_contactList = cl->contacts();
// 	m_blockList = cl->blockList();
// 	m_allowList = cl->allowList();
// 	m_reverseList = cl->reverseList();

	MessengerContact *myContact = new MessengerContact ( this, accountId, Kopete::ContactList::self()->myself());
	setMyself( myContact );

	// Set the client Id for the myself contact.  It sets what Messenger feature we support.
	m_clientId = MessengerProtocol::MessengerC4 | MessengerProtocol::InkFormatGIF | MessengerProtocol::SupportMultiPacketMessaging;

	// starting with offline status
	m_initialPresence = Papillon::Presence::Offline;
}

MessengerAccount::~MessengerAccount()
{
	disconnect ( Kopete::Account::Manual );
  	//cleanup ();
}

void MessengerAccount::cleanup ()
{

	delete m_messengerClient;
	m_messengerClient = 0L;

	//TODO delete also contact list
// 	delete m_contactPool;
// 	m_contactPool = 0L;

}
bool MessengerAccount::removedAccount()
{
      //TODO
      return false;
}
QString MessengerAccount::serverName() const
{
	return configGroup()->readEntry(  "serverName" , MESSENGER_DEFAULT_SERVER );
}

int MessengerAccount::serverPort() const
{
	return configGroup()->readEntry(  "serverPort" , MESSENGER_DEFAULT_PORT );
}

bool MessengerAccount::useHttpMethod() const
{
	return configGroup()->readEntry(  "useHttpMethod" , false );
}

QString MessengerAccount::myselfClientId() const
{
	return QString::number(m_clientId, 10);
}

QString MessengerAccount::pictureUrl()
{
	return m_pictureFilename;
}

void MessengerAccount::setPictureUrl(const QString &url)
{
	m_pictureFilename = url;
}

QString MessengerAccount::pictureObject()
{
	if(m_pictureObj.isNull())
		resetPictureObject(true); //silent=true to keep infinite loop away
	return m_pictureObj;
}

void MessengerAccount::resetPictureObject(bool silent)
{
	QString old=m_pictureObj;

	if(!configGroup()->readEntry("exportCustomPicture", false))
	{
		m_pictureObj="";
		myself()->removeProperty( Kopete::Global::Properties::self()->photo() );
	}
	else
	{
		QFile pictFile( pictureUrl() );
		if (!pictFile.open(QIODevice::ReadOnly))
		{
			kDebug(14140) << "Could not open avatar picture.";

			m_pictureObj="";
			myself()->removeProperty( Kopete::Global::Properties::self()->photo() );
		}
		else
		{
			QByteArray ar = pictFile.readAll();

			QByteArray sha1d = QCryptographicHash::hash(ar, QCryptographicHash::Sha1).toBase64();

			QString size = QString::number( pictFile.size() );
			QString all = "Creator"+accountId()+"Size"+size+"Type3Locationkopete.tmpFriendlyAAA=SHA1D"+ sha1d;
			m_pictureObj="<msnobj Creator=\"" + accountId() + "\" Size=\"" + size  + "\" Type=\"3\" Location=\"kopete.tmp\" Friendly=\"AAA=\" SHA1D=\""+sha1d+"\" SHA1C=\""+
				QString( QCryptographicHash::hash(all.toUtf8(), QCryptographicHash::Sha1).toBase64() )  +"\"/>";
			myself()->setProperty( Kopete::Global::Properties::self()->photo(), pictureUrl() );
		}
	}
/*
	if( old != m_pictureObj && isConnected() && m_notifySocket && !silent )
	{
		kDebug(14140) << "Changing avatar(and status) on server";
		//update the msn pict
		m_notifySocket->setStatus( myself()->onlineStatus() );
	}*/
}


void MessengerAccount::fillActionMenu( KActionMenu *actionMenu )
{
	Kopete::Account::fillActionMenu( actionMenu );

	if(isConnected())
	{
		m_openInboxAction->setEnabled( true );
		m_startChatAction->setEnabled( true );
		m_changeDNAction->setEnabled( true );

	}
	else
	{
		m_openInboxAction->setEnabled( false );
		m_startChatAction->setEnabled( false );
		m_changeDNAction->setEnabled( false );
	}
	actionMenu->addSeparator();

	actionMenu->addAction( m_changeDNAction );
	actionMenu->addAction( m_startChatAction );
	actionMenu->addAction( m_openInboxAction );

#if defined MESSENGER_DEBUG
	KActionMenu *debugMenu = new KActionMenu( "Debug", this );

	KAction *rawCmd = new KAction( i18n( "Send Raw C&ommand..." ), this );
        //, "m_debugRawCommand" );
	QObject::connect( rawCmd, SIGNAL(triggered()), this, SLOT(slotDebugRawCommand()) );
	debugMenu->addAction(rawCmd);

	actionMenu->addSeparator();
	actionMenu->addAction( debugMenu );
#endif
}

void MessengerAccount::connectWithPassword(const QString &password)
{
	/* Cancel connection process if no password has been supplied or if password is null */
	if(password.isEmpty() or password.isNull())
	{
		kDebug() << " isEmpty or isNull password EXIT " << endl;
		disconnect ( Kopete::Account::Manual );
		return;
	}

	/* Don't do anything if we are already connected. */
	if ( isConnected () )
	{
		kDebug() << " is Connected EXIT " << endl;
		return;
	}

	// instantiate new client backend or clean up old one
	if ( !m_messengerClient )
	{
	      kDebug() << " ################################### creation de client " << endl;


	//create a MessengerClient
	m_messengerClient = new Papillon::Client(new Papillon::QtConnector(this), this);
	QObject::connect(m_messengerClient, SIGNAL(connectionStatusChanged(Papillon::Client::ConnectionStatus)), this, SLOT(clientConnectionStatusChanged(Papillon::Client::ConnectionStatus)));
	
     
	QObject::connect ( m_messengerClient, SIGNAL(connectionStatusChanged(Papillon::Client::Connecting)),this,SLOT ( slotConnected () ) );

	QObject::connect ( m_messengerClient, SIGNAL(connectionStatusChanged(Papillon::Client::LoggedIn)), this, SLOT( contactListLoaded() ));
 	}
	else
	{     
	      kDebug() << " deconnexion " << endl;
	      m_messengerClient->disconnectFromServer();
	}

	kDebug() << " connexion a etablir " << endl;

	m_password = password;
	QMessageBox::information(qobject_cast<QWidget *>(this), tr("Kopete Connexion"),
                   "Client ........ "+accountId());
	
	m_messengerClient->userContact()->setLoginInformation( accountId(), m_password );
	m_messengerClient->setServer("messenger.hotmail.com", 1863);
	m_messengerClient->connectToServer();
	m_messengerClient->contactList()->load();

	//contactListLoaded();
}

void MessengerAccount::disconnect()
{
	//kDebug(MESSENGER_DEBUG) << k_funcinfo << "attempt to set status offline"<<endl;
	m_messengerClient->disconnectFromServer();
}

void MessengerAccount::disconnect ( Kopete::Account::DisconnectReason reason )
{
	//kDebug (MESSENGER_DEBUG_GLOBAL) << "disconnect() called";

	if (isConnected ())
	{
		//kDebug (MESSENGER_DEBUG_GLOBAL) << "Still connected, closing connection...";
		disconnect();
	}

	// make sure that the connection animation gets stopped if we're still
	// in the process of connecting

	//TODO sauvegarde de l'etat presence a faire avec Status Presence de libpapillon
	//setPresence ( XMPP::Status ("", "", 0, false) );
	m_initialPresence = Papillon::Presence::Offline;
	
	//kDebug (MESSENGER_DEBUG_GLOBAL) << "Disconnected.";
	disconnected ( reason );
}
void MessengerAccount::slotConnected(){}

void MessengerAccount::setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason)
{
	
}

void MessengerAccount::clientConnectionStatusChanged(Papillon::Client::ConnectionStatus status)
{
	QString message;
	switch(status)
	{
		case Papillon::Client::Disconnected:
			qDebug() << "Disconnected from server....";
			message = "Disconnected from server....";
			break;
		case Papillon::Client::Connecting:
			qDebug() << "Connecting to Windows Live Messenger service...";
			message = "Connecting to Windows Live Messenger service...";
			break;
		case Papillon::Client::Connected:
			qDebug() << "Connected to Windows Live Messenger service, login in...";
			message = "Connected to Windows Live Messenger service, login in...";
			break;
		case Papillon::Client::LoggedIn:
			qDebug() << "Logged in.";
			message = "Logged in.";
			break;
		case Papillon::Client::LoginBadPassword:
			qDebug() << "Login got a bad password.";
			message = "Login got a bad password.";
			break;
	}
	QMessageBox::information(qobject_cast<QWidget *>(this), tr("Kopete Connection status"),
                   message);
}

void MessengerAccount::contactListLoaded()
{
	QStringList allowList, blockList, reverseList, pendingList;

	foreach(Papillon::Contact *contact, m_messengerClient->contactList()->allowList())
	{
		allowList << contact->contactId();
	}

	foreach(Papillon::Contact *contact, m_messengerClient->contactList()->blockList())
	{
		blockList << contact->contactId();
	}

	foreach(Papillon::Contact *contact, m_messengerClient->contactList()->reverseList())
	{
		reverseList << contact->contactId();
	}

	foreach(Papillon::Contact *contact, m_messengerClient->contactList()->pendingList())
	{
		pendingList << contact->contactId();
	}

	// Allow list
	QStringListModel *allowModel = new QStringListModel( allowList );
	QListView *allowView = new QListView(0);
	allowView->setModel(allowModel);
	allowView->setWindowTitle( QLatin1String("Allow List") );
	allowView->show();

	// Block list
	QStringListModel *blockModel = new QStringListModel( blockList );
	QListView *blockView = new QListView(0);
	blockView->setModel(blockModel);
	blockView->setWindowTitle( QLatin1String("Block List") );
	blockView->show();

	// Reverse list
	QStringListModel *reverseModel = new QStringListModel( reverseList );
	QListView *reserveView = new QListView(0);
	reserveView->setModel(reverseModel);
	reserveView->setWindowTitle( QLatin1String("Reverse List") );
	reserveView->show();

	// Pending list
	QStringListModel *pendingModel = new QStringListModel( pendingList );
	QListView *pendingView = new QListView(0);
	pendingView->setModel(pendingModel);
	pendingView->setWindowTitle( QLatin1String("Pending list") );
	pendingView->show();
}

void MessengerAccount::setStatusMessage(const Kopete::StatusMessage &statusMessage)
{
	myself()->setStatusMessage(statusMessage);
	//m_messengerClient->setStatusMessage(statusMessage);
}

bool MessengerAccount::createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact)
{
	if ( !parentMetaContact->isTemporary())// && m_notifySocket)
	{
		//m_addWizard_metaContact = parentMetaContact;

		// TODO: finish contact add
		//addContactServerside(contactId, metaContact->groups());
		// FIXME: Find out if this contact was really added or not!
		return true;
	}
	else
	{
		// This is a temporary contact.		
		MessengerContact *newContact = new MessengerContact( this, contactId, parentMetaContact );
		//TODO define a method to make deleted a contact
		//newContact->setDeleted(true);
		return true;
	}
return 0;
}

void MessengerAccount::slotUserInfo()
{
	//myself()->slotUserInfo();
}

void MessengerAccount::slotOpenInbox()
{

}
 
void MessengerAccount::setPublicName( const QString &publicName )
{
// 	if ( d->client )
// 	{
// 		d->client->changePublicName( publicName, QString() );
// 	}
}

void MessengerAccount::slotStartChat()
{
	bool ok;
	QString handle ;

	handle = KInputDialog::getText( i18n( "Start Chat - Messenger Plugin" ),
		i18n( "Please enter the email address of the person with whom you want to chat:" ), QString(), &ok ).toLower();
	if ( ok )
	{
// 		if ( MessengerProtocol::validContactId( handle ) )
// 		{
// 			if ( !contacts()[ handle ] )
// 				addContact( handle, handle, 0L, Kopete::Account::Temporary );
// 
// 			contacts()[ handle ]->execute();
// 		}
// 		else
// 		{
// 			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
// 				i18n( "<qt>You must enter a valid email address.</qt>" ), i18n( "Messenger Plugin" ) );
// 		}
	}
}

void MessengerAccount::slotChangePublicName()
{
// 	if ( !isConnected() )
// 	{
// 		return;
// 	}

	bool ok;
	QString name = KInputDialog::getText( i18n( "Change Display Name - Messenger Plugin" ),
			i18n( "Enter the new display name by which you want to be visible to your friends on Messenger:" ),
			myself()->property( Kopete::Global::Properties::self()->nickName()).value().toString(), &ok );

	if ( ok )
	{
		if ( name.length() > 387 )
		{
			KMessageBox::error( Kopete::UI::Global::mainWidget(),
					i18n( "<qt>The display name you entered is too long. Please use a shorter name.\n"
						"Your display name has <b>not</b> been changed.</qt>" ),
					i18n( "Change Display Name - Messenger Plugin" ) );
			return;
		}
		setPublicName( name );
	}
	//return d->client;
}

#include "messengeraccount.moc"
 
