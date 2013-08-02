/*
  icqontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003      by Stefan Gehn  <metz@gehn.net>
  Copyright (c) 2003      by Olivier Goffart <oggoffart@kde.org>
  Copyright (c) 2006,2007 by Roman Jarosz <kedgedev@centrum.cz>

  Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

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

#include <QPointer>
#include <qtimer.h>
#include <KActionCollection>
#include <klocale.h>
#include <knotification.h>
#include <kinputdialog.h>
#include <krandom.h>
#include <ktoggleaction.h>
#include <kicon.h>

#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"

#include "icqprotocol.h"
#include "icqaccount.h"
#include "icquserinfowidget.h"
#include "icqauthreplydialog.h"

#include "oscarutils.h"
#include "contactmanager.h"
#include "oscarstatusmanager.h"


ICQContact::ICQContact( Kopete::Account* account, const QString &name, Kopete::MetaContact *parent,
						const QString& icon )
: ICQContactBase( account, name, parent, icon )
{
	m_requestingInfo = InfoNone;
	mProtocol = static_cast<ICQProtocol *>(protocol());
	m_infoWidget = 0L;

	setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline ) );

	QObject::connect( mAccount->engine(), SIGNAL(loggedIn()), this, SLOT(loggedIn()) );
	//QObject::connect( mAccount->engine(), SIGNAL(userIsOnline(QString)), this, SLOT(userOnline(QString,UserDetails)) );
	QObject::connect( mAccount->engine(), SIGNAL(userIsOffline(QString)), this, SLOT(userOffline(QString)) );
	QObject::connect( mAccount->engine(), SIGNAL(authReplyReceived(QString,QString,bool)),
	                  this, SLOT(slotGotAuthReply(QString,QString,bool)) );
	QObject::connect( mAccount->engine(), SIGNAL(receivedIcqShortInfo(QString)),
	                  this, SLOT(receivedShortInfo(QString)) );
	QObject::connect( mAccount->engine(), SIGNAL(receivedIcqLongInfo(QString)),
	                  this, SLOT(receivedLongInfo(QString)) );
	QObject::connect( mAccount->engine(), SIGNAL(receivedUserInfo(QString,UserDetails)),
	                  this, SLOT(userInfoUpdated(QString,UserDetails)) );
	QObject::connect( mAccount->engine(), SIGNAL(receivedIcqTlvInfo(QString)),
	                  this, SLOT(receivedTlvInfo(QString)) );
}

ICQContact::~ICQContact()
{
	delete m_infoWidget;
}

void ICQContact::setSSIItem( const OContact& ssiItem )
{
	if ( ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );

	if ( ssiItem.type() != 0xFFFF && ssiItem.waitingAuth() == false &&
	     onlineStatus().status() == Kopete::OnlineStatus::Unknown )
	{
		//make sure they're offline
		setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline ) );
	}

	if ( mAccount->engine()->isActive() && m_ssiItem.metaInfoId() != ssiItem.metaInfoId() )
	{
		// User info has changed, check nickname or status description if needed.
		// If mAccount->isConnected() is false then the metaInfoId has changed while
		// we were offline and we don't know how many users changed its info, so better
		// delay the request.
		if ( mAccount->isConnected() )
			QTimer::singleShot( 0, this, SLOT(requestMediumTlvInfo()) );
		else
			requestMediumTlvInfoDelayed();
	}

	ICQContactBase::setSSIItem( ssiItem );
}

void ICQContact::setEncoding( int mib )
{
	ICQContactBase::setEncoding( mib );
	QTimer::singleShot( 0, this, SLOT(requestShortInfo()) );
}

void ICQContact::userInfoUpdated( const QString& contact, const UserDetails& details )
{
	if ( Oscar::normalize( contact  ) != Oscar::normalize( contactId() ) )
		return;

	// invalidate old away message if user was offline
	if ( !isOnline() )
	{
		removeProperty( mProtocol->statusTitle );
		removeProperty( mProtocol->statusMessage );
	}

	kDebug( OSCAR_ICQ_DEBUG ) << "extendedStatus is " << details.extendedStatus();
	Oscar::Presence presence = mProtocol->statusManager()->presenceOf( details.extendedStatus(), details.userClass() );

	if ( details.dcOutsideSpecified() )
		setProperty( mProtocol->ipAddress, details.dcExternalIp().toString() );

	if ( details.capabilitiesSpecified() )
		setProperty( mProtocol->clientFeatures, details.clientName() );

	OscarContact::userInfoUpdated( contact, details );

	refreshStatus( m_details, presence );
}

void ICQContact::refreshStatus( const UserDetails& details, Oscar::Presence presence )
{
	// Filter our XStatus and ExtStatus
	presence.setFlags( presence.flags() & ~Oscar::Presence::StatusTypeMask );

	if ( details.statusMood() != -1 )
	{
		presence.setFlags( presence.flags() | Oscar::Presence::ExtStatus2 );
		presence.setMood( details.statusMood() );

		Kopete::StatusMessage msg;
		msg.setTitle( details.personalMessage() );
		setStatusMessage( msg );
	}
	// XStatus don't support offline status so don't show it (xtrazStatusSpecified can be true if contact was online)
	else if ( details.xtrazStatus() != -1 && presence.type() != Oscar::Presence::Offline )
	{
		presence.setFlags( presence.flags() | Oscar::Presence::XStatus );
		presence.setXtrazStatus( details.xtrazStatus() );

		Kopete::StatusMessage msg;
		msg.setTitle( details.personalMessage() );
		setStatusMessage( msg );
	}
	else if ( !details.personalMessage().isEmpty() )
	{
		presence.setFlags( presence.flags() | Oscar::Presence::ExtStatus );

		Kopete::StatusMessage msg;
		msg.setTitle( details.personalMessage() );
		setStatusMessage( msg );
	}
	else
	{
		Kopete::StatusMessage msg;
		setStatusMessage( msg ); // set an empty status message
	}

	setPresenceTarget( presence );

	Oscar::Presence selfPres( mProtocol->statusManager()->presenceOf( account()->myself()->onlineStatus() ) );
	bool selfVisible = !(selfPres.flags() & Oscar::Presence::Invisible);

	if ( selfVisible && isReachable() && presence.type() != Oscar::Presence::Offline )
	{
		Client::ICQStatus contactStatus = Client::ICQOnline;
		if ( details.xtrazStatus() != -1 )
		{
			contactStatus = Client::ICQXStatus;
		}
		else
		{
			switch ( presence.type() )
			{
			case Oscar::Presence::Online:
				contactStatus = Client::ICQOnline;
				break;
			case Oscar::Presence::Away:
				contactStatus = Client::ICQAway;
				break;
			case Oscar::Presence::NotAvailable:
				contactStatus = Client::ICQNotAvailable;
				break;
			case Oscar::Presence::Occupied:
				contactStatus = Client::ICQOccupied;
				break;
			case Oscar::Presence::DoNotDisturb:
				contactStatus = Client::ICQDoNotDisturb;
				break;
			case Oscar::Presence::FreeForChat:
				contactStatus = Client::ICQFreeForChat;
				break;
			default:
				break;
			}
		}

		// FIXME: How can we check if client supports status plugin messages?
		if ( details.onlineStatusMsgSupport() )
			contactStatus |= Client::ICQPluginStatus;

		// If contact is online and doesn't support status plugin messages than
		// this contact can't have online status message.
		if ( contactStatus == Client::ICQOnline && !details.onlineStatusMsgSupport() )
		{
			mAccount->engine()->removeICQAwayMessageRequest( contactId() );
			removeProperty( mProtocol->statusMessage );
		}
		else
		{
			mAccount->engine()->addICQAwayMessageRequest( contactId(), contactStatus );
		}
	}
	else
	{
		mAccount->engine()->removeICQAwayMessageRequest( contactId() );
	}
}

void ICQContact::userOnline( const QString& userId )
{
	if ( Oscar::normalize( userId ) != Oscar::normalize( contactId() ) )
		return;

	kDebug(OSCAR_ICQ_DEBUG) << "Setting " << userId << " online";
	setPresenceTarget( Oscar::Presence( Oscar::Presence::Online ) );
}

void ICQContact::userOffline( const QString& userId )
{
	if ( Oscar::normalize( userId ) != Oscar::normalize( contactId() ) )
		return;

	m_details.clear();

	kDebug(OSCAR_ICQ_DEBUG) << "Setting " << userId << " offline";
	if ( m_ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );
	else
		refreshStatus( m_details, Oscar::Presence( Oscar::Presence::Offline ) );

	removeProperty( mProtocol->statusTitle );
	removeProperty( mProtocol->statusMessage );
}

void ICQContact::loggedIn()
{
	if ( metaContact()->isTemporary() )
		return;

	if ( m_ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );

	requestShortInfoDelayed();
}

void ICQContact::slotRequestAuth()
{
	QString reason = KInputDialog::getText( i18n("Request Authorization"),
	                                        i18n("Reason for requesting authorization:"),
	                                        i18n("Please authorize me so I can add you to my contact list") );
	if ( !reason.isNull() )
		mAccount->engine()->requestAuth( contactId(), reason );
}

void ICQContact::slotSendAuth()
{
	kDebug(OSCAR_ICQ_DEBUG) << "Sending auth reply";
	QPointer <ICQAuthReplyDialog> replyDialog = new ICQAuthReplyDialog( 0, false );

	replyDialog->setUser( displayName() );
	if ( replyDialog->exec() && replyDialog )
		mAccount->engine()->sendAuth( contactId(), replyDialog->reason(), replyDialog->grantAuth() );
	delete replyDialog;
}

void ICQContact::slotGotAuthReply( const QString& contact, const QString& reason, bool granted )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	if ( account()->isBusy() )
		return;

	kDebug(OSCAR_ICQ_DEBUG) ;
	QString message;
	if( granted )
	{
		message = i18n( "User %1 has granted your authorization request.\nReason: %2" ,
			  displayName() ,
			  reason );

		// remove the unknown status
		setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline ) );
	}
	else
	{
		message = i18n( "User %1 has rejected the authorization request.\nReason: %2" ,
			  displayName() ,
			  reason );
	}
	KNotification::event( QString::fromLatin1("icq_authorization"), message );
}

void ICQContact::requestShortInfo()
{
	kDebug(OSCAR_ICQ_DEBUG) << "requesting short info for " << contactId();
	if ( mAccount->engine()->isActive() )
		mAccount->engine()->requestShortInfo( contactId() );

	// Don't clear m_requestingInfo if info with higher contents was requested
	if ( m_requestingInfo <= InfoShort )
		m_requestingInfo = InfoNone;
}

void ICQContact::receivedShortInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	QTextCodec* codec = contactCodec();

	ICQShortInfo shortInfo = mAccount->engine()->getShortInfo( contact );

	setProperty( mProtocol->firstName, codec->toUnicode( shortInfo.firstName ) );
	setProperty( mProtocol->lastName, codec->toUnicode( shortInfo.lastName ) );
	setNickName( codec->toUnicode( shortInfo.nickname ) );
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

	kDebug(OSCAR_ICQ_DEBUG) << "received long info from engine";

	ICQGeneralUserInfo genInfo = mAccount->engine()->getGeneralInfo( contact );

	setProperty( mProtocol->firstName, codec->toUnicode( genInfo.firstName.get() ) );
	setProperty( mProtocol->lastName, codec->toUnicode( genInfo.lastName.get() ) );
	setNickName( codec->toUnicode( genInfo.nickName.get() ) );

	emit haveBasicInfo( genInfo );

	ICQWorkUserInfo workInfo = mAccount->engine()->getWorkInfo( contact );
	emit haveWorkInfo( workInfo );

	ICQEmailInfo emailInfo = mAccount->engine()->getEmailInfo( contact );
	emit haveEmailInfo( emailInfo );

	ICQNotesInfo notesInfo = mAccount->engine()->getNotesInfo( contact );
	emit haveNotesInfo( notesInfo );

	ICQMoreUserInfo moreInfo = mAccount->engine()->getMoreInfo( contact );
	emit haveMoreInfo( moreInfo );

	ICQInterestInfo interestInfo = mAccount->engine()->getInterestInfo( contact );
	emit haveInterestInfo( interestInfo );

	ICQOrgAffInfo orgAffInfo = mAccount->engine()->getOrgAffInfo( contact );
	emit haveOrgAffInfo( orgAffInfo );
}

void ICQContact::requestMediumTlvInfo()
{
	kDebug(OSCAR_ICQ_DEBUG) << "requesting medium tlv info for " << contactId();
	if ( mAccount->engine()->isActive() && !m_ssiItem.metaInfoId().isEmpty() )
		mAccount->engine()->requestMediumTlvInfo( contactId(), m_ssiItem.metaInfoId() );

	// Don't clear m_requestingInfo if info with higher contents was requested
	if ( m_requestingInfo <= InfoMediumTlv )
		m_requestingInfo = InfoNone;
}

void ICQContact::receivedTlvInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	ICQFullInfo info = mAccount->engine()->getFullInfo( contact );

	setProperty( mProtocol->firstName, QString::fromUtf8( info.firstName.get() ) );
	setProperty( mProtocol->lastName, QString::fromUtf8( info.lastName.get() ) );
	setNickName( QString::fromUtf8( info.nickName.get() ) );
}

void ICQContact::requestShortInfoDelayed( int minDelay )
{
	if ( mAccount->engine()->isActive() && m_requestingInfo < InfoShort )
	{
		m_requestingInfo = InfoShort;
		int time = ( KRandom::random() % 20 ) * 1000 + minDelay;
		kDebug(OSCAR_ICQ_DEBUG) << "requesting info in " << time/1000 << " seconds";
		QTimer::singleShot( time, this, SLOT(infoDelayTimeout()) );
	}
}

void ICQContact::requestMediumTlvInfoDelayed( int minDelay )
{
	if ( mAccount->engine()->isActive() && m_requestingInfo < InfoMediumTlv )
	{
		m_requestingInfo = InfoMediumTlv;
		int time = ( KRandom::random() % 20 ) * 1000 + minDelay;
		kDebug(OSCAR_ICQ_DEBUG) << "requesting info in " << time/1000 << " seconds";
		QTimer::singleShot( time, this, SLOT(infoDelayTimeout()) );
	}
}

void ICQContact::infoDelayTimeout()
{
	if ( m_requestingInfo == InfoMediumTlv )
		requestMediumTlvInfo();
	else if ( m_requestingInfo == InfoShort )
		requestShortInfo();
}

#if 0
void ICQContact::slotContactChanged(const UserInfo &u)
{
	if (u.sn != contactName())
		return;

	// update mInfo and general stuff from OscarContact
	slotParseUserInfo(u);

	/*kDebug(14190) << "Called for '"
		<< displayName() << "', contactName()=" << contactName() << endl;*/
	QStringList capList;
	// Append client name and version in case we found one
	if (!mInfo.clientName.isEmpty())
	{
		if (!mInfo.clientVersion.isEmpty())
		{
			capList << i18nc("Translators: client-name client-version",
				"%1 %2", mInfo.clientName, mInfo.clientVersion);
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
				removeProperty(mProtocol->statusMessage);
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
	removeProperty(mProtocol->statusMessage);
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

QList<KAction*> *ICQContact::customContextMenuActions()
{
	QList<KAction*> *actions = new QList<KAction*>();

	actionRequestAuth = new KAction( i18n("&Request Authorization"), this );
        //, "actionRequestAuth");
	actionRequestAuth->setIcon( KIcon( "mail-reply-sender" ) );
	QObject::connect( actionRequestAuth, SIGNAL(triggered(bool)), this, SLOT(slotRequestAuth()) );

	actionSendAuth = new KAction( i18n("&Grant Authorization"), this );
        //, "actionSendAuth");
	actionSendAuth->setIcon( KIcon( "mail-forward" ) );
	QObject::connect( actionSendAuth, SIGNAL(triggered(bool)), this, SLOT(slotSendAuth()) );

	m_actionIgnore = new KToggleAction(i18n("&Ignore"), this );
        //, "actionIgnore");
	QObject::connect( m_actionIgnore, SIGNAL(triggered(bool)), this, SLOT(slotIgnore()) );

	m_actionVisibleTo = new KToggleAction(i18n("Always &Visible To"), this );
        //, "actionVisibleTo");
	QObject::connect( m_actionVisibleTo, SIGNAL(triggered(bool)), this, SLOT(slotVisibleTo()) );

	m_actionInvisibleTo = new KToggleAction(i18n("Always &Invisible To"), this );
        //, "actionInvisibleTo");
	QObject::connect( m_actionInvisibleTo, SIGNAL(triggered(bool)), this, SLOT(slotInvisibleTo()) );

	m_selectEncoding = new KAction( i18n( "Select Encoding..." ), this );
        //, "changeEncoding" );
	m_selectEncoding->setIcon( KIcon( "character-set" ) );
	QObject::connect( m_selectEncoding, SIGNAL(triggered(bool)), this, SLOT(changeContactEncoding()) );

	bool on = account()->isConnected();
	if ( m_ssiItem.waitingAuth() )
		actionRequestAuth->setEnabled(on);
	else
		actionRequestAuth->setEnabled(false);

	actionSendAuth->setEnabled(on);
	m_actionIgnore->setEnabled(on);
	m_actionVisibleTo->setEnabled(on);
	m_actionInvisibleTo->setEnabled(on);

	ContactManager* ssi = account()->engine()->ssiManager();
	m_actionIgnore->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_IGNORE ));
	m_actionVisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_VISIBLE ));
	m_actionInvisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_INVISIBLE ));

	actions->append(actionRequestAuth);
	actions->append(actionSendAuth);
    actions->append( m_selectEncoding );

	actions->append(m_actionIgnore);
	actions->append(m_actionVisibleTo);
	actions->append(m_actionInvisibleTo);

	// temporary action collection, used to apply Kiosk policy to the actions
	KActionCollection tempCollection((QObject*)0);
	tempCollection.addAction(QLatin1String("contactRequestAuth"), actionRequestAuth);
	tempCollection.addAction(QLatin1String("contactSendAuth"), actionSendAuth);
	tempCollection.addAction(QLatin1String("contactSelectEncoding"), m_selectEncoding);
	tempCollection.addAction(QLatin1String("contactIgnore"), m_actionIgnore);
	tempCollection.addAction(QLatin1String("oscarContactAlwaysVisibleTo"), m_actionVisibleTo);
	tempCollection.addAction(QLatin1String("oscarContactAlwaysInvisibleTo"), m_actionInvisibleTo);
	return actions;
}


void ICQContact::slotUserInfo()
{
	if ( m_infoWidget )
	{
		m_infoWidget->showNormal();
		m_infoWidget->activateWindow();
		return;
	}

	m_infoWidget = new ICQUserInfoWidget( this, Kopete::UI::Global::mainWidget() );
	QObject::connect( m_infoWidget, SIGNAL(finished()), this, SLOT(closeUserInfoDialog()) );
	QObject::connect( m_infoWidget, SIGNAL(okClicked()), this, SLOT(storeUserInfoDialog()) );
	m_infoWidget->show();
}

void ICQContact::storeUserInfoDialog()
{
	QString alias = m_infoWidget->getAlias();
	if ( alias.isEmpty() )
		requestShortInfoDelayed( 5000 );
	else
		mAccount->engine()->changeContactAlias( contactId(), alias );
}

void ICQContact::closeUserInfoDialog()
{
	QObject::disconnect( this, 0, m_infoWidget, 0 );
	m_infoWidget->delayedDestruct();
	m_infoWidget = 0L;
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
	kDebug(14153) << "account='" << account()->accountId() <<
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
	kDebug(14150) <<  property(mProtocol->statusMessage).value().toString();
	return property(mProtocol->statusMessage).value().toString();
}


void ICQContact::setAwayMessage(const QString &message)
{
	/*kDebug(14150) <<
		"Called for '" << displayName() << "', away msg='" << message << "'" << endl;*/
	setProperty(mProtocol->statusMessage, message);
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

	setDisplayName(generalInfo.nickName);

	incUserInfoCounter();
}


void ICQContact::slotSnacFailed(WORD snacID)
{
	if (userinfoRequestSequence != 0)
		kDebug(14153) << "snacID = " << snacID << " seq = " << userinfoRequestSequence;

	//TODO: ugly interaction between snacID and request sequence, see OscarSocket::sendCLI_TOICQSRV
	if (snacID == (0x0000 << 16) | userinfoRequestSequence)
	{
		userinfoRequestSequence = 0;
		emit userInfoRequestFailed();
	}
}

void ICQContact::slotIgnore()
{
	kDebug(14150) <<
		"Called; ignore = " << actionIgnore->isChecked() << endl;
	setIgnore(actionIgnore->isChecked(), true);
}

void ICQContact::slotVisibleTo()
{
	kDebug(14150) <<
		"Called; visible = " << actionVisibleTo->isChecked() << endl;
	setVisibleTo(actionVisibleTo->isChecked(), true);
}
#endif
#include "icqcontact.moc"
//kate: indent-mode csands; tab-width 4; replace-tabs off; space-indent off;
