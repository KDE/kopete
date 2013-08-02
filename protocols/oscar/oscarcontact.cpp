/*
  oscarcontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
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

#include "oscarcontact.h"

#include <time.h>

#include <qapplication.h>
#include <qtextcodec.h>
#include <qtimer.h>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNodeList>
#include <QtGui/QTextDocument>
#include <QtGui/QTextCharFormat>
#include <QtGui/QTextBlock>

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <krandom.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kinputdialog.h>

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
#include "kopeteavatarmanager.h"

#include "oscaraccount.h"
#include "client.h"
#include "contactmanager.h"
#include "oscarutils.h"
#include "oscarprotocol.h"
#include "oscarencodingselectiondialog.h"
#include "oscarstatusmanager.h"
#include "filetransferhandler.h"

#include <assert.h>

OscarContact::OscarContact( Kopete::Account* account, const QString& name,
                            Kopete::MetaContact* parent, const QString& icon )
: Kopete::Contact( account, name, parent, icon )
{
	mAccount = static_cast<OscarAccount*>(account);
	mName = name;
	mMsgManager = 0L;
	m_buddyIconDirty = false;
	m_oesd = 0;

	setFileCapable( true );

	QObject::connect( mAccount->engine(), SIGNAL(haveIconForContact(QString,QByteArray)),
	                  this, SLOT(haveIcon(QString,QByteArray)) );
	QObject::connect( mAccount->engine(), SIGNAL(iconServerConnected()),
	                  this, SLOT(requestBuddyIcon()) );
	QObject::connect( mAccount->engine(), SIGNAL(receivedAwayMessage(QString,QString)),
	                  this, SLOT(receivedStatusMessage(QString,QString)) );
	QObject::connect( mAccount->engine(), SIGNAL(messageAck(QString,uint)),
	                  this, SLOT(messageAck(QString,uint)) );
	QObject::connect( mAccount->engine(), SIGNAL(messageError(QString,uint)),
	                  this, SLOT(messageError(QString,uint)) );
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
	serializedData["ssi_metaInfoId"] = m_ssiItem.metaInfoId().toHex();
}

bool OscarContact::isOnServer() const
{
    ContactManager* serverList = mAccount->engine()->ssiManager();
	OContact ssi = serverList->findContact( Oscar::normalize( contactId() ) );

	return ( ssi && ssi.type() != 0xFFFF );
}

void OscarContact::setSSIItem( const OContact& ssiItem )
{
	setCustomName( ssiItem.alias() );
	m_ssiItem = ssiItem;
}

OContact OscarContact::ssiItem() const
{
	return m_ssiItem;
}

Kopete::ChatSession* OscarContact::manager( CanCreateFlags canCreate )
{
	if ( !mMsgManager && canCreate )
	{
		/*kDebug(14190) <<
			"Creating new ChatSession for contact '" << displayName() << "'" << endl;*/

		QList<Kopete::Contact*> theContact;
		theContact.append(this);

		mMsgManager = Kopete::ChatSessionManager::self()->create(account()->myself(), theContact, protocol());

		// This is for when the user types a message and presses send
		connect(mMsgManager, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
		        this, SLOT(slotSendMsg(Kopete::Message&,Kopete::ChatSession*)) );

		// For when the message manager is destroyed
		connect(mMsgManager, SIGNAL(destroyed()),
		        this, SLOT(chatSessionDestroyed()) );

		connect(mMsgManager, SIGNAL(myselfTyping(bool)),
		        this, SLOT(slotTyping(bool)) );
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
		
		kDebug(OSCAR_GEN_DEBUG) << "Moving a contact between groups";
		ContactManager* ssiManager = mAccount->engine()->ssiManager();
		
		OContact oldGroup = ssiManager->findGroup( m_ssiItem.gid() );
		Kopete::Group* newGroup = metaContact()->groups().first();
		QString newGroupName = newGroup->displayName();
		if ( newGroup->type() == Kopete::Group::TopLevel )
			newGroupName = "Buddies";
		if ( newGroupName == oldGroup.name() )
			return; //we didn't really move
		
		if ( m_ssiItem.isValid() )
			mAccount->changeContactGroupInSSI( contactId(), newGroupName, true );
		else
			mAccount->addContactToSSI( contactId(), newGroupName, true );
	}

	if ( flags & Kopete::Contact::DisplayNameChanged && mAccount->engine() )
	{
		kDebug(OSCAR_GEN_DEBUG) << "Changing contact alias";
		mAccount->engine()->changeContactAlias( contactId(), metaContact()->displayName() );
	}
}

void OscarContact::userInfoUpdated( const QString& contact, const UserDetails& details  )
{
	Q_UNUSED( contact );
	
	if ( details.buddyIconHash().size() > 0 && details.buddyIconHash() != m_details.buddyIconHash() )
	{
		OscarProtocol *p = static_cast<OscarProtocol*>(protocol());
		QString photoPath = property( Kopete::Global::Properties::self()->photo() ).value().toString();
		if ( property( p->buddyIconHash ).value().toByteArray() != details.buddyIconHash() || QFileInfo(photoPath).size() == 0 )
		{
			m_buddyIconDirty = true;
			
			if ( !mAccount->engine()->hasIconConnection() )
			{
				mAccount->engine()->connectToIconServer();
			}
			else
			{
				int time = ( KRandom::random() % 10 ) * 1000;
				kDebug(OSCAR_GEN_DEBUG) << "updating buddy icon in "
					<< time/1000 << " seconds" << endl;
				QTimer::singleShot( time, this, SLOT(requestBuddyIcon()) );
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
	//if ( m_details.userClass() & 0x0080 /* WIRELESS */ )
	//	capList << i18n( "Mobile AIM Client" );
	//else
	//{
	//	if ( !m_details.clientName().isEmpty() )
	//	{
	//		capList << i18nc( "Translators: client name and version",
	//		                "%1", m_details.clientName() );
	//	}
	//}
	
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

	m_clientFeatures = capList.join( ", " );
	setProperty( static_cast<OscarProtocol*>(protocol())->clientFeatures, m_clientFeatures );

	setProperty( static_cast<OscarProtocol*>(protocol())->memberSince, details.memberSinceTime() );
	setProperty( static_cast<OscarProtocol*>(protocol())->client, details.clientName() );
	setProperty( static_cast<OscarProtocol*>(protocol())->protocolVersion, QString::number(details.dcProtoVersion()) );
}

void OscarContact::startedTyping()
{
	Kopete::ChatSession* cs = manager();
	// We want the user to know if someone is typing a message
	// but there is no chat session for this contact
	cs->receivedTypingMsg( this, true );
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

void OscarContact::messageAck( const QString& contact, uint messageId )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;
	
	Kopete::ChatSession* chatSession = manager();
	if ( chatSession )
		chatSession->receivedMessageState( messageId, Kopete::Message::StateSent );
}

void OscarContact::messageError( const QString& contact, uint messageId )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;
	
	Kopete::ChatSession* chatSession = manager();
	if ( chatSession )
		chatSession->receivedMessageState( messageId, Kopete::Message::StateError );
}

QTextCodec* OscarContact::contactCodec() const
{
	if ( hasProperty( "contactEncoding" ) )
	{
		QTextCodec* codec = QTextCodec::codecForMib( property( "contactEncoding" ).value().toInt() );

		if ( codec )
			return codec;
		else
			return QTextCodec::codecForMib( 4 );
	}
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

void OscarContact::setEncoding( int mib )
{
	OscarProtocol* p = static_cast<OscarProtocol*>( protocol() );
	if ( mib != 0 )
	{
		kDebug(OSCAR_GEN_DEBUG) << "setting encoding mib to " << mib << endl;
		setProperty( p->contactEncoding, m_oesd->selectedEncoding() );
	}
	else
	{
		kDebug(OSCAR_GEN_DEBUG) << "setting encoding to default" << endl;
		removeProperty( p->contactEncoding );
	}
}

//here's where a filetransfer usually begins
//could be called by a KAction or our dcop code or something
void OscarContact::sendFile( const KUrl &sourceURL, const QString &altFileName, uint fileSize )
{
	kDebug(OSCAR_GEN_DEBUG) << "file: '" << sourceURL 
		<< "' '" << altFileName << "' size " << fileSize << endl;
	QStringList files;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
		files = KFileDialog::getOpenFileNames( KUrl() ,"*", 0l  , i18n( "Kopete File Transfer" ));
	else
		files << sourceURL.path(KUrl::RemoveTrailingSlash);

	if( files.isEmpty() )
	{
		kDebug(OSCAR_GEN_DEBUG) << "files empty, assuming cancel";
		return;
	}
	kDebug(OSCAR_GEN_DEBUG) << "files: '" << files << "' ";

	FileTransferHandler *ftHandler = mAccount->engine()->createFileTransfer( mName, files );

	Kopete::TransferManager *transferManager = Kopete::TransferManager::transferManager();
	Kopete::Transfer *transfer = transferManager->addTransfer( this, files, ftHandler->totalSize(), mName, Kopete::FileTransferInfo::Outgoing);

	connect( transfer, SIGNAL(transferCanceled()), ftHandler, SLOT(cancel()) );

	connect( ftHandler, SIGNAL(transferCancelled()), transfer, SLOT(slotCancelled()) );
	connect( ftHandler, SIGNAL(transferError(int,QString)), transfer, SLOT(slotError(int,QString)) );
	connect( ftHandler, SIGNAL(transferProcessed(uint)), transfer, SLOT(slotProcessed(uint)) );
	connect( ftHandler, SIGNAL(transferFinished()), transfer, SLOT(slotComplete()) );
	connect( ftHandler, SIGNAL(transferNextFile(QString,QString)),
	         transfer, SLOT(slotNextFile(QString,QString)) );

	ftHandler->send();
}

void OscarContact::setAwayMessage( const QString &message )
{
	kDebug(OSCAR_AIM_DEBUG) <<
		"Called for '" << contactId() << "', away msg='" << message << "'" << endl;
	
	if ( !message.isEmpty() )
		setProperty( static_cast<OscarProtocol*>( protocol() )->statusMessage, filterAwayMessage( message ) );
	else
		removeProperty( static_cast<OscarProtocol*>( protocol() )->statusMessage );

	emit statusMessageChanged( this );
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

void OscarContact::requestAuthorization()
{
	QString info = i18n("The user %1 requires authorization before being added to a contact list. "
	                    "Do you want to send an authorization request?\n\nReason for requesting authorization:",
	                    displayName() );

	QString reason = KInputDialog::getText( i18n("Request Authorization"), info,
	                                        i18n("Please authorize me so I can add you to my contact list") );
	if ( !reason.isNull() )
		mAccount->engine()->requestAuth( contactId(), reason );
}

void OscarContact::slotSendMsg(Kopete::Message& message, Kopete::ChatSession *)
{
	if (message.plainBody().isEmpty()) // no text, do nothing
		return;
	//okay, now we need to change the message.escapedBody from real HTML to aimhtml.
	//looking right now for docs on that "format".
	//looks like everything except for alignment codes comes in the format of spans

	//font-style:italic -> <i>
	//font-weight:600 -> <b> (anything > 400 should be <b>, 400 is not bold)
	//text-decoration:underline -> <u>
	//font-family: -> <font face="">
	//font-size:xxpt -> <font ptsize=xx>

	QTextDocument doc;
	doc.setHtml( message.escapedBody() );

	QString rtfText = QString( "<HTML><BODY dir=\"%1\">" ).arg( message.isRightToLeft() ? "rtl" : "ltr" );

	bool hasFontTag = false;
	QTextCharFormat defaultCharFormat;
	for ( QTextBlock it = doc.begin(); it != doc.end(); it = it.next() )
	{
		QTextBlockFormat blockFormat = it.blockFormat();

		// Plain text message has p tags without margin attributes and Qt's topMargin()
		// returns default margins so we will end up with line break before text.
		if ( message.format() != Qt::PlainText || it.blockNumber() != 0 )
			rtfText += brMargin( blockFormat.topMargin(), defaultCharFormat.fontPointSize() );

		bool lastFragmentHasLineSeparator = false;
		for ( QTextBlock::iterator it2 = it.begin(); !(it2.atEnd()); ++it2 )
		{
			QTextFragment currentFragment = it2.fragment();
			if ( currentFragment.isValid() )
			{
				QTextCharFormat format = currentFragment.charFormat();
				if ( format.fontFamily() != defaultCharFormat.fontFamily() ||
				     format.foreground() != defaultCharFormat.foreground() ||
				     oscarFontSize(format.fontPointSize()) != oscarFontSize(defaultCharFormat.fontPointSize()) )
				{
					if ( hasFontTag )
					{
						rtfText += "</FONT>";
						hasFontTag = false;
					}

					QString fontTag;
					if ( !format.fontFamily().isEmpty() )
						fontTag += QString( " FACE=\"%1\"" ).arg( format.fontFamily() );
					if ( format.fontPointSize() > 0 )
						fontTag += QString( " SIZE=%1" ).arg( oscarFontSize( format.fontPointSize() ) );
					if ( format.foreground().style() != Qt::NoBrush )
						fontTag += QString( " COLOR=%1" ).arg( format.foreground().color().name() );
					if ( format.background().style() != Qt::NoBrush )
						fontTag += QString( " BACK=%1" ).arg( format.background().color().name() );

					if ( !fontTag.isEmpty() )
					{
						rtfText += QString("<FONT%1>").arg( fontTag );
						hasFontTag = true;
					}
				}

				if ( format.font().bold() != defaultCharFormat.font().bold() )
					rtfText += ( format.font().bold() ) ? "<B>" : "</B>";
				if ( format.fontItalic() != defaultCharFormat.fontItalic() )
					rtfText += ( format.hasProperty(QTextFormat::FontItalic) ) ? "<I>" : "</I>";
				if ( format.fontUnderline() != defaultCharFormat.fontUnderline() )
					rtfText += ( format.hasProperty(QTextFormat::FontUnderline) ) ? "<U>" : "</U>";

				QString text = currentFragment.text();
				lastFragmentHasLineSeparator = text.endsWith( QChar::LineSeparator );
				rtfText += Qt::escape( text );
				defaultCharFormat = format;
			}
		}
		rtfText += brMargin( blockFormat.bottomMargin(), defaultCharFormat.fontPointSize(), !lastFragmentHasLineSeparator );
	}

	rtfText.replace( QChar::LineSeparator, "<BR>" );

	if ( rtfText.endsWith( "<BR>" ) )
		rtfText.chop(4);

	if ( hasFontTag )
		rtfText += "</FONT>";
	if ( defaultCharFormat.font().bold() )
		rtfText += "</B>";
	if ( defaultCharFormat.hasProperty( QTextFormat::FontItalic ) )
		rtfText += "</I>";
	if ( defaultCharFormat.hasProperty( QTextFormat::FontUnderline ) )
		rtfText += "</U>";

	rtfText += "</BODY></HTML>";

	kDebug(OSCAR_GEN_DEBUG) << "sending: " << rtfText;

	// TODO: Need to check for message size?

	Oscar::Message msg;
	// Allow UCS2 because official AIM client doesn't sets the CAP_UTF8 anymore!
	bool allowUCS2 = !isOnline() || !(m_details.userClass() & Oscar::CLASS_ICQ) || m_details.hasCap( CAP_UTF8 );
	msg.setText( Oscar::Message::encodingForText( rtfText, allowUCS2 ), rtfText, contactCodec() );

	msg.setId( message.id() );
	msg.setReceiver(mName);
	msg.setSender( mAccount->accountId() );
	msg.setTimestamp(message.timestamp());
	msg.setChannel(0x01);

	mAccount->engine()->sendMessage(msg);

	message.setState( Kopete::Message::StateSending );
	// Show the message we just sent in the chat window
	manager(Kopete::Contact::CanCreate)->appendMessage(message);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void OscarContact::changeEncodingDialogClosed( int result )
{
	if ( result == QDialog::Accepted )
		setEncoding( m_oesd->selectedEncoding() );
	
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
		                                       m_details.iconType(), m_details.iconCheckSumType() );
	}
}

void OscarContact::haveIcon( const QString& user, QByteArray icon )
{
	if ( Oscar::normalize( user ) != Oscar::normalize( contactId() ) )
		return;
	
	kDebug(OSCAR_GEN_DEBUG) << "Updating icon for " << contactId();
	
	KMD5 buddyIconHash( icon );
	if ( memcmp( buddyIconHash.rawDigest(), m_details.buddyIconHash().data(), 16 ) == 0 )
	{
		QImage img;
		img.loadFromData(icon);
		Kopete::AvatarManager::AvatarEntry entry;
		entry.name = contactId();
		entry.category = Kopete::AvatarManager::Contact;
		entry.contact = this;
		entry.image = img;
		entry = Kopete::AvatarManager::self()->add(entry);

		if (!entry.dataPath.isNull())
		{
			removeProperty( Kopete::Global::Properties::self()->photo() );
			setProperty( Kopete::Global::Properties::self()->photo(), entry.dataPath );
			setProperty( static_cast<OscarProtocol*>(protocol())->buddyIconHash, m_details.buddyIconHash() );
		}

		m_buddyIconDirty = false;
	}
	else
	{
		kDebug(14153) << "Buddy icon hash does not match!";
		removeProperty( static_cast<OscarProtocol*>(protocol())->buddyIconHash );
		removeProperty( Kopete::Global::Properties::self()->photo() );
	}
}

void OscarContact::receivedStatusMessage( const QString& contact, const QString& message )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;
	
	setAwayMessage( message );
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

int OscarContact::oscarFontSize( int size ) const
{
	if ( size <= 0 )
		return 0;
	else if ( 1 <= size && size <= 9 )
		return 1;
	else if ( 10 <= size && size <= 11 )
		return 2;
	else if ( 12 <= size && size <= 13 )
		return 3;
	else if ( 14 <= size && size <= 16 )
		return 4;
	else if ( 17 <= size && size <= 22 )
		return 5;
	else if ( 23 <= size && size <= 29 )
		return 6;
	else
		return 7;
}

QString OscarContact::brMargin( int margin, int fontPointSize, bool forceBr ) const
{
	int brHeight = ( fontPointSize == 0 ) ? 12 : fontPointSize;
	int brCount = margin / brHeight;
	
	if ( brCount <= 0 )
		return ( forceBr ) ? "<BR>" : "";
	
	QString s;
	while ( brCount-- > 0 )
		s += "<BR>";
	
	return s;
}

#include "oscarcontact.moc"
//kate: tab-width 4; indent-mode csands;
