/*
  icqontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003      by Stefan Gehn  <metz AT gehn.net>
  Copyright (c) 2003      by Olivier Goffart
  Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "icqcontact.h"

#include <qtimer.h>
#include <qimage.h>
#include <qfile.h>

#include <kaction.h>
#include <kactionclasses.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kpassivepopup.h>
#include <kinputdialog.h>
#include <kmdcodec.h>
#include <kstandarddirs.h>

#include "kopetechatsessionmanager.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"

#include "icquserinfo.h"
#include "icqreadaway.h"
#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqpresence.h"
#include "icquserinfowidget.h"
#include "icqauthreplydialog.h"

#include "client.h"
#include "oscarutils.h"
#include "oscarencodingselectiondialog.h"
#include "ssimanager.h"

ICQContact::ICQContact( ICQAccount *account, const QString &name, Kopete::MetaContact *parent,
						const QString& icon, const Oscar::SSI& ssiItem )
: OscarContact( account, name, parent, icon, ssiItem )
{
	mProtocol = static_cast<ICQProtocol *>(protocol());
	m_infoWidget = 0L;
	m_requestingNickname = false;
    m_oesd = 0;
	m_buddyIconDirty = false;

	if ( ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );
	else
		setOnlineStatus( ICQ::Presence( ICQ::Presence::Offline, ICQ::Presence::Visible ).toOnlineStatus() );

	QObject::connect( mAccount->engine(), SIGNAL( loggedIn() ), this, SLOT( loggedIn() ) );
	//QObject::connect( mAccount->engine(), SIGNAL( userIsOnline( const QString& ) ), this, SLOT( userOnline( const QString&, UserDetails ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( userIsOffline( const QString& ) ), this, SLOT( userOffline( const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( authRequestReceived( const QString&, const QString& ) ),
	                  this, SLOT( slotGotAuthRequest( const QString&, const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( authReplyReceived( const QString&, const QString&, bool ) ),
	                  this, SLOT( slotGotAuthReply(const QString&, const QString&, bool ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( receivedIcqShortInfo( const QString& ) ),
	                  this, SLOT( receivedShortInfo( const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( receivedIcqLongInfo( const QString& ) ),
	                  this, SLOT( receivedLongInfo( const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( receivedUserInfo( const QString&, const UserDetails& ) ),
	                  this, SLOT( userInfoUpdated( const QString&, const UserDetails& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( receivedAwayMessage( const QString&, const QString& ) ),
	                  this, SLOT( receivedStatusMessage( const QString&, const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( receivedAwayMessage( const Oscar::Message& ) ),
	                  this, SLOT( receivedStatusMessage( const Oscar::Message& ) ) );
	QObject::connect( this, SIGNAL( featuresUpdated() ), this, SLOT( updateFeatures() ) );
	QObject::connect( mAccount->engine(), SIGNAL( iconServerConnected() ),
	                  this, SLOT( requestBuddyIcon() ) );
	QObject::connect( mAccount->engine(), SIGNAL( haveIconForContact( const QString&, QByteArray ) ),
	                  this, SLOT( haveIcon( const QString&, QByteArray ) ) );

}

ICQContact::~ICQContact()
{
	delete m_infoWidget;
}

void ICQContact::updateSSIItem()
{
	//kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << endl;
	if ( m_ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );

	if ( m_ssiItem.type() != 0xFFFF && m_ssiItem.waitingAuth() == false &&
	     onlineStatus() == Kopete::OnlineStatus::Unknown )
	{
		//make sure they're offline
		setOnlineStatus( ICQ::Presence( ICQ::Presence::Offline, ICQ::Presence::Visible ).toOnlineStatus() );
	}
}


void ICQContact::userInfoUpdated( const QString& contact, const UserDetails& details )
{
	//kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << contact << contactId() << endl;
	if ( Oscar::normalize( contact  ) != Oscar::normalize( contactId() ) )
		return;

	// invalidate old away message if user was offline
	if ( !isOnline() )
		removeProperty( mProtocol->awayMessage );

	kdDebug( OSCAR_ICQ_DEBUG ) << k_funcinfo << "extendedStatus is " << details.extendedStatus() << endl;
	ICQ::Presence presence = ICQ::Presence::fromOscarStatus( details.extendedStatus() & 0xffff );
	setOnlineStatus( presence.toOnlineStatus() );

	// ICQ does not support status messages for state Online
	if ( presence.type() == ICQ::Presence::Online )
	{
		mAccount->engine()->removeICQAwayMessageRequest( contactId() );
		removeProperty( mProtocol->awayMessage );
	}
	else
	{
		if ( ICQ::Presence::fromOnlineStatus( account()->myself()->onlineStatus() ).visibility() == ICQ::Presence::Visible )
		{
			switch ( presence.type() )
			{
			case ICQ::Presence::Away:
				mAccount->engine()->addICQAwayMessageRequest( contactId(), Client::ICQAway );
				break;
			case ICQ::Presence::NotAvailable:
				mAccount->engine()->addICQAwayMessageRequest( contactId(), Client::ICQNotAvailable );
				break;
			case ICQ::Presence::Occupied:
				mAccount->engine()->addICQAwayMessageRequest( contactId(), Client::ICQOccupied );
				break;
			case ICQ::Presence::DoNotDisturb:
				mAccount->engine()->addICQAwayMessageRequest( contactId(), Client::ICQDoNotDisturb );
				break;
			case ICQ::Presence::FreeForChat:
				mAccount->engine()->addICQAwayMessageRequest( contactId(), Client::ICQFreeForChat );
				break;
			default:
				break;
			}
		}
		else
		{
			mAccount->engine()->removeICQAwayMessageRequest( contactId() );
		}
	}
		

	if ( details.dcOutsideSpecified() )
	{
		if ( details.dcExternalIp().isUnspecified() )
			removeProperty( mProtocol->ipAddress );
		else
			setProperty( mProtocol->ipAddress, details.dcExternalIp().toString() );
	}

	if ( details.capabilitiesSpecified() )
	{
		if ( details.clientName().isEmpty() )
			removeProperty( mProtocol->clientFeatures );
		else
			setProperty( mProtocol->clientFeatures, details.clientName() );
	}

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
				int time = ( KApplication::random() % 10 ) * 1000;
				kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "updating buddy icon in "
				                         << time/1000 << " seconds" << endl;
				QTimer::singleShot( time, this, SLOT( requestBuddyIcon() ) );
			}
		}
	}

	OscarContact::userInfoUpdated( contact, details );
}

void ICQContact::userOnline( const QString& userId )
{
	if ( Oscar::normalize( userId ) != Oscar::normalize( contactId() ) )
		return;

	kdDebug(OSCAR_ICQ_DEBUG) << "Setting " << userId << " online" << endl;
	ICQ::Presence online = mProtocol->statusManager()->presenceOf( ICQ::Presence::Online );
	//mAccount->engine()->requestStatusInfo( contactId() );
}

void ICQContact::userOffline( const QString& userId )
{
	if ( Oscar::normalize( userId ) != Oscar::normalize( contactId() ) )
		return;

	kdDebug(OSCAR_ICQ_DEBUG) << "Setting " << userId << " offline" << endl;
	ICQ::Presence offline = mProtocol->statusManager()->presenceOf( ICQ::Presence::Offline );
	setOnlineStatus( mProtocol->statusManager()->onlineStatusOf( offline ) );
}

void ICQContact::loggedIn()
{
	if ( metaContact()->isTemporary() )
		return;

	if ( m_ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );

	if ( ( ( hasProperty( Kopete::Global::Properties::self()->nickName().key() )
	         && nickName() == contactId() )
	     || !hasProperty( Kopete::Global::Properties::self()->nickName().key() ) ) &&
	     !m_requestingNickname && m_ssiItem.alias().isEmpty() )
	{
		m_requestingNickname = true;
		int time = ( KApplication::random() % 20 ) * 1000;
		kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "updating nickname in " << time/1000 << " seconds" << endl;
		QTimer::singleShot( time, this, SLOT( requestShortInfo() ) );
	}

}

void ICQContact::requestShortInfo()
{
	if ( mAccount->isConnected() )
		mAccount->engine()->requestShortInfo( contactId() );
}

void ICQContact::slotRequestAuth()
{
	QString reason = KInputDialog::getText( i18n("Request Authorization"),
	                                        i18n("Reason for requesting authorization:") );
	if ( !reason.isNull() )
		mAccount->engine()->requestAuth( contactId(), reason );
}

void ICQContact::slotSendAuth()
{
	kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "Sending auth reply" << endl;
	ICQAuthReplyDialog replyDialog( 0, "replyDialog", false );

	replyDialog.setUser( property( Kopete::Global::Properties::self()->nickName() ).value().toString() );
	if ( replyDialog.exec() )
		mAccount->engine()->sendAuth( contactId(), replyDialog.reason(), replyDialog.grantAuth() );
}

void ICQContact::slotGotAuthReply( const QString& contact, const QString& reason, bool granted )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << endl;
	QString message;
	if( granted )
	{
		message = i18n( "User %1 has granted your authorization request.\nReason: %2" )
			.arg( property( Kopete::Global::Properties::self()->nickName() ).value().toString() )
			.arg( reason );

		// remove the unknown status
		setOnlineStatus( ICQ::Presence( ICQ::Presence::Offline, ICQ::Presence::Visible ).toOnlineStatus() );
	}
	else
	{
		message = i18n( "User %1 has rejected the authorization request.\nReason: %2" )
			.arg( property( Kopete::Global::Properties::self()->nickName() ).value().toString() )
			.arg( reason );
	}
	KNotifyClient::event( Kopete::UI::Global::sysTrayWId(), "icq_authorization", message );
}

void ICQContact::slotGotAuthRequest( const QString& contact, const QString& reason )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	ICQAuthReplyDialog *replyDialog = new ICQAuthReplyDialog();

	connect( replyDialog, SIGNAL( okClicked() ), this, SLOT( slotAuthReplyDialogOkClicked() ) );
	replyDialog->setUser( property( Kopete::Global::Properties::self()->nickName() ).value().toString() );
	replyDialog->setRequestReason( reason );
	replyDialog->setModal( TRUE );
	replyDialog->show();
}

void ICQContact::slotAuthReplyDialogOkClicked()
{
    // Do not need to delete will delete itself automatically
    ICQAuthReplyDialog *replyDialog = (ICQAuthReplyDialog*)sender();

    if (replyDialog)
	mAccount->engine()->sendAuth( contactId(), replyDialog->reason(), replyDialog->grantAuth() );
}

void ICQContact::receivedLongInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
	{
		if ( m_infoWidget )
			m_infoWidget->delayedDestruct();
		return;
	}

	QTextCodec* codec = contactCodec();

	kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "received long info from engine" << endl;

	ICQGeneralUserInfo genInfo = mAccount->engine()->getGeneralInfo( contact );
	if ( m_ssiItem.alias().isEmpty() && !genInfo.nickname.isEmpty() )
		setNickName( codec->toUnicode( genInfo.nickname ) );
	emit haveBasicInfo( genInfo );

	ICQWorkUserInfo workInfo = mAccount->engine()->getWorkInfo( contact );
	emit haveWorkInfo( workInfo );

	ICQMoreUserInfo moreInfo = mAccount->engine()->getMoreInfo( contact );
	emit haveMoreInfo( moreInfo );

	ICQInterestInfo interestInfo = mAccount->engine()->getInterestInfo( contact );
	emit haveInterestInfo( interestInfo );

}

void ICQContact::receivedShortInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	QTextCodec* codec = contactCodec();

	m_requestingNickname = false; //done requesting nickname
	ICQShortInfo shortInfo = mAccount->engine()->getShortInfo( contact );
	/*
	if(!shortInfo.firstName.isEmpty())
		setProperty( mProtocol->firstName, codec->toUnicode( shortInfo.firstName ) );
	else
		removeProperty(mProtocol->firstName);

	if(!shortInfo.lastName.isEmpty())
		setProperty( mProtocol->lastName, codec->toUnicode( shortInfo.lastName ) );
	else
		removeProperty(mProtocol->lastName);
	*/
	if ( m_ssiItem.alias().isEmpty() && !shortInfo.nickname.isEmpty() )
	{
		kdDebug(14153) << k_funcinfo <<
			"setting new displayname for former UIN-only Contact" << endl;
		setProperty( Kopete::Global::Properties::self()->nickName(), codec->toUnicode( shortInfo.nickname ) );
	}

}

void ICQContact::receivedStatusMessage( const QString &contact, const QString &message )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	if ( ! message.isEmpty() )
		setProperty( mProtocol->awayMessage, message );
	else
		removeProperty( mProtocol->awayMessage );
}

void ICQContact::receivedStatusMessage( const Oscar::Message &message )
{
	if ( Oscar::normalize( message.sender() ) != Oscar::normalize( contactId() ) )
		return;
	
	//decode message
	QTextCodec* codec = contactCodec();
	
	QString realText = message.text(codec);

	if ( !realText.isEmpty() )
		setProperty( mProtocol->awayMessage, realText );
	else
		removeProperty( mProtocol->awayMessage );
}

void ICQContact::slotSendMsg( Kopete::Message& msg, Kopete::ChatSession* session )
{
	//Why is this unused?
	Q_UNUSED( session );

	QTextCodec* codec = contactCodec();

	int messageChannel = 0x01;
	Oscar::Message::Encoding messageEncoding;

	if ( isOnline() && m_details.hasCap( CAP_UTF8 ) )
		messageEncoding = Oscar::Message::UCS2;
	else
		messageEncoding = Oscar::Message::UserDefined;

	QString msgText( msg.plainBody() );
	// TODO: More intelligent handling of message length.
	uint chunk_length = !isOnline() ? 450 : 4096;
	uint msgPosition = 0;

	do
	{
		QString msgChunk( msgText.mid( msgPosition, chunk_length ) );
		// Try to split on space if needed
		if ( msgChunk.length() == chunk_length )
		{
			for ( int i = 0; i < 100; i++ )
			{
				if ( msgChunk[chunk_length - i].isSpace() )
				{
					msgChunk = msgChunk.left( chunk_length - i );
					msgPosition++;
				}
			}
		}
		msgPosition += msgChunk.length();

		Oscar::Message message( messageEncoding, msgChunk, messageChannel, 0, msg.timestamp(), codec );
		message.setSender( mAccount->accountId() );
		message.setReceiver( mName );
		mAccount->engine()->sendMessage( message );
	} while ( msgPosition < msgText.length() );

	manager(Kopete::Contact::CanCreate)->appendMessage(msg);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void ICQContact::updateFeatures()
{
	setProperty( static_cast<ICQProtocol*>(protocol())->clientFeatures, m_clientFeatures );
}

void ICQContact::requestBuddyIcon()
{
	if ( m_buddyIconDirty && m_details.buddyIconHash().size() > 0 )
	{
		account()->engine()->requestBuddyIcon( contactId(), m_details.buddyIconHash(),
		                                       m_details.iconCheckSumType() );
	}
}

void ICQContact::haveIcon( const QString& user, QByteArray icon )
{
	if ( Oscar::normalize( user ) != Oscar::normalize( contactId() ) )
		return;
	
	kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "Updating icon for " << contactId() << endl;
	
	KMD5 buddyIconHash( icon );
	if ( memcmp( buddyIconHash.rawDigest(), m_details.buddyIconHash().data(), 16 ) == 0 )
	{
		QString iconLocation( locateLocal( "appdata", "oscarpictures/"+ contactId() ) );
		
		QFile iconFile( iconLocation );
		if ( !iconFile.open( IO_WriteOnly ) )
		{
			kdDebug(14153) << k_funcinfo << "Cannot open file"
			               << iconLocation << " for writing!" << endl;
			return;
		}
		
		iconFile.writeBlock( icon );
		iconFile.close();
		
		setProperty( Kopete::Global::Properties::self()->photo(), QString::null );
		setProperty( Kopete::Global::Properties::self()->photo(), iconLocation );
		m_buddyIconDirty = false;
	}
	else
	{
		kdDebug(14153) << k_funcinfo << "Buddy icon hash does not match!" << endl;
		removeProperty( Kopete::Global::Properties::self()->photo() );
	}
}

bool ICQContact::cachedBuddyIcon( QByteArray hash )
{
	QString iconLocation( locateLocal( "appdata", "oscarpictures/"+ contactId() ) );
	
	QFile iconFile( iconLocation );
	if ( !iconFile.open( IO_ReadOnly ) )
		return false;
	
	KMD5 buddyIconHash;
	buddyIconHash.update( iconFile );
	iconFile.close();
	
	if ( memcmp( buddyIconHash.rawDigest(), hash.data(), 16 ) == 0 )
	{
		kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "Updating icon for "
		                         << contactId() << " from local cache" << endl;
		setProperty( Kopete::Global::Properties::self()->photo(), QString::null );
		setProperty( Kopete::Global::Properties::self()->photo(), iconLocation );
		m_buddyIconDirty = false;
		return true;
	}
	else
	{
		return false;
	}
}

#if 0
void ICQContact::slotContactChanged(const UserInfo &u)
{
	if (u.sn != contactName())
		return;

	// update mInfo and general stuff from OscarContact
	slotParseUserInfo(u);

	/*kdDebug(14190) << k_funcinfo << "Called for '"
		<< displayName() << "', contactName()=" << contactName() << endl;*/
	QStringList capList;
	// Append client name and version in case we found one
	if (!mInfo.clientName.isEmpty())
	{
		if (!mInfo.clientVersion.isEmpty())
		{
			capList << i18n("Translators: client-name client-version",
				"%1 %2").arg(mInfo.clientName, mInfo.clientVersion);
		}
		else
		{
			capList << mInfo.clientName;
		}
	}
	// and now for some general informative capabilities
	if (hasCap(CAP_UTF8))
		capList << i18n("UTF-8");
	if (hasCap(CAP_RTFMSGS))
		capList << i18n("RTF-Messages");
	if (hasCap(CAP_IMIMAGE))
		capList << i18n("DirectIM/IMImage");
	if (hasCap(CAP_CHAT))
		capList << i18n("Groupchat");

	if (capList.count() > 0)
		setProperty(mProtocol->clientFeatures, capList.join(", "));
	else
		removeProperty(mProtocol->clientFeatures);

	unsigned int newStatus = 0;
	mInvisible = (mInfo.icqextstatus & ICQ_STATUS_IS_INVIS);

	if (mInfo.icqextstatus & ICQ_STATUS_IS_FFC)
		newStatus = OSCAR_FFC;
	else if (mInfo.icqextstatus & ICQ_STATUS_IS_DND)
		newStatus = OSCAR_DND;
	else if (mInfo.icqextstatus & ICQ_STATUS_IS_OCC)
		newStatus = OSCAR_OCC;
	else if (mInfo.icqextstatus & ICQ_STATUS_IS_NA)
		newStatus = OSCAR_NA;
	else if (mInfo.icqextstatus & ICQ_STATUS_IS_AWAY)
		newStatus = OSCAR_AWAY;
	else
		newStatus = OSCAR_ONLINE;

	if (this != account()->myself())
	{
		if(newStatus != onlineStatus().internalStatus())
		{
			if(newStatus != OSCAR_ONLINE) // if user changed to some state other than online
			{
				mAccount->engine()->requestAwayMessage(this);
			}
			else // user changed to "Online" status and has no away message anymore
			{
				removeProperty(mProtocol->awayMessage);
			}
		}
	}

	setStatus(newStatus);
}

void ICQContact::slotOffgoingBuddy(QString sender)
{
	if(sender != contactName())
		return;

	removeProperty(mProtocol->clientFeatures);
	removeProperty(mProtocol->awayMessage);
	setOnlineStatus(mProtocol->statusOffline);
}

void ICQContact::gotIM(OscarSocket::OscarMessageType /*type*/, const QString &message)
{
	// Build a Kopete::Message and set the body as Rich Text
	Kopete::ContactPtrList tmpList;
	tmpList.append(account()->myself());
	Kopete::Message msg(this, tmpList, message, Kopete::Message::Inbound,
		Kopete::Message::RichText);
	manager(true)->appendMessage(msg);
}


void ICQContact::slotSendMsg(Kopete::Message& message, Kopete::ChatSession *)
{
	if (message.plainBody().isEmpty()) // no text, do nothing
		return;

	// Check to see if we're even online
	if(!account()->isConnected())
	{
		KMessageBox::sorry(Kopete::UI::Global::mainWidget(),
			i18n("<qt>You must be logged on to ICQ before you can "
				"send a message to a user.</qt>"),
			i18n("Not Signed On"));
		return;
	}

	// FIXME: We don't do HTML in ICQ
	// we might be able to do that in AIM and we might also convert
	// HTML to RTF for ICQ type-2 messages  [mETz]
	static_cast<OscarAccount*>(account())->engine()->sendIM(
		message.plainBody(), this, false);

	// Show the message we just sent in the chat window
	manager(Kopete::Contact::CanCreate)->appendMessage(message);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

#endif

bool ICQContact::isReachable()
{
	return account()->isConnected();
}

QPtrList<KAction> *ICQContact::customContextMenuActions()
{
	QPtrList<KAction> *actionCollection = new QPtrList<KAction>();
/*
	QString awTxt;
	QString awIcn;
	unsigned int status = onlineStatus().internalStatus();
	if (status >= 15)
		status -= 15; // get rid of invis addon
	switch(status)
	{
		case OSCAR_FFC:
			awTxt = i18n("Read 'Free For Chat' &Message");
			awIcn = "icq_ffc";
			break;
		case OSCAR_DND:
			awTxt = i18n("Read 'Do Not Disturb' &Message");
			awIcn = "icq_dnd";
			break;
		case OSCAR_NA:
			awTxt = i18n("Read 'Not Available' &Message");
			awIcn = "icq_na";
			break;
		case OSCAR_OCC:
			awTxt = i18n("Read 'Occupied' &Message");
			awIcn = "icq_occ";
			break;
		default:
			awTxt = i18n("Read 'Away' &Message");
			awIcn = "icq_away";
			break;
	}

	if(actionReadAwayMessage==0)
	{
		actionReadAwayMessage = new KAction(awTxt, awIcn, 0,
			this, SLOT(slotReadAwayMessage()), this, "actionReadAwayMessage");
		*/
		actionRequestAuth = new KAction(i18n("&Request Authorization"), "mail_reply", 0,
			this, SLOT(slotRequestAuth()), this, "actionRequestAuth");
		actionSendAuth = new KAction(i18n("&Grant Authorization"), "mail_forward", 0,
			this, SLOT(slotSendAuth()), this, "actionSendAuth");
		/*
	}
	else
	{
		actionReadAwayMessage->setText(awTxt);
		actionReadAwayMessage->setIconSet(SmallIconSet(awIcn));
	}

*/
	m_actionIgnore = new KToggleAction(i18n("&Ignore"), "", 0,
	                                   this, SLOT(slotIgnore()), this, "actionIgnore");
	m_actionVisibleTo = new KToggleAction(i18n("Always &Visible To"), "", 0,
	                                      this, SLOT(slotVisibleTo()), this, "actionVisibleTo");
	m_actionInvisibleTo = new KToggleAction(i18n("Always &Invisible To"), "", 0,
	                                        this, SLOT(slotInvisibleTo()), this, "actionInvisibleTo");

	bool on = account()->isConnected();
	if ( m_ssiItem.waitingAuth() )
		actionRequestAuth->setEnabled(on);
	else
		actionRequestAuth->setEnabled(false);

	actionSendAuth->setEnabled(on);


    m_selectEncoding = new KAction( i18n( "Select Encoding..." ), "charset", 0,
                                    this, SLOT( changeContactEncoding() ), this, "changeEncoding" );

/*
	actionReadAwayMessage->setEnabled(status != OSCAR_OFFLINE && status != OSCAR_ONLINE);
*/
	m_actionIgnore->setEnabled(on);
	m_actionVisibleTo->setEnabled(on);
	m_actionInvisibleTo->setEnabled(on);

	SSIManager* ssi = account()->engine()->ssiManager();
	m_actionIgnore->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_IGNORE ));
	m_actionVisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_VISIBLE ));
	m_actionInvisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_INVISIBLE ));

	actionCollection->append(actionRequestAuth);
	actionCollection->append(actionSendAuth);
    actionCollection->append( m_selectEncoding );

	actionCollection->append(m_actionIgnore);
	actionCollection->append(m_actionVisibleTo);
	actionCollection->append(m_actionInvisibleTo);

//	actionCollection->append(actionReadAwayMessage);

	return actionCollection;
}


void ICQContact::slotUserInfo()
{
	m_infoWidget = new ICQUserInfoWidget( Kopete::UI::Global::mainWidget(), "icq info" );
	QObject::connect( m_infoWidget, SIGNAL( finished() ), this, SLOT( closeUserInfoDialog() ) );
	m_infoWidget->setContact( this );
	m_infoWidget->show();
	if ( account()->isConnected() )
		mAccount->engine()->requestFullInfo( contactId() );
}

void ICQContact::closeUserInfoDialog()
{
	QObject::disconnect( this, 0, m_infoWidget, 0 );
	m_infoWidget->delayedDestruct();
	m_infoWidget = 0L;
}

void ICQContact::changeContactEncoding()
{
    if ( m_oesd )
        return;

    m_oesd = new OscarEncodingSelectionDialog( Kopete::UI::Global::mainWidget(), property(mProtocol->contactEncoding).value().toInt() );
    connect( m_oesd, SIGNAL( closing( int ) ),
             this, SLOT( changeEncodingDialogClosed( int ) ) );
    m_oesd->show();
}

void ICQContact::changeEncodingDialogClosed( int result )
{
	if ( result == QDialog::Accepted )
	{
		int mib = m_oesd->selectedEncoding();
		if ( mib != 0 )
		{
			kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "setting encoding mib to "
			                         << m_oesd->selectedEncoding() << endl;
			setProperty( mProtocol->contactEncoding, m_oesd->selectedEncoding() );
		}
		else
		{
			kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo
			                         << "setting encoding to default" << endl;
			removeProperty( mProtocol->contactEncoding );
		}
	}

    if ( m_oesd )
    {
        m_oesd->delayedDestruct();
        m_oesd = 0L;
    }
}


void ICQContact::slotIgnore()
{
	account()->engine()->setIgnore( contactId(), m_actionIgnore->isChecked() );
}

void ICQContact::slotVisibleTo()
{
	account()->engine()->setVisibleTo( contactId(), m_actionVisibleTo->isChecked() );
}

void ICQContact::slotInvisibleTo()
{
	account()->engine()->setInvisibleTo( contactId(), m_actionInvisibleTo->isChecked() );
}

#if 0

void ICQContact::slotReadAwayMessage()
{
	kdDebug(14153) << k_funcinfo << "account='" << account()->accountId() <<
		"', contact='" << displayName() << "'" << endl;

	if (!awayMessageDialog)
	{
		awayMessageDialog = new ICQReadAway(this, 0L, "awayMessageDialog");
		if(!awayMessageDialog)
			return;
		QObject::connect(awayMessageDialog, SIGNAL(closing()), this, SLOT(slotCloseAwayMessageDialog()));
		awayMessageDialog->show();
	}
	else
	{
		awayMessageDialog->raise();
	}
}


void ICQContact::slotCloseAwayMessageDialog()
{
	awayMessageDialog->delayedDestruct();
	awayMessageDialog = 0L;
}


const QString ICQContact::awayMessage()
{
	kdDebug(14150) << k_funcinfo <<  property(mProtocol->awayMessage).value().toString() << endl;
	return property(mProtocol->awayMessage).value().toString();
}


void ICQContact::setAwayMessage(const QString &message)
{
	/*kdDebug(14150) << k_funcinfo <<
		"Called for '" << displayName() << "', away msg='" << message << "'" << endl;*/
	setProperty(mProtocol->awayMessage, message);
	emit awayMessageChanged();
}


void ICQContact::slotUpdGeneralInfo(const int seq, const ICQGeneralUserInfo &inf)
{
	// compare reply's sequence with the one we sent with our last request
	if(seq != userinfoRequestSequence)
		return;
	generalInfo = inf;

	if(!generalInfo.firstName.isEmpty())
		setProperty(mProtocol->firstName, generalInfo.firstName);
	else
		removeProperty(mProtocol->firstName);

	if(!generalInfo.lastName.isEmpty())
		setProperty(mProtocol->lastName, generalInfo.lastName);
	else
		removeProperty(mProtocol->lastName);

	if(!generalInfo.eMail.isEmpty())
		setProperty(mProtocol->emailAddress, generalInfo.eMail);
	else
		removeProperty(mProtocol->emailAddress);
	/*
	if(!generalInfo.phoneNumber.isEmpty())
		setProperty("privPhoneNum", generalInfo.phoneNumber);
	else
		removeProperty("privPhoneNum");

	if(!generalInfo.faxNumber.isEmpty())
		setProperty("privFaxNum", generalInfo.faxNumber);
	else
		removeProperty("privFaxNum");

	if(!generalInfo.cellularNumber.isEmpty())
		setProperty("privMobileNum", generalInfo.cellularNumber);
	else
		removeProperty("privMobileNum");
	*/

	if(contactName() == displayName() && !generalInfo.nickName.isEmpty())
	{
		kdDebug(14153) << k_funcinfo << "setting new displayname for former UIN-only Contact" << endl;
		setDisplayName(generalInfo.nickName);
	}

	incUserInfoCounter();
}


void ICQContact::slotSnacFailed(WORD snacID)
{
	if (userinfoRequestSequence != 0)
		kdDebug(14153) << k_funcinfo << "snacID = " << snacID << " seq = " << userinfoRequestSequence << endl;

	//TODO: ugly interaction between snacID and request sequence, see OscarSocket::sendCLI_TOICQSRV
	if (snacID == (0x0000 << 16) | userinfoRequestSequence)
	{
		userinfoRequestSequence = 0;
		emit userInfoRequestFailed();
	}
}

void ICQContact::slotIgnore()
{
	kdDebug(14150) << k_funcinfo <<
		"Called; ignore = " << actionIgnore->isChecked() << endl;
	setIgnore(actionIgnore->isChecked(), true);
}

void ICQContact::slotVisibleTo()
{
	kdDebug(14150) << k_funcinfo <<
		"Called; visible = " << actionVisibleTo->isChecked() << endl;
	setVisibleTo(actionVisibleTo->isChecked(), true);
}
#endif
#include "icqcontact.moc"
//kate: indent-mode csands; tab-width 4; replace-tabs off; space-indent off;
