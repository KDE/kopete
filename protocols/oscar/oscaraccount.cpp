/*
    oscaraccount.cpp  -  Oscar Account Class

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2002 by Chris TenHarmsel <tenharmsel@staticmethod.net>
    Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
    Copyright (c) 2008 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "oscaraccount.h"

#include "kopetepassword.h"
#include "kopeteprotocol.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopeteidentity.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include "kopetecontact.h"
#include "kopetechatsession.h"

#include <assert.h>

#include <qapplication.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <qimage.h>
#include <qfile.h>
#include <qdom.h>
#include <QHash>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QNetworkProxy>
#include <QtGui/QTextDocument> // Qt::escape


#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <knotification.h>
#include <kstandarddirs.h>
#include <kprotocolmanager.h>

#include "client.h"
#include "connection.h"
#include "oscartypeclasses.h"
#include "oscarmessage.h"
#include "oscarutils.h"
#include "oscarclientstream.h"
#include "contactmanager.h"
#include "oscarlistnonservercontacts.h"
#include "kopetetransfermanager.h"
#include "kopeteversion.h"
#include "oscarversionupdater.h"
#include "filetransferhandler.h"
#include "chatroomhandler.h"
#include "nscainfoevent.h"
#include "oscarpresence.h"
#include "oscarprotocol.h"
#include "oscarstatusmanager.h"
#include <kopetesockettimeoutwatcher.h>

using namespace Oscar;


class OscarAccountPrivate : public Client::CodecProvider
{
	// Backreference
	OscarAccount& account;
public:
	OscarAccountPrivate( OscarAccount& a ): account( a ) {}

	//The liboscar hook for the account
	Client* engine;

	quint32 ssiLastModTime;

	//contacts waiting on SSI add ack and their metacontact
	QMap<QString, Kopete::MetaContact*> addContactMap;

	//contacts waiting on their group to be added
	QMap<QString, QString> contactAddQueue;
	QMap<QString, QString> contactChangeQueue;
	QMap<uint, FileTransferHandler*> fileTransferHandlerMap;
	
    OscarListNonServerContacts* olnscDialog;
	
	unsigned int versionUpdaterStamp;
	bool versionAlreadyUpdated;
	
	bool buddyIconDirty;

	virtual QTextCodec* codecForContact( const QString& contactName ) const
	{
		return account.contactCodec( Oscar::normalize( contactName ) );
	}

	virtual QTextCodec* codecForAccount() const
	{
		return account.defaultCodec();
	}
};

OscarAccount::OscarAccount(Kopete::Protocol *parent, const QString &accountID, bool isICQ)
: Kopete::PasswordedAccount( parent, accountID, false )
{
	kDebug(OSCAR_GEN_DEBUG) << " accountID='" << accountID <<
		"', isICQ=" << isICQ << endl;

	d = new OscarAccountPrivate( *this );
	d->engine = new Client( this );
	QObject::connect( d->engine, SIGNAL(createClientStream(ClientStream**)), this, SLOT(createClientStream(ClientStream**)) );
	d->engine->setIsIcq( isICQ );
	// Set version capability
	// last 4 bytes determine version
	// first number, major version
	// second number,  minor version
	// third number, point version 100+
	// fourth number,  point version 0-99
	QByteArray kg( "Kopete ICQ     ", 16 );
	kg[12] = KOPETE_VERSION_MAJOR;
	kg[13] = KOPETE_VERSION_MINOR;
	kg[14] = KOPETE_VERSION_RELEASE / 100;
	kg[15] = KOPETE_VERSION_RELEASE % 100;
	d->engine->setVersionCap( kg );
	
	d->versionAlreadyUpdated = false;
	d->buddyIconDirty = false;
	d->versionUpdaterStamp = OscarVersionUpdater::self()->stamp();
	if ( isICQ )
		d->engine->setVersion( OscarVersionUpdater::self()->getICQVersion() );
	else
		d->engine->setVersion( OscarVersionUpdater::self()->getAIMVersion() );

	d->engine->setCodecProvider( d );
    d->olnscDialog = 0L;
    QObject::connect( d->engine, SIGNAL(loggedIn()), this, SLOT(loginActions()) );
	QObject::connect( d->engine, SIGNAL(messageReceived(Oscar::Message)),
	                  this, SLOT(messageReceived(Oscar::Message)) );
	QObject::connect( d->engine, SIGNAL(socketError(int,QString)),
	                  this, SLOT(slotSocketError(int,QString)) );
	QObject::connect( d->engine, SIGNAL(taskError(Oscar::SNAC,int,bool)),
	                  this, SLOT(slotTaskError(Oscar::SNAC,int,bool)) );
	QObject::connect( d->engine, SIGNAL(userStartedTyping(QString)),
	                  this, SLOT(userStartedTyping(QString)) );
	QObject::connect( d->engine, SIGNAL(userStoppedTyping(QString)),
	                  this, SLOT(userStoppedTyping(QString)) );
	QObject::connect( d->engine, SIGNAL(iconNeedsUploading()),
	                  this, SLOT(slotSendBuddyIcon()) );
	QObject::connect( d->engine, SIGNAL(incomingFileTransfer(FileTransferHandler*)),
	                  this, SLOT(incomingFileTransfer(FileTransferHandler*)) );
	QObject::connect( d->engine, SIGNAL(chatroomRequest(ChatRoomHandler*)),
	                  this, SLOT(chatroomRequest(ChatRoomHandler*)) );

	Kopete::TransferManager *tm = Kopete::TransferManager::transferManager();
	QObject::connect( tm, SIGNAL(refused(Kopete::FileTransferInfo)),
	                  this, SLOT(fileTransferRefused(Kopete::FileTransferInfo)) );
	QObject::connect( tm, SIGNAL(accepted(Kopete::Transfer*,QString)),
	                  this, SLOT(fileTransferAccept(Kopete::Transfer*,QString)) );
}

OscarAccount::~OscarAccount()
{
	OscarAccount::disconnect();
	delete d;
}

Client* OscarAccount::engine()
{
	return d->engine;
}

void OscarAccount::logOff( Kopete::Account::DisconnectReason reason )
{
	kDebug(OSCAR_GEN_DEBUG) << "accountId='" << accountId() << "'";
	//disconnect the signals
	Kopete::ContactList* kcl = Kopete::ContactList::self();
	QObject::disconnect( kcl, SIGNAL(groupRenamed(Kopete::Group*,QString)),
	                     this, SLOT(kopeteGroupRenamed(Kopete::Group*,QString)) );
	QObject::disconnect( kcl, SIGNAL(groupRemoved(Kopete::Group*)),
	                     this, SLOT(kopeteGroupRemoved(Kopete::Group*)) );
	QObject::disconnect( d->engine->ssiManager(), SIGNAL(contactAdded(OContact)),
	                     this, SLOT(ssiContactAdded(OContact)) );
	QObject::disconnect( d->engine->ssiManager(), SIGNAL(groupAdded(OContact)),
	                     this, SLOT(ssiGroupAdded(OContact)) );
	QObject::disconnect( d->engine->ssiManager(), SIGNAL(groupUpdated(OContact)),
	                     this, SLOT(ssiGroupUpdated(OContact)) );
	QObject::disconnect( d->engine->ssiManager(), SIGNAL(contactUpdated(OContact)),
	                     this, SLOT(ssiContactUpdated(OContact)) );

	d->engine->close();
	OscarProtocol* p = dynamic_cast<OscarProtocol*>(protocol());
	if ( myself() && p && p->statusManager() )
		myself()->setOnlineStatus( p->statusManager()->onlineStatusOf( Oscar::Presence( Oscar::Presence::Offline ) ) );

	d->contactAddQueue.clear();
	d->contactChangeQueue.clear();

	disconnected( reason );
}

void OscarAccount::disconnect()
{
	logOff( Kopete::Account::Manual );
}

bool OscarAccount::passwordWasWrong()
{
	return password().isWrong();
}

bool OscarAccount::setIdentity( Kopete::Identity *ident )
{
	if ( !Kopete::PasswordedAccount::setIdentity( ident ) )
		return false;

	QObject::connect( ident, SIGNAL(propertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)),
	                  this, SLOT(slotIdentityPropertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)) );
	
	QString photoPath = ident->property( Kopete::Global::Properties::self()->photo() ).value().toString();
	updateBuddyIcon( photoPath );
	return true;
}

void OscarAccount::loginActions()
{
    password().setWrong( false );
    kDebug(OSCAR_GEN_DEBUG) << "processing SSI list";
    processSSIList();

	//start a chat nav connection
	if ( !engine()->isIcq() )
	{
		kDebug(OSCAR_GEN_DEBUG) << "sending request for chat nav service";
		d->engine->requestServerRedirect( 0x000D );
	}

	kDebug(OSCAR_RAW_DEBUG) << "sending request for icon service";
	d->engine->connectToIconServer();

	if ( d->buddyIconDirty )
		updateBuddyIconInSSI();
}

void OscarAccount::processSSIList()
{
	//disconnect signals so we don't attempt to add things to SSI!
	Kopete::ContactList* kcl = Kopete::ContactList::self();
	QObject::disconnect( kcl, SIGNAL(groupRenamed(Kopete::Group*,QString)),
	                     this, SLOT(kopeteGroupRenamed(Kopete::Group*,QString)) );
	QObject::disconnect( kcl, SIGNAL(groupRemoved(Kopete::Group*)),
	                     this, SLOT(kopeteGroupRemoved(Kopete::Group*)) );

	kDebug(OSCAR_RAW_DEBUG) ;

	ContactManager* listManager = d->engine->ssiManager();

    //first add groups
	QList<OContact> groupList = listManager->groupList();
	QList<OContact>::const_iterator git = groupList.constBegin();
	QList<OContact>::const_iterator listEnd = groupList.constEnd();
	//the protocol dictates that there is at least one group that has contacts
	//so i don't have to check for an empty group list

	kDebug(OSCAR_GEN_DEBUG) << "Adding " << groupList.count() << " groups to contact list";
	for( ; git != listEnd; ++git )
	{ //add all the groups.
		if ( ( *git ).name() == "Buddies" ) continue;
		kDebug( OSCAR_GEN_DEBUG ) << "Adding SSI group'" << ( *git ).name()
			<< "' to the kopete contact list" << endl;
		kcl->findGroup( ( *git ).name() );
	}

	//then add contacts
	QList<OContact> contactList = listManager->contactList();
	QList<OContact>::const_iterator bit = contactList.constBegin();
	QList<OContact>::const_iterator blistEnd = contactList.constEnd();
	kDebug(OSCAR_GEN_DEBUG) << "Adding " << contactList.count() << " contacts to contact list";
	for ( ; bit != blistEnd; ++bit )
	{
		OContact groupForAdd = listManager->findGroup( ( *bit ).gid() );
		Kopete::Group* group;
		if ( groupForAdd.isValid() && groupForAdd.name() != "Buddies" )
			group = kcl->findGroup( groupForAdd.name() ); //add if not present
		else
			group = Kopete::Group::topLevel();

		kDebug( OSCAR_GEN_DEBUG ) << "Adding contact '" << ( *bit ).name() << "' to kopete list in group " <<
			group->displayName() << endl;
		OscarContact* oc = dynamic_cast<OscarContact*>( contacts().value( ( *bit ).name() ) );
		if ( oc )
		{
			OContact item = ( *bit );
			oc->setSSIItem( item );

			//only synchronizes group if metacontact is a member of
			//a single group
			if ( oc->metaContact()->groups().size() == 1 )
			{
				Kopete::Group* oldGrp = oc->metaContact()->groups().first();
				if ( oldGrp->displayName() != group->displayName() &&
				     oc->metaContact()->contacts().count() == 1 )
				{
					oc->metaContact()->moveToGroup( oldGrp, group );
				}
			}
		}
		else
			addContact( ( *bit ).name(), QString(), group, Kopete::Account::DontChangeKABC );
	}

	QObject::connect( kcl, SIGNAL(groupRenamed(Kopete::Group*,QString)),
	                  this, SLOT(kopeteGroupRenamed(Kopete::Group*,QString)) );
	QObject::connect( kcl, SIGNAL(groupRemoved(Kopete::Group*)),
	                  this, SLOT(kopeteGroupRemoved(Kopete::Group*)) );
	QObject::connect( listManager, SIGNAL(contactAdded(OContact)),
	                  this, SLOT(ssiContactAdded(OContact)) );
	QObject::connect( listManager, SIGNAL(groupAdded(OContact)),
	                  this, SLOT(ssiGroupAdded(OContact)) );
	QObject::connect( listManager, SIGNAL(groupUpdated(OContact)),
	                  this, SLOT(ssiGroupUpdated(OContact)) );
	QObject::connect( listManager, SIGNAL(contactUpdated(OContact)),
	                  this, SLOT(ssiContactUpdated(OContact)) );

	// TODO: Synchronize groups.
	// Currently groups that have been removed from the server do not get
	// removed from the client's list.  The problem is that a group can hold
	// contacts from other protocols.  Perhaps groups should store which
	// protocols are using it.  Asking the user for which account to create
	// a group, similar to how contact addition, could work.

    const QHash<QString, Kopete::Contact*> &nonServerContacts = contacts();
    QHash<QString, Kopete::Contact*>::ConstIterator it = nonServerContacts.constBegin();
    QStringList nonServerContactList;
    for ( ; it != nonServerContacts.constEnd(); ++it )
    {
        const OscarContact* oc = dynamic_cast<const OscarContact*>( ( *it ) );
        if ( !oc )
            continue;
        kDebug(OSCAR_GEN_DEBUG) << oc->contactId() << " contact ssi type: " << oc->ssiItem().type();
        if ( !oc->isOnServer() )
            nonServerContactList.append( ( *it )->contactId() );
    }
    kDebug(OSCAR_GEN_DEBUG) << "the following contacts are not on the server side list"
                             << nonServerContactList << endl;
	bool showMissingContactsDialog = configGroup()->readEntry(QString::fromLatin1("ShowMissingContactsDialog"), true);
    if ( !nonServerContactList.isEmpty() && showMissingContactsDialog )
    {
        d->olnscDialog = new OscarListNonServerContacts( Kopete::UI::Global::mainWidget() );
        QObject::connect( d->olnscDialog, SIGNAL(closing()),
                          this, SLOT(nonServerAddContactDialogClosed()) );
        d->olnscDialog->addContacts( nonServerContactList );
        d->olnscDialog->show();
    }
}

void OscarAccount::nonServerAddContactDialogClosed()
{
	if ( !d->olnscDialog )
		return;

	if ( d->olnscDialog->result() == KDialog::Yes )
	{
		NonServerContactsAddInfoEvent *event = new NonServerContactsAddInfoEvent( d->engine->ssiManager(), engine()->isIcq(), this );
		event->sendEvent();

		//start adding contacts
		kDebug(OSCAR_GEN_DEBUG) << "adding non server contacts to the contact list";
		//get the contact list. get the OscarContact object, then the group
		//check if the group is on ssi, if not, add it
		//if so, add the contact.
		QStringList offliners = d->olnscDialog->nonServerContactList();
		QStringList::iterator it, itEnd = offliners.end();
		for ( it = offliners.begin(); it != itEnd; ++it )
		{
			OscarContact* oc = dynamic_cast<OscarContact*>( contacts().value( ( *it ) ) );
			if ( !oc )
			{
				kDebug(OSCAR_GEN_DEBUG) << "no OscarContact object available for" << ( *it );
				continue;
			}

			Kopete::MetaContact* mc = oc->metaContact();
			if ( !mc )
			{
				kDebug(OSCAR_GEN_DEBUG) << "no metacontact object available for" << oc->contactId();
				continue;
			}

			Kopete::Group* group = mc->groups().first();
			if ( !group )
			{
				kDebug(OSCAR_GEN_DEBUG) << "no metacontact object available for" << oc->contactId();
				continue;
			}

			event->addContact( *it );
			addContactToSSI( ( *it ), group->displayName(), true );
		}
	}
	else if ( d->olnscDialog->result() == KDialog::No )
	{
		//remove contacts
		kDebug( OSCAR_GEN_DEBUG ) << "removing non server contacts from the "
			                         "contact list";
		Kopete::ContactList* kcl = Kopete::ContactList::self();
		QStringList offliners = d->olnscDialog->nonServerContactList();
		QStringList::iterator it, itEnd = offliners.end();
		for ( it = offliners.begin(); it != itEnd; ++it )
		{
			OscarContact* oc = dynamic_cast<OscarContact*>( contacts().value( (*it) ) );
			if ( !oc )
			{
				kDebug( OSCAR_GEN_DEBUG ) << "no OscarContact object available "
				                             "for" << ( *it ) << endl;
				continue;
			}

			Kopete::MetaContact* mc = oc->metaContact();
			if ( !mc )
			{
				kDebug( OSCAR_GEN_DEBUG ) << "no metacontact object available "
				                             "for" << ( oc->contactId() )
				                             << endl;
				continue;
			}

			if ( oc->metaContact()->contacts().count() <= 1 )
			{
				kcl->removeMetaContact( oc->metaContact() );
			}
			else
			{
				kDebug( OSCAR_GEN_DEBUG ) << oc->contactId() << " metacontact "
			                                 "contains multiple contacts.";
			}
		}
	}

	bool showOnce = d->olnscDialog->onlyShowOnce();
	configGroup()->writeEntry( QString::fromLatin1("ShowMissingContactsDialog") , !showOnce);
	configGroup()->sync();
	
    d->olnscDialog->deleteLater();
    d->olnscDialog = 0L;
}

void OscarAccount::chatroomRequest( ChatRoomHandler* handler )
{
	KGuiItem buttonYes( KStandardGuiItem::yes() );
	KGuiItem buttonNo( KStandardGuiItem::no() );
	buttonYes.setText( i18nc( "@action:button filter-yes", "%1", KStandardGuiItem::yes().text() ) );
	buttonNo.setText( i18nc( "@action:button filter-no", "%1", KStandardGuiItem::no().text() ) );
	i18nc( "@action:button post-filter", "." );

	KDialog *dialog = new KDialog( NULL, Qt::Dialog );
	dialog->setCaption( i18n( "Chat Room Invitation" ) );
	dialog->setButtons( KDialog::Yes | KDialog::No );
	dialog->setObjectName( "questionYesNoCancel" );
	dialog->setModal( false );
	dialog->showButtonSeparator( true );
	dialog->setButtonGuiItem( KDialog::Yes, buttonYes );
	dialog->setButtonGuiItem( KDialog::No, buttonNo );
	dialog->setDefaultButton( KDialog::Yes );
	dialog->setEscapeButton( KDialog::No );

	QObject::connect( dialog, SIGNAL(yesClicked()),
	                  handler, SLOT(accept()) );
	QObject::connect( dialog, SIGNAL(noClicked()),
	                  handler, SLOT(reject()) );
	QObject::connect( handler, SIGNAL(joinChatRoom(QString,int)),
	                  engine(), SLOT(joinChatRoom(QString,int)) );

	KMessageBox::createKMessageBox( dialog, QMessageBox::Question,
	                                ( handler->contact() + ": " + handler->invite() ), QStringList(),
	                                QString(), NULL, KMessageBox::NoExec );

	dialog->show();
	dialog->raise();
	dialog->activateWindow();
}

void OscarAccount::incomingFileTransfer( FileTransferHandler* ftHandler )
{
	QString sender = Oscar::normalize( ftHandler->contact() );
	if ( !contacts().value( sender ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Adding '" << sender << "' as temporary contact";
		addContact( sender, QString(), 0,  Kopete::Account::Temporary );
	}
	Kopete::Contact * ct = contacts().value( sender );

	// Fill the fileNameList with empty filenames if we have more files so the kopete transfer knows about them
	QStringList fileNameList;
	fileNameList << ftHandler->fileName();
	for ( int i = 1; i < ftHandler->fileCount(); i++ )
		fileNameList << "";

	Kopete::TransferManager* tm = Kopete::TransferManager::transferManager();
	uint ftId = tm->askIncomingTransfer( ct, fileNameList, ftHandler->totalSize(), ftHandler->description(),
	                                     ftHandler->internalId(), QPixmap() );
	QObject::connect( ftHandler, SIGNAL(destroyed(QObject*)), this, SLOT(fileTransferDestroyed(QObject*)) );
	QObject::connect( ftHandler, SIGNAL(transferCancelled()), this, SLOT(fileTransferCancelled()) );

	d->fileTransferHandlerMap.insert( ftId, ftHandler );
}

void OscarAccount::fileTransferDestroyed( QObject* object )
{
	uint key = d->fileTransferHandlerMap.key( (FileTransferHandler*)object, 0 );
	if ( key > 0 )
		d->fileTransferHandlerMap.remove( key );
	else
		kDebug(OSCAR_GEN_DEBUG) << "FileTransferHandler not in the map!!!";
}

void OscarAccount::fileTransferCancelled()
{
	FileTransferHandler* ftHandler = qobject_cast<FileTransferHandler*>(sender());
	if ( !ftHandler )
		return;

	uint key = d->fileTransferHandlerMap.key( ftHandler, 0 );
	if ( key == 0 )
	{
		kDebug(OSCAR_GEN_DEBUG) << "FileTransferHandler not in the map!!!";
		return;
	}

	QObject::disconnect( ftHandler, SIGNAL(transferCancelled()), this, SLOT(fileTransferCancelled()) );
	Kopete::TransferManager::transferManager()->cancelIncomingTransfer( key );
}

void OscarAccount::fileTransferRefused( const Kopete::FileTransferInfo& info )
{
	FileTransferHandler* ftHandler = d->fileTransferHandlerMap.value( info.transferId(), 0 );
	if ( !ftHandler )
		return;

	QObject::disconnect( ftHandler, SIGNAL(transferCancelled()), this, SLOT(fileTransferCancelled()) );
	ftHandler->cancel();
}

void OscarAccount::fileTransferAccept( Kopete::Transfer* transfer, const QString& fileName )
{
	FileTransferHandler* ftHandler = d->fileTransferHandlerMap.value( transfer->info().transferId(), 0 );
	if ( !ftHandler )
		return;

	QObject::disconnect( ftHandler, SIGNAL(transferCancelled()), this, SLOT(fileTransferCancelled()) );

	QObject::connect( transfer, SIGNAL(transferCanceled()), ftHandler, SLOT(cancel()) );
	QObject::connect( ftHandler, SIGNAL(transferCancelled()), transfer, SLOT(slotCancelled()) );
	QObject::connect( ftHandler, SIGNAL(transferError(int,QString)), transfer, SLOT(slotError(int,QString)) );
	QObject::connect( ftHandler, SIGNAL(transferProcessed(uint)), transfer, SLOT(slotProcessed(uint)) );
	QObject::connect( ftHandler, SIGNAL(transferFinished()), transfer, SLOT(slotComplete()) );
	QObject::connect( ftHandler, SIGNAL(transferNextFile(QString,QString)),
	                  transfer, SLOT(slotNextFile(QString,QString)) );

	if ( transfer->info().saveToDirectory() )
		ftHandler->save( fileName );
	else
		ftHandler->saveAs( QStringList() << fileName );
}

void OscarAccount::kopeteGroupRemoved( Kopete::Group* group )
{
	if ( isConnected() && group->displayName() != "Buddies" )
		d->engine->removeGroup( group->displayName() );
}

void OscarAccount::kopeteGroupAdded( Kopete::Group* group )
{
	if ( isConnected() )
		d->engine->addGroup( group->displayName() );
}

void OscarAccount::kopeteGroupRenamed( Kopete::Group* group, const QString& oldName )
{
	if ( isConnected() && oldName != "Buddies" )
		d->engine->renameGroup( oldName, group->displayName() );
}

void OscarAccount::messageReceived( const Oscar::Message& message )
{
	//the message isn't for us somehow
	if ( Oscar::normalize( message.receiver() ) != Oscar::normalize( accountId() ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "got a message but we're not the receiver: "
			<< message.textArray() << endl;
		return;
	}

	/* Logic behind this:
	 * If we don't have the contact yet, create it as a temporary
	 * Create the message manager
	 * Get the sanitized message back
	 * Append to the chat window
	 */
	QString sender = Oscar::normalize( message.sender() );
	if ( !contacts().value( sender ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "Adding '" << sender << "' as temporary contact";
		addContact( sender, QString(), 0,  Kopete::Account::Temporary );
	}

	OscarContact* ocSender = static_cast<OscarContact *> ( contacts().value( sender ) ); //should exist now

	if ( !ocSender )
	{
		kWarning(OSCAR_RAW_DEBUG) << "Temporary contact creation failed for '"
			<< sender << "'! Discarding message: " << message.textArray() << endl;
		return;
	}
	else
	{
		if ( message.hasProperty( Oscar::Message::WWP ) )
			ocSender->setNickName( i18n("ICQ Web Express") );
		if ( message.hasProperty( Oscar::Message::EMail ) )
			ocSender->setNickName( i18n("ICQ Email Express") );
	}

	Kopete::ChatSession* chatSession = ocSender->manager( Kopete::Contact::CanCreate );
	chatSession->receivedTypingMsg( ocSender, false ); //person is done typing


	//decode message
	QString realText( message.text( contactCodec( ocSender ) ) );

	//sanitize;
	QString sanitizedMsg = sanitizedMessage( realText );

	Kopete::ContactPtrList me;
	me.append( myself() );
	Kopete::Message chatMessage( ocSender, me );
	chatMessage.setHtmlBody( sanitizedMsg );
	chatMessage.setTimestamp( message.timestamp() );
	chatMessage.setDirection( Kopete::Message::Inbound );

	chatSession->appendMessage( chatMessage );
}

QString OscarAccount::sanitizedMessage( const QString& message ) const
{
	QDomDocument doc;
	QString domError;
	int errLine = 0, errCol = 0;

	QString msg = addQuotesAroundAttributes(message);
	msg = makeWellFormedXML( msg ); // Official AIM client send crap so we have to sort it out.
	msg.replace( "<BR>", "<BR/>", Qt::CaseInsensitive );

	doc.setContent( msg, false, &domError, &errLine, &errCol );
	if ( !domError.isEmpty() ) //error parsing, do nothing
	{
		kDebug(OSCAR_AIM_DEBUG) << "error from dom document conversion: " << domError << "line:" << errLine << "col:" << errCol;

		// HACK: for trillian which sends totaly mangled html (there are not ended font tags)
		if ( message.indexOf( QRegExp( "[\\s]*<[\\s]*HTML[\\s]*>[\\s]*<[\\s]*BODY", Qt::CaseInsensitive ) ) != 0 )
			return sanitizedPlainMessage( message );
		else
			return message;
	}
	else
	{
		kDebug(OSCAR_AIM_DEBUG) << "conversion to dom document successful."
			<< "looking for font tags" << endl;
		QList<QDomNode> fontTagList = getElementsByTagNameCI( doc, "FONT" );
		if ( fontTagList.count() == 0 )
		{
			if ( message.indexOf( QRegExp( "[\\s]*<[\\s]*HTML[\\s]*>[\\s]*<[\\s]*BODY", Qt::CaseInsensitive ) ) != 0 )
			{
				kDebug(OSCAR_AIM_DEBUG) << "No html tags found. Returning normal message";
				return sanitizedPlainMessage( message );
			}
		}
		else
		{
			kDebug(OSCAR_AIM_DEBUG) << "Found font tags. Attempting replacement";
			uint numFontTags = fontTagList.count();
			for ( uint i = 0; i < numFontTags; i++ )
			{
				QDomNode fontNode = fontTagList.at(i);
				QDomElement fontEl;
				if ( !fontNode.isNull() && fontNode.isElement() )
					fontEl = fontNode.toElement();
				else
					continue;
				if ( fontEl.hasAttribute( "BACK" ) )
				{
					QString backgroundColor = fontEl.attribute( "BACK" );
					backgroundColor.insert( 0, "background-color: " );
					backgroundColor.append( ';' );
					fontEl.setAttribute( "style", backgroundColor );
					fontEl.removeAttribute( "BACK" );
				}
			}
		}
	}
	kDebug(OSCAR_AIM_DEBUG) << "sanitized message is " << doc.toString();
	return doc.toString();
}

void OscarAccount::setServerAddress(const QString &server)
{
	configGroup()->writeEntry( QString::fromLatin1( "Server" ), server );
}

void OscarAccount::setServerPort(int port)
{
	if (port<=0)
		port=5190;

	configGroup()->writeEntry( QString::fromLatin1( "Port" ), port);
}

void OscarAccount::setServerEncrypted( bool encrypted )
{
	configGroup()->writeEntry( QString::fromLatin1( "Encrypted" ), encrypted);
}


void OscarAccount::setProxyServerSocks5( bool enable )
{
	configGroup()->writeEntry( QString::fromLatin1( "ProxySocks5" ), enable );
}

void OscarAccount::setProxyServerAddress(const QString &server)
{
	configGroup()->writeEntry( QString::fromLatin1( "ProxyServer" ), server );
}

void OscarAccount::setProxyServerPort(int port)
{
	configGroup()->writeEntry( QString::fromLatin1( "ProxyPort" ), port);
}

void OscarAccount::setProxyServerEnabled(bool enable)
{
	configGroup()->writeEntry( QString::fromLatin1( "ProxyEnable" ), enable);
}

QTextCodec* OscarAccount::defaultCodec() const
{
	QTextCodec* codec = QTextCodec::codecForMib( configGroup()->readEntry( "DefaultEncoding", 4 ) );

	if ( codec )
		return codec;
	else
		return QTextCodec::codecForMib( 4 );
}

QTextCodec* OscarAccount::contactCodec( const OscarContact* contact ) const
{
	if ( contact )
		return contact->contactCodec();
	else
		return defaultCodec();
}

QTextCodec* OscarAccount::contactCodec( const QString& contactName ) const
{
	// XXX  Need const_cast because Kopete::Account::contacts()
	// XXX  method is not const for some strange reason.
	OscarContact* contact = static_cast<OscarContact *> ( const_cast<OscarAccount *>(this)->contacts().value( contactName ) );
	return contactCodec( contact );
}

void OscarAccount::updateBuddyIcon( const QString &path )
{
	myself()->removeProperty( Kopete::Global::Properties::self()->photo() );

	if ( !path.isEmpty() )
	{
		QImage image( path );
		if ( image.isNull() )
			return;
		
		const QSize size = ( d->engine->isIcq() ) ? QSize( 52, 64 ) : QSize( 48, 48 );
		
		image = image.scaled( size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
		if( image.width() > size.width())
			image = image.copy( ( image.width() - size.width() ) / 2, 0, size.width(), image.height() );
		
		if( image.height() > size.height())
			image = image.copy( 0, ( image.height() - size.height() ) / 2, image.width(), size.height() );
		
		QString newlocation( KStandardDirs::locateLocal( "appdata", "oscarpictures/" + accountId() + ".jpg" ) );
		
		kDebug(OSCAR_RAW_DEBUG) << "Saving buddy icon: " << newlocation;
		if ( !image.save( newlocation, "JPEG" ) )
			return;
		
		myself()->setProperty( Kopete::Global::Properties::self()->photo() , newlocation );
	}
	
	d->buddyIconDirty = true;
	updateBuddyIconInSSI();
}

bool OscarAccount::addContactToSSI( const QString& contactName, const QString& groupName, bool autoAddGroup )
{
	ContactManager* listManager = d->engine->ssiManager();
	if ( !listManager->findGroup( groupName ) )
	{
		if ( !autoAddGroup )
			return false;

		kDebug(OSCAR_GEN_DEBUG) << "adding non-existent group "
			<< groupName << endl;

		d->contactAddQueue[Oscar::normalize( contactName )] = groupName;
		d->engine->addGroup( groupName );
	}
	else
	{
		d->engine->addContact( contactName, groupName );
	}

	return true;
}

bool OscarAccount::changeContactGroupInSSI( const QString& contact, const QString& newGroupName, bool autoAddGroup )
{
	ContactManager* listManager = d->engine->ssiManager();
	if ( !listManager->findGroup( newGroupName ) )
	{
		if ( !autoAddGroup )
			return false;
		
		kDebug(OSCAR_GEN_DEBUG) << "adding non-existent group " 
				<< newGroupName << endl;
			
		d->contactChangeQueue[Oscar::normalize( contact )] = newGroupName;
		d->engine->addGroup( newGroupName );
	}
	else
	{
		d->engine->changeContactGroup( contact, newGroupName );
	}
	
	return true;
}

bool OscarAccount::createContact(const QString &contactId,
	Kopete::MetaContact *parentContact)
{
	/* We're not even online or connecting
	 * (when getting server contacts), so don't bother
	 */
	if ( !engine()->isActive() )
	{
		kDebug(OSCAR_GEN_DEBUG) << "Can't add contact, we are offline!";
		return false;
	}

	/* Logic for SSI additions
	If the contact is temporary, no SSI addition at all. Just create the contact and be done with it
	If the contact is not temporary, we need to do the following:
		1. Check if contact already exists in the SSI manager, if so, just create the contact
		2. If contact doesn't exist:
		2.a. create group on SSI if needed
		2.b. create contact on SSI
		2.c. create kopete contact
	 */

	QList<TLV> dummyList;
	if ( parentContact->isTemporary() )
	{
		OContact tempItem( contactId, 0, 0, 0xFFFF, dummyList, 0 );
		return createNewContact( contactId, parentContact, tempItem );
	}

	OContact ssiItem = d->engine->ssiManager()->findContact( contactId );
	if ( ssiItem )
	{
		kDebug(OSCAR_GEN_DEBUG) << "Have new SSI entry. Finding contact";
		if ( contacts().value( ssiItem.name() ) )
		{
			kDebug(OSCAR_GEN_DEBUG) << "Found contact in list. Updating SSI item";
			OscarContact* oc = static_cast<OscarContact*>( contacts().value( ssiItem.name() ) );
			oc->setSSIItem( ssiItem );
			return true;
		}
		else
		{
			kDebug(OSCAR_GEN_DEBUG) << "Didn't find contact in list, creating new contact";
			return createNewContact( contactId, parentContact, ssiItem );
		}
	}
	else
	{ //new contact, check temporary, if temporary, don't add to SSI. otherwise, add.
		kDebug(OSCAR_GEN_DEBUG) << "New contact '" << contactId << "' not in SSI."
			<< " Creating new contact" << endl;

		kDebug(OSCAR_GEN_DEBUG) << "Adding " << contactId << " to server side list";

		QString groupName;
		Kopete::GroupList kopeteGroups = parentContact->groups(); //get the group list

		if ( kopeteGroups.isEmpty() || kopeteGroups.first() == Kopete::Group::topLevel() )
		{
			kDebug(OSCAR_GEN_DEBUG) << "Contact with NO group. " << "Adding to group 'Buddies'";
			groupName = "Buddies";
		}
		else
		{
				//apparently kopeteGroups.first() can be invalid. Attempt to prevent
				//crashes in SSIData::findGroup(const QString& name)
			groupName = kopeteGroups.first() ? kopeteGroups.first()->displayName() : "Buddies";

			kDebug(OSCAR_GEN_DEBUG) << "Contact with group." << " No. of groups = " << kopeteGroups.count() <<
				" Name of first group = " << groupName << endl;
		}

		if( groupName.isEmpty() )
		{ // emergency exit, should never occur
			kWarning(OSCAR_GEN_DEBUG) << "Could not add contact because no groupname was given";
			return false;
		}

		d->addContactMap[Oscar::normalize( contactId )] = parentContact;
		addContactToSSI( Oscar::normalize( contactId ), groupName, true );
		return true;
	}
}

void OscarAccount::updateVersionUpdaterStamp()
{
	d->versionUpdaterStamp = OscarVersionUpdater::self()->stamp();
}

void OscarAccount::ssiContactAdded( const OContact& item )
{
	QString normalizedName = Oscar::normalize( item.name() );
	if ( contacts().value( item.name() ) )
	{
		kDebug(OSCAR_GEN_DEBUG) << "Received confirmation from server. modifying " << item.name();
		OscarContact* oc = static_cast<OscarContact*>( contacts().value( item.name() ) );
		oc->setSSIItem( item );
		// To be safe remove contact from addContactMap
		d->addContactMap.remove( normalizedName );
	}
	else if ( d->addContactMap.contains( normalizedName ) )
	{
		kDebug(OSCAR_GEN_DEBUG) << "Received confirmation from server. adding " << item.name()
			<< " to the contact list" << endl;
		OscarContact* oc = createNewContact( item.name(), d->addContactMap[normalizedName], item );
		d->addContactMap.remove( normalizedName );
		if ( oc && oc->ssiItem().waitingAuth() )
			QTimer::singleShot( 1, oc, SLOT(requestAuthorization()) );
	}
	else
		kDebug(OSCAR_GEN_DEBUG) << "Got addition for contact we weren't waiting on";
}

void OscarAccount::ssiGroupAdded( const OContact& item )
{
	//check the contact add queue for any contacts matching the
	//group name we just added
	kDebug(OSCAR_GEN_DEBUG) << "Looking for contacts to be added in group " << item.name();
	QMap<QString,QString>::iterator it;
	for ( it = d->contactAddQueue.begin(); it != d->contactAddQueue.end(); ++it )
	{
		if ( Oscar::normalize( it.value() ) == Oscar::normalize( item.name() ) )
		{
			kDebug(OSCAR_GEN_DEBUG) << "starting delayed add of contact '" << it.key()
				<< "' to group " << item.name() << endl;
			
			d->engine->addContact( Oscar::normalize( it.key() ), item.name() );
			d->contactAddQueue.erase( it );
		}
	}
	
	for ( it = d->contactChangeQueue.begin(); it != d->contactChangeQueue.end(); ++it )
	{
		if ( Oscar::normalize( it.value() ) == Oscar::normalize( item.name() ) )
		{
			kDebug(OSCAR_GEN_DEBUG) << "starting delayed change of contact '" << it.key()
				<< "' to group " << item.name() << endl;
			
			d->engine->changeContactGroup( it.key(),  item.name() );
			d->contactChangeQueue.erase( it );
		}
	}
}

void OscarAccount::ssiContactUpdated( const OContact& item )
{
	Kopete::Contact* contact = contacts().value( item.name() );
	if ( !contact )
		return;
		
	kDebug(OSCAR_RAW_DEBUG) << "Updating SSI Item";
	OscarContact* oc = static_cast<OscarContact*>( contact );
	oc->setSSIItem( item );
}

void OscarAccount::userStartedTyping( const QString & contact )
{
	Kopete::Contact * ct = contacts().value( Oscar::normalize( contact ) );
	if ( ct )
	{
		OscarContact * oc = static_cast<OscarContact *>( ct );
		oc->startedTyping();
	}
}

void OscarAccount::userStoppedTyping( const QString & contact )
{
	Kopete::Contact * ct = contacts().value( Oscar::normalize( contact ) );
	if ( ct )
	{
		OscarContact * oc = static_cast<OscarContact *>( ct );
		oc->stoppedTyping();
	}
}

void OscarAccount::slotSocketError( int errCode, const QString& errString )
{
	Q_UNUSED( errCode );

	if ( !isBusy() )
		KNotification::event( QLatin1String("connection_error"), i18nc( "account has been disconnected", "Kopete: %1 disconnected", accountId() ),
	                      errString,
	                      myself()->onlineStatus().protocolIcon(KIconLoader::SizeMedium),
	                      Kopete::UI::Global::mainWidget() );

	logOff( Kopete::Account::ConnectionReset );
}

void OscarAccount::slotTaskError( const Oscar::SNAC& s, int code, bool fatal )
{
	kDebug(OSCAR_GEN_DEBUG) << "error received from task";
	kDebug(OSCAR_GEN_DEBUG) << "service: " << s.family
		<< " subtype: " << s.subtype << " code: " << code << endl;

	QString message;
	if ( s.family == 0 && s.subtype == 0 )
	{
		message = getFLAPErrorMessage( code );
		if ( !isBusy() )
			KNotification::event( QLatin1String("connection_error"), i18nc( "account has been disconnected", "Kopete: %1 disconnected", accountId() ),
		                      message, myself()->onlineStatus().protocolIcon(KIconLoader::SizeMedium),
		                      Kopete::UI::Global::mainWidget() );
		switch ( code )
		{
		case 0x0000:
			logOff( Kopete::Account::Unknown );
			break;
		case 0x0004:
		case 0x0005:
			logOff( Kopete::Account::BadPassword );
			break;
		case 0x0007:
		case 0x0008:
		case 0x0009:
		case 0x0011:
			logOff( Kopete::Account::BadUserName );
			break;
		case 0x001B:
		case 0x001C:
			OscarVersionUpdater::self()->update( d->versionUpdaterStamp );
			if ( !d->versionAlreadyUpdated )
			{
				logOff( Kopete::Account::Unknown );
				d->versionAlreadyUpdated = true;
			}
			else
			{
				logOff( Kopete::Account::Manual );
			}
			break;	
		default:
			logOff( Kopete::Account::Manual );
		}
		return;
	}
	if ( !fatal )
		message = i18n("There was an error in the protocol handling; it was not fatal, so you will not be disconnected.");
	else
		message = i18n("There was an error in the protocol handling; automatic reconnection occurring.");

	if ( !isBusy() )
		KNotification::event( QLatin1String("server_error"), i18n("Kopete: OSCAR Protocol error"), message, myself()->onlineStatus().protocolIcon(KIconLoader::SizeMedium),
	                      Kopete::UI::Global::mainWidget() );
	if ( fatal )
		logOff( Kopete::Account::ConnectionReset );
}

void OscarAccount::updateBuddyIconInSSI()
{
	if ( !engine()->isActive() )
		return;
	
	QString photoPath = myself()->property( Kopete::Global::Properties::self()->photo() ).value().toString();

	ContactManager* ssi = engine()->ssiManager();
	OContact item = ssi->findItemForIconByRef( 1 );
	
	if ( photoPath.isEmpty() )
	{
		if ( item )
		{
			kDebug(OSCAR_GEN_DEBUG) << "Removing icon hash item from ssi";
			OContact s(item);
			
			//remove hash and alias
			QList<TLV> tList( item.tlvList() );
			TLV t = Oscar::findTLV( tList, 0x00D5 );
			if ( t )
				tList.removeAll( t );
			
			t = Oscar::findTLV( tList, 0x0131 );
			if ( t )
				tList.removeAll( t );
			
			item.setTLVList( tList );
			//s is old, item is new. modification will occur
			engine()->modifyContactItem( s, item );
		}
	}
	else
	{
		QFile iconFile( photoPath );
		iconFile.open( QIODevice::ReadOnly );
		
		KMD5 iconHash;
		iconHash.update( iconFile );
		kDebug(OSCAR_GEN_DEBUG) << "hash is :" << iconHash.hexDigest();
	
		QByteArray iconTLVData;
		iconTLVData.resize( 18 );
		iconTLVData[0] = ( d->engine->isIcq() ) ? 0x01 : 0x00;
		iconTLVData[1] = 0x10;
		memcpy( iconTLVData.data() + 2, iconHash.rawDigest(), 16 );
		
		QList<Oscar::TLV> tList;
		tList.append( TLV( 0x00D5, iconTLVData.size(), iconTLVData ) );
		tList.append( TLV( 0x0131, 0, 0 ) );
		
		
		//find old item, create updated item
		if ( !item )
		{
			kDebug(OSCAR_GEN_DEBUG) << "no existing icon hash item in ssi. creating new";
			
			OContact s( "1", 0, ssi->nextContactId(), ROSTER_BUDDYICONS, tList );
			
			//item is a non-valid ssi item, so the function will add an item
			kDebug(OSCAR_GEN_DEBUG) << "setting new icon item";
			engine()->modifyContactItem( item, s );
		}
		else
		{ //found an item
			OContact s(item);
			
			if ( Oscar::updateTLVs( s, tList ) == true )
			{
				kDebug(OSCAR_GEN_DEBUG) << "modifying old item in ssi.";
				
				//s is old, item is new. modification will occur
				engine()->modifyContactItem( item, s );
			}
			else
			{
				kDebug(OSCAR_GEN_DEBUG) << "not updating, item is the same.";
			}
		}
		
		iconFile.close();
	}
	
	d->buddyIconDirty = false;
}

void OscarAccount::slotSendBuddyIcon()
{
	//need to disconnect because we could end up with many connections
	QObject::disconnect( engine(), SIGNAL(iconServerConnected()), this, SLOT(slotSendBuddyIcon()) );
	QString photoPath = myself()->property( Kopete::Global::Properties::self()->photo() ).value().toString();
	if ( photoPath.isEmpty() )
		return;
	
	kDebug(OSCAR_RAW_DEBUG) << photoPath;
	QFile iconFile( photoPath );
	
	if ( iconFile.open( QIODevice::ReadOnly ) )
	{
		if ( !engine()->hasIconConnection() )
		{
			//will send icon when we connect to icon server
			QObject::connect( engine(), SIGNAL(iconServerConnected()),
			                  this, SLOT(slotSendBuddyIcon()) );
			
			engine()->connectToIconServer();
			return;
		}
		QByteArray imageData = iconFile.readAll();
		engine()->sendBuddyIcon( imageData );
	}
}

void OscarAccount::slotIdentityPropertyChanged( Kopete::PropertyContainer*, const QString &key,
                                                const QVariant&, const QVariant &newValue )
{
	kDebug(OSCAR_GEN_DEBUG) << "Identity property changed";
	if ( key == Kopete::Global::Properties::self()->photo().key() )
	{
		updateBuddyIcon( newValue.toString() );
	}
}

void OscarAccount::slotGoOffline()
{
}

void OscarAccount::slotGoOnline()
{
}

QString OscarAccount::getFLAPErrorMessage( int code )
{
	bool isICQ = d->engine->isIcq();
	QString acctType = isICQ ? i18n("ICQ") : i18n("AIM");
	QString acctDescription = isICQ ? i18nc("ICQ user id", "UIN") : i18nc("AIM user id", "screen name");
	QString reason;
	//FLAP errors are always fatal
	//negative codes are things added by liboscar developers
	//to indicate generic errors in the task
	switch ( code )
	{
	case 0x0001:
		if ( isConnected() ) // multiple logins (on same UIN)
		{
			reason = i18n( "You have logged in more than once with the same %1," \
			               " account %2 is now disconnected.",
				  acctDescription, accountId() );
		}
		else // error while logging in
		{
			reason = i18n( "Sign on failed because either your %1 or " \
			               "password are invalid. Please check your settings for account %2.",
				  acctDescription, accountId() );

		}
		break;
	case 0x0002: // Service temporarily unavailable
	case 0x0014: // Reservation map error
		reason = i18n("The %1 service is temporarily unavailable. Please try again later.",
			  acctType );
		break;
	case 0x0004: // Incorrect nick or password, re-enter
	case 0x0005: // Mismatch nick or password, re-enter
		reason = i18n("Could not sign on to %1 with account %2 because the " \
		              "password was incorrect.", acctType, accountId() );
		break;
	case 0x0007: // non-existent ICQ#
	case 0x0008: // non-existent ICQ#
		reason = i18n("Could not sign on to %1 with nonexistent account %2.",
			  acctType, accountId() );
		break;
	case 0x0009: // Expired account
		reason = i18n("Sign on to %1 failed because your account %2 expired.",
			  acctType, accountId() );
		break;
	case 0x0011: // Suspended account
		reason = i18n("Sign on to %1 failed because your account %2 is " \
		              "currently suspended.", acctType, accountId() );
		break;
	case 0x0015: // too many clients from same IP
	case 0x0016: // too many clients from same IP
	case 0x0017: // too many clients from same IP (reservation)
		reason = i18n("Could not sign on to %1 as there are too many clients" \
		              " from the same computer.", acctType );
		break;
	case 0x0018: // rate exceeded (turboing)
		if ( isConnected() )
		{
			reason = i18n("Account %1 was blocked on the %2 server for" \
							" sending messages too quickly." \
							" Wait ten minutes and try again." \
							" If you continue to try, you will" \
							" need to wait even longer.",
				  accountId(), acctType );
		}
		else
		{
			reason = i18n("Account %1 was blocked on the %2 server for" \
							" reconnecting too quickly." \
							" Wait ten minutes and try again." \
							" If you continue to try, you will" \
							" need to wait even longer.",
				  accountId(), acctType) ;
		}
		break;
	case 0x001B:
	case 0x001C:
		if ( !d->versionAlreadyUpdated )
		{
			reason = i18n("Sign on to %1 with your account %2 failed.",
			              acctType, accountId() );
		}
		else
		{
			reason = i18n( "The %1 server thinks the client you are using is " \
			               "too old. Please report this as a bug at http://bugs.kde.org",
			               acctType );
		}
		break;
	case 0x0022: // Account suspended because of your age (age < 13)
		reason = i18n("Account %1 was disabled on the %2 server because " \
		              "of your age (under than 13).",
			  accountId(), acctType );
		break;
	default:
		if ( !isConnected() )
		{
			reason = i18n("Sign on to %1 with your account %2 failed.",
				  acctType, accountId() );
		}
		break;
	}
	return reason;
}

QString OscarAccount::makeWellFormedXML( const QString& message ) const
{
	// QList<QPair<tagName, data>>, if tagName isn't empty then data is <tag ....>
	// otherwise data is normal text which is between tags.
	QList< QPair<QString, QString> > tagsAndText;

	QRegExp tagRegExp( QString::fromLatin1("<([/]?[\\w]+).*>") );
	tagRegExp.setMinimal( true );
	int index = 0;

	while ( tagRegExp.indexIn( message, index ) != -1 )
	{
		if ( index < tagRegExp.pos() )
		{
			// Append text which was between tags
			QPair<QString, QString> pair;
			pair.second = message.mid( index, tagRegExp.pos() - index );
			tagsAndText.append( pair );
		}

		// Add tag
		QPair<QString, QString> pair;
		pair.first = tagRegExp.cap( 1 );
		pair.second = message.mid( tagRegExp.pos(), tagRegExp.matchedLength() );
		tagsAndText.append( pair );
		index = tagRegExp.pos() + tagRegExp.matchedLength();
	}

	if ( index < message.length() )
	{
		// Add text which was at end
		QPair<QString, QString> pair;
		pair.second = message.mid( index, message.length() - index );
		tagsAndText.append( pair );
	}

	// Make the list well-formed
	QStack<QString> openTags;
	const int tagsAndTextCount = tagsAndText.count();
	for ( int i = 0; i < tagsAndTextCount; ++i )
	{
		QPair<QString, QString> pair = tagsAndText.at( i );
		if ( pair.first.isEmpty() ) // We don't move text
			continue;
		
		bool endTag = pair.first.startsWith( "/" );
		if ( !endTag )
		{
			openTags.push( pair.first );
		}
		else if ( !openTags.isEmpty() )
		{
			QString desiredTag = "/" + openTags.pop();
			if ( pair.first != desiredTag )
			{
				// Find desired end tag and insert it into correct position
				for ( int j = i + 1; j < tagsAndTextCount; ++j )
				{
					QPair<QString, QString> pair2 = tagsAndText.at( j );
					if ( pair2.first.isEmpty() )
					{
						// Text is between desired tag so we can't move it
						qWarning() << "Can't make well-formed XML!";
						return message;
					}

					if ( pair2.first == desiredTag )
					{
						// Move tag to correct position
						tagsAndText.removeAt( j );
						tagsAndText.insert( i, pair2 );
						break;
					}
				}
			}
		}
	}

	QString wellFormedMessage;
	for ( int i = 0; i < tagsAndTextCount; ++i )
		wellFormedMessage += tagsAndText.at( i ).second;

	return wellFormedMessage;
}

QString OscarAccount::addQuotesAroundAttributes( QString message ) const
{
	int sIndex = 0;
	int eIndex = 0;
	int searchIndex = 0;

	QRegExp attrRegExp( "[\\d\\w]*=[^\"'/>\\s]+" );
	QString attrValue( "\"%1\"" );

	sIndex = message.indexOf( "<", eIndex );
	eIndex = message.indexOf( ">", sIndex );

	if ( sIndex == -1 || eIndex == -1 )
		return message;

	while ( attrRegExp.indexIn( message, searchIndex ) != -1 )
	{
		int startReplace = message.indexOf( "=", attrRegExp.pos() ) + 1;
		int replaceLength = attrRegExp.pos() + attrRegExp.matchedLength() - startReplace;

		while ( eIndex != -1 && sIndex != -1 && startReplace + replaceLength > eIndex )
		{
			sIndex = message.indexOf( "<", eIndex );
			eIndex = message.indexOf( ">", sIndex );
		}

		if ( sIndex == -1 || eIndex == -1 )
			return message;

		searchIndex = attrRegExp.pos() + attrRegExp.matchedLength();
		if ( startReplace <= sIndex )
			continue;

		QString replaceText = attrValue.arg( message.mid( startReplace, replaceLength ) );
		message.replace( startReplace, replaceLength, replaceText );

		searchIndex += 2;
		eIndex += 2;
	}

	return message;
}

QString OscarAccount::sanitizedPlainMessage( const QString& message ) const
{
	// FIXME: messages from AIM to ICQ shouldn't be escaped, we need to redesign this
	QString sanitizedMsg = (d->engine->isIcq()) ? Qt::escape( message ) : message;
	sanitizedMsg.replace( QRegExp(QString::fromLatin1("[\r]?[\n]")), QString::fromLatin1("<br />") );
	return sanitizedMsg;
}

QList<QDomNode> OscarAccount::getElementsByTagNameCI( const QDomNode& node, const QString& tagName ) const
{
	QList<QDomNode> nodeList;

	QDomNode childNode = node.firstChild();
	while ( !childNode.isNull() )
	{
		nodeList.append( getElementsByTagNameCI( childNode, tagName ) );
		if ( childNode.isElement() && childNode.nodeName().compare( tagName, Qt::CaseInsensitive ) == 0 )
			nodeList.append( childNode );

		childNode = childNode.nextSibling();
	}
	return nodeList;
}

void OscarAccount::createClientStream( ClientStream **clientStream )
{
	QSslSocket* tcpSocket = new QSslSocket();

	if (configGroup()->readEntry( QString::fromLatin1( "ProxyEnable" ), false))
	{
		QString proxyExp=configGroup()->readEntry( QString::fromLatin1( "ProxyServer" ), QString() );
		int proxyPort=configGroup()->readEntry( QString::fromLatin1( "ProxyPort" ), 0 );
		bool proxySocks5=configGroup()->readEntry( QString::fromLatin1( "ProxySocks5" ), false );
		tcpSocket->setProxy(QNetworkProxy(proxySocks5 ? QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy, proxyExp, proxyPort));
	}
	else
	{
		const QString &proxyUrl = KProtocolManager::proxyForUrl( KUrl( "http:" ) );
		if (!proxyUrl.isEmpty() && proxyUrl != QLatin1String( "DIRECT" ))
		{
			const KUrl url( proxyUrl );
			QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;
			if (url.protocol() == QLatin1String( "http" ))
				proxyType = QNetworkProxy::HttpProxy;
			else if (url.protocol() == QLatin1String( "socks" ))
				proxyType = QNetworkProxy::Socks5Proxy;
			if (proxyType != QNetworkProxy::NoProxy)
				tcpSocket->setProxy( QNetworkProxy( proxyType, url.host(), url.port(), url.user(), url.pass() ) );
		}
	}

	ClientStream *cs = new ClientStream( tcpSocket, 0 );
	
	Kopete::SocketTimeoutWatcher* timeoutWatcher = Kopete::SocketTimeoutWatcher::watch(tcpSocket);
	if ( timeoutWatcher )
	{
		QObject::connect( timeoutWatcher, SIGNAL(error(QAbstractSocket::SocketError)),
		                  cs, SLOT(socketError(QAbstractSocket::SocketError)) );
	}

	*clientStream = cs;
}

#include "oscaraccount.moc"
//kate: tab-width 4; indent-mode cstyle; replace-tabs 0;
