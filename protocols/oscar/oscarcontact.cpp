/*
  oscarcontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "oscarcontact.h"

#include <time.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qtimer.h>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <krandom.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include <kdeversion.h>
#include <kfiledialog.h>

#include "kopeteaccount.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include <kopeteglobal.h>
#include "kopetetransfermanager.h"

#include "oscaraccount.h"
#include "client.h"
#include "contactmanager.h"
#include "oscarutils.h"
#include "oscarprotocol.h"
#include "oscarencodingselectiondialog.h"
#include "oscarstatusmanager.h"

#include <assert.h>

OscarContact::OscarContact( Kopete::Account* account, const QString& name,
                            Kopete::MetaContact* parent, const QString& icon, const OContact& ssiItem )
: Kopete::Contact( account, name, parent, icon )
{
	mAccount = static_cast<OscarAccount*>(account);
	mName = name;
	mMsgManager = 0L;
	m_ssiItem = ssiItem;
	m_buddyIconDirty = false;
	m_haveAwayMessage = false;
	m_oesd = 0;

	connect( this, SIGNAL(updatedSSI()), this, SLOT(updateSSIItem()) );
	setFileCapable( true );

	QObject::connect( mAccount->engine(), SIGNAL(haveIconForContact(const QString&, QByteArray)),
	                  this, SLOT(haveIcon(const QString&, QByteArray)) );
	QObject::connect( mAccount->engine(), SIGNAL(iconServerConnected()),
	                  this, SLOT(requestBuddyIcon()) );
	QObject::connect( mAccount->engine(), SIGNAL(receivedAwayMessage(const QString&, const QString& )),
	                  this, SLOT(receivedStatusMessage(const QString&, const QString&)) );
}

OscarContact::~OscarContact()
{
}

void OscarContact::serialize(QMap<QString, QString> &serializedData,
                             QMap<QString, QString> &/*addressBookData*/)
{
	serializedData["ssi_name"] = m_ssiItem.name();
	serializedData["ssi_type"] = QString::number( m_ssiItem.type() );
	serializedData["ssi_gid"] = QString::number( m_ssiItem.gid() );
	serializedData["ssi_bid"] = QString::number( m_ssiItem.bid() );
	serializedData["ssi_alias"] = m_ssiItem.alias();
	serializedData["ssi_waitingAuth"] = m_ssiItem.waitingAuth() ? QString::fromLatin1( "true" ) : QString::fromLatin1( "false" );
}

bool OscarContact::isOnServer() const
{
    ContactManager* serverList = mAccount->engine()->ssiManager();
	OContact ssi = serverList->findContact( Oscar::normalize( contactId() ) );

	return ( ssi && ssi.type() != 0xFFFF );
}

void OscarContact::setSSIItem( const OContact& ssiItem )
{
	m_ssiItem = ssiItem;

	if ( !m_ssiItem.alias().isEmpty() )
		setProperty( Kopete::Global::Properties::self()->nickName(), m_ssiItem.alias() );

	emit updatedSSI();
}

OContact OscarContact::ssiItem() const
{
	return m_ssiItem;
}

Kopete::ChatSession* OscarContact::manager( CanCreateFlags canCreate )
{
	if ( !mMsgManager && canCreate )
	{
		/*kDebug(14190) << k_funcinfo <<
			"Creating new ChatSession for contact '" << displayName() << "'" << endl;*/

		QList<Kopete::Contact*> theContact;
		theContact.append(this);

		mMsgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), theContact, protocol());

		// This is for when the user types a message and presses send
		connect(mMsgManager, SIGNAL( messageSent( Kopete::Message&, Kopete::ChatSession * ) ),
		        this, SLOT( slotSendMsg( Kopete::Message&, Kopete::ChatSession * ) ) );

		// For when the message manager is destroyed
		connect(mMsgManager, SIGNAL( destroyed() ),
		        this, SLOT( chatSessionDestroyed() ) );

		connect(mMsgManager, SIGNAL( myselfTyping( bool ) ),
		        this, SLOT( slotTyping( bool ) ) );
	}
	return mMsgManager;
}

void OscarContact::deleteContact()
{
	mAccount->engine()->removeContact( contactId() );
	deleteLater();
}

void OscarContact::chatSessionDestroyed()
{
	mMsgManager = 0L;
}

// Called when the metacontact owning this contact has changed groups
void OscarContact::sync(unsigned int flags)
{
	/* 
	 * If the contact has changed groups, then we update the server
	 *   adding the group if it doesn't exist, changing the ssi item
	 *   contained in the client and updating the contact's ssi item
	 * Otherwise, we don't do much
	 */
	
	if( !metaContact() || metaContact()->isTemporary() )
		return;
	
	if ( (flags & Kopete::Contact::MovedBetweenGroup) == Kopete::Contact::MovedBetweenGroup )
	{
		
		kDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Moving a contact between groups" << endl;
		ContactManager* ssiManager = mAccount->engine()->ssiManager();
		
		OContact oldGroup = ssiManager->findGroup( m_ssiItem.gid() );
		Kopete::Group* newGroup = metaContact()->groups().first();
		if ( newGroup->displayName() == oldGroup.name() )
			return; //we didn't really move
		
		if ( m_ssiItem.isValid() )
			mAccount->changeContactGroupInSSI( contactId(), newGroup->displayName(), true );
		else
			mAccount->addContactToSSI( contactId(), newGroup->displayName(), true );
	}
	return;
}

void OscarContact::userInfoUpdated( const QString& contact, const UserDetails& details  )
{
	Q_UNUSED( contact );
	
	if ( details.buddyIconHash().size() > 0 && details.buddyIconHash() != m_details.buddyIconHash() )
	{
		m_buddyIconDirty = true;
		if ( cachedBuddyIcon( details.buddyIconHash() ) == false )
		{
			if ( !mAccount->engine()->hasIconConnection() )
			{
				mAccount->engine()->connectToIconServer();
			}
			else
			{
				int time = ( KRandom::random() % 10 ) * 1000;
				kDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "updating buddy icon in "
					<< time/1000 << " seconds" << endl;
				QTimer::singleShot( time, this, SLOT( requestBuddyIcon() ) );
			}
		}
	}
	
	setProperty( Kopete::Global::Properties::self()->onlineSince(), details.onlineSinceTime() );
	setIdleTime( details.idleTime() );
	m_warningLevel = details.warningLevel();
	m_details.merge( details );

	setFileCapable( m_details.hasCap( CAP_SENDFILE ) );

	QStringList capList;
	// Append client name and version in case we found one
	if ( m_details.userClass() & 0x0080 /* WIRELESS */ )
		capList << i18n( "Mobile AIM Client" );
	else
	{
		if ( !m_details.clientName().isEmpty() )
		{
			capList << i18nc( "Translators: client name and version",
			                "%1", m_details.clientName() );
		}
	}
	
	// and now for some general informative capabilities
	if ( m_details.hasCap( CAP_BUDDYICON ) )
		capList << i18n( "Buddy icons" );
	if ( m_details.hasCap( CAP_UTF8 ) )
		capList << i18n( "UTF-8" );
	if ( m_details.hasCap( CAP_RTFMSGS ) )
		capList << i18n( "Rich text messages" );
	if ( m_details.hasCap( CAP_CHAT ) )
		capList << i18n( "Group chat" );
	if ( m_details.hasCap( CAP_VOICE ) )
		capList << i18n( "Voice chat" );
	if ( m_details.hasCap( CAP_IMIMAGE ) )
		capList << i18n( "DirectIM/IMImage" );
	if ( m_details.hasCap( CAP_SENDBUDDYLIST ) )
		capList << i18n( "Send buddy list" );
	if ( m_details.hasCap( CAP_SENDFILE ) )
		capList << i18n( "File transfers" );
	if ( m_details.hasCap( CAP_GAMES ) || m_details.hasCap( CAP_GAMES2 ) )
		capList << i18n( "Games" );
	if ( m_details.hasCap( CAP_TRILLIAN ) )
		capList << i18n( "Trillian user" );
	
	m_clientFeatures = capList.join( ", " );
	setProperty( static_cast<OscarProtocol*>(protocol())->clientFeatures, m_clientFeatures );
}

void OscarContact::startedTyping()
{
	if ( mMsgManager )
		mMsgManager->receivedTypingMsg( this, true );
}

void OscarContact::stoppedTyping()
{
	if ( mMsgManager )
		mMsgManager->receivedTypingMsg( this, false );
}

void OscarContact::slotTyping( bool typing )
{
	if ( this != account()->myself() )
		account()->engine()->sendTyping( contactId(), typing );
}

QTextCodec* OscarContact::contactCodec() const
{
	if ( hasProperty( "contactEncoding" ) )
		return QTextCodec::codecForMib( property( "contactEncoding" ).value().toInt() );
	else
		return mAccount->defaultCodec();
}

bool OscarContact::hasCap( int capNumber ) const
{
	return m_details.hasCap( capNumber );
}

void OscarContact::setPresenceTarget( const Oscar::Presence &presence )
{
	OscarProtocol* p = static_cast<OscarProtocol *>(protocol());
	setOnlineStatus( p->statusManager()->onlineStatusOf( presence ) );
}

//here's where a filetransfer usually begins
//could be called by a KAction or our dcop code or something
void OscarContact::sendFile( const KUrl &sourceURL, const QString &altFileName, uint fileSize )
{
	kDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "file: '" << sourceURL 
		<< "' '" << altFileName << "' size " << fileSize << endl;
	QStringList files;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
		files = KFileDialog::getOpenFileNames( KUrl() ,"*", 0l  , i18n( "Kopete File Transfer" ));
	else
		files << sourceURL.path(KUrl::RemoveTrailingSlash);

	if( files.isEmpty() )
	{
		kDebug(OSCAR_GEN_DEBUG) << "files empty, assuming cancel" << endl;
		return;
	}
	kDebug(OSCAR_GEN_DEBUG) << "files: '" << files << "' " << endl;

	Kopete::Transfer *t = Kopete::TransferManager::transferManager()->addTransfer( this, files.at(0), QFile( files.at(0) ).size(), mName, Kopete::FileTransferInfo::Outgoing);
	mAccount->engine()->sendFiles( mName, files, t );
}

void OscarContact::setAwayMessage( const QString &message )
{
	kDebug(OSCAR_AIM_DEBUG) << k_funcinfo <<
		"Called for '" << contactId() << "', away msg='" << message << "'" << endl;
	
	if ( !message.isEmpty() )
		setProperty( static_cast<OscarProtocol*>( protocol() )->awayMessage, filterAwayMessage( message ) );
	else
		removeProperty( static_cast<OscarProtocol*>( protocol() )->awayMessage );
}

void OscarContact::changeContactEncoding()
{
	if ( m_oesd )
		return;

	OscarProtocol* p = static_cast<OscarProtocol*>( protocol() );
	m_oesd = new OscarEncodingSelectionDialog( Kopete::UI::Global::mainWidget(), property(p->contactEncoding).value().toInt() );
	connect( m_oesd, SIGNAL(closing(int)), this, SLOT(changeEncodingDialogClosed(int)) );
	m_oesd->show();
}

void OscarContact::changeEncodingDialogClosed( int result )
{
	if ( result == QDialog::Accepted )
	{
		OscarProtocol* p = static_cast<OscarProtocol*>( protocol() );
		int mib = m_oesd->selectedEncoding();
		if ( mib != 0 )
		{
			kDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "setting encoding mib to "
				<< m_oesd->selectedEncoding() << endl;
			setProperty( p->contactEncoding, m_oesd->selectedEncoding() );
		}
		else
		{
			kDebug(OSCAR_ICQ_DEBUG) << k_funcinfo
				<< "setting encoding to default" << endl;
			removeProperty( p->contactEncoding );
		}
	}
	
	if ( m_oesd )
	{
		m_oesd->deleteLater();
		m_oesd = 0L;
	}
}

void OscarContact::requestBuddyIcon()
{
	if ( m_buddyIconDirty && m_details.buddyIconHash().size() > 0 )
	{
		account()->engine()->requestBuddyIcon( contactId(), m_details.buddyIconHash(),
		                                       m_details.iconCheckSumType() );
	}
}

void OscarContact::haveIcon( const QString& user, QByteArray icon )
{
	if ( Oscar::normalize( user ) != Oscar::normalize( contactId() ) )
		return;
	
	kDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Updating icon for " << contactId() << endl;
	
	KMD5 buddyIconHash( icon );
	if ( memcmp( buddyIconHash.rawDigest(), m_details.buddyIconHash().data(), 16 ) == 0 )
	{
		QString iconLocation = KStandardDirs::locateLocal( "appdata", "oscarpictures/" + Oscar::normalize( contactId() ) );
		
		QFile iconFile( iconLocation );
		if ( !iconFile.open( QIODevice::WriteOnly ) )
		{
			kDebug(14153) << k_funcinfo << "Cannot open file"
				<< iconLocation << " for writing!" << endl;
			return;
		}
		
		iconFile.write( icon );
		iconFile.close();
		
		removeProperty( Kopete::Global::Properties::self()->photo() );
		setProperty( Kopete::Global::Properties::self()->photo(), iconLocation );
		m_buddyIconDirty = false;
	}
	else
	{
		kDebug(14153) << k_funcinfo << "Buddy icon hash does not match!" << endl;
		removeProperty( Kopete::Global::Properties::self()->photo() );
	}
}

void OscarContact::receivedStatusMessage( const QString& contact, const QString& message )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;
	
	setAwayMessage( message );
	m_haveAwayMessage = true;
}

QString OscarContact::filterAwayMessage( const QString &message ) const
{
	QString filteredMessage = message;
	filteredMessage.replace(
	                         QRegExp(QString::fromLatin1("<[hH][tT][mM][lL].*>(.*)</[hH][tT][mM][lL]>")),
	                         QString::fromLatin1("\\1"));
	filteredMessage.replace(
	                         QRegExp(QString::fromLatin1("<[bB][oO][dD][yY].*>(.*)</[bB][oO][dD][yY]>")),
	                         QString::fromLatin1("\\1") );
	QRegExp fontRemover( QString::fromLatin1("<[fF][oO][nN][tT].*>(.*)</[fF][oO][nN][tT]>") );
	fontRemover.setMinimal(true);
	while ( filteredMessage.indexOf( fontRemover ) != -1 )
		filteredMessage.replace( fontRemover, QString::fromLatin1("\\1") );
	return filteredMessage;
}

bool OscarContact::cachedBuddyIcon( QByteArray hash )
{
	QString iconLocation = KStandardDirs::locateLocal( "appdata", "oscarpictures/" + Oscar::normalize( contactId() ) );
	
	QFile iconFile( iconLocation );
	if ( !iconFile.open( QIODevice::ReadOnly ) )
		return false;
	
	KMD5 buddyIconHash;
	buddyIconHash.update( iconFile );
	iconFile.close();
	
	if ( memcmp( buddyIconHash.rawDigest(), hash.data(), 16 ) == 0 )
	{
		kDebug(OSCAR_GEN_DEBUG) << k_funcinfo << "Updating icon for "
			<< contactId() << " from local cache" << endl;
		
		setProperty( Kopete::Global::Properties::self()->photo(), iconLocation );
		m_buddyIconDirty = false;
		return true;
	}
	else
	{
		return false;
	}
}

#include "oscarcontact.moc"
//kate: tab-width 4; indent-mode csands;
