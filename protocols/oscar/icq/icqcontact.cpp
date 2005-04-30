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

#include "kopetemessagemanagerfactory.h"
#include "kopeteuiglobal.h"

#include "icquserinfo.h"
#include "icqreadaway.h"
#include "icqprotocol.h"
#include "icqaccount.h"
#include "icqpresence.h"
#include "icquserinfowidget.h"
#include "icqauthreplydialog.h"

#include "oscarutils.h"

ICQContact::ICQContact( ICQAccount *account, const QString &name, Kopete::MetaContact *parent,
						const QString& icon, const Oscar::SSI& ssiItem )
: OscarContact( account, name, parent, icon, ssiItem )
{
	mProtocol = static_cast<ICQProtocol *>(protocol());
	m_infoWidget = 0L;
	
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

	kdDebug( OSCAR_ICQ_DEBUG ) << k_funcinfo << "extendedStatus is " << details.extendedStatus() << endl;
	ICQ::Presence presence = ICQ::Presence::fromOscarStatus( details.extendedStatus() & 0xffff );
	setOnlineStatus( presence.toOnlineStatus() );
	
	if ( details.clientName().isEmpty() )
		removeProperty( mProtocol->clientFeatures );
	else
		setProperty( mProtocol->clientFeatures, details.clientName() );
	
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
	if ( m_ssiItem.waitingAuth() )
		setOnlineStatus( mProtocol->statusManager()->waitingForAuth() );

	QString nickname = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if ( nickname.isEmpty() || Oscar::normalize( nickname ) == Oscar::normalize( contactId() ) )
	{
		int time = ( KApplication::random() % 25 ) * 1000;
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
	
	ICQAuthReplyDialog replyDialog;
	
	replyDialog.setUser( property( Kopete::Global::Properties::self()->nickName() ).value().toString() );
	replyDialog.setRequestReason( reason );
	if ( replyDialog.exec() )
		mAccount->engine()->sendAuth( contactId(), replyDialog.reason(), replyDialog.grantAuth() );
}

void ICQContact::receivedLongInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
	{
		if ( m_infoWidget )
			m_infoWidget->delayedDestruct();
		return;
	}
	
	kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "received long info from engine" << endl;
	
	ICQGeneralUserInfo genInfo = mAccount->engine()->getGeneralInfo( contact );
	emit haveBasicInfo( genInfo );
	
	ICQWorkUserInfo workInfo = mAccount->engine()->getWorkInfo( contact );
	emit haveWorkInfo( workInfo );
	
	ICQMoreUserInfo moreInfo = mAccount->engine()->getMoreInfo( contact );
	emit haveMoreInfo( moreInfo );
	
}

void ICQContact::receivedShortInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;
	
	ICQShortInfo shortInfo = mAccount->engine()->getShortInfo( contact );
	/*
	if(!shortInfo.firstName.isEmpty())
		setProperty(mProtocol->firstName, shortInfo.firstName);
	else
		removeProperty(mProtocol->firstName);
	
	if(!shortInfo.lastName.isEmpty())
		setProperty(mProtocol->lastName, shortInfo.lastName);
	else
		removeProperty(mProtocol->lastName);
	*/
	if ( !shortInfo.nickname.isEmpty() )
	{
		kdDebug(14153) << k_funcinfo <<
			"setting new displayname for former UIN-only Contact" << endl;
		setProperty( Kopete::Global::Properties::self()->nickName(), shortInfo.nickname );
	}
	
}

void ICQContact::slotSendMsg( Kopete::Message& msg, Kopete::ChatSession* session )
{
	//Why is this unused?
	Q_UNUSED( session );
	Oscar::Message message;
	
	message.setText( msg.plainBody() );
	
	message.setTimestamp( msg.timestamp() );
	message.setSender( mAccount->accountId() );
	message.setReceiver( mName );
	message.setType( 0x01 );
	
	//TODO add support for type 2 messages
	/*if ( msg.type() == Kopete::Message::PlainText )
		message.setType( 0x01 );
	else
		message.setType( 0x02 );*/
	//TODO: we need to check for channel 0x04 messages too;
	
	mAccount->engine()->sendMessage( message );
	manager(Kopete::Contact::CanCreate)->appendMessage(msg);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
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
		actionIgnore = new KToggleAction(i18n("&Ignore"), "", 0,
			this, SLOT(slotIgnore()), this, "actionIgnore");
		actionVisibleTo = new KToggleAction(i18n("Always &Visible To"), "", 0,
			this, SLOT(slotVisibleTo()), this, "actionVisibleTo");
		actionInvisibleTo = new KToggleAction(i18n("Always &Invisible To"), "", 0,
			this, SLOT(slotInvisibleTo()), this, "actionInvisibleTo");
	}
	else
	{
		actionReadAwayMessage->setText(awTxt);
		actionReadAwayMessage->setIconSet(SmallIconSet(awIcn));
	}

*/
	QString i1 = i18n("&Ignore");
	QString i2 = i18n("Always &Visible To");
	QString i3 = i18n("Always &Invisible To");

	Q_UNUSED( i1 );
	Q_UNUSED( i2 );
	Q_UNUSED( i3 );
		
	
	bool on = account()->isConnected();
	if ( m_ssiItem.waitingAuth() )
		actionRequestAuth->setEnabled(on);
	else
		actionRequestAuth->setEnabled(false);
	
	actionSendAuth->setEnabled(on);
/*
	actionReadAwayMessage->setEnabled(status != OSCAR_OFFLINE && status != OSCAR_ONLINE);
	actionIgnore->setEnabled(on);
	actionVisibleTo->setEnabled(on);
	actionInvisibleTo->setEnabled(on);

	actionIgnore->setChecked(mIgnore);
	actionVisibleTo->setChecked(mVisibleTo);
	actionInvisibleTo->setChecked(mInvisibleTo);

*/
	actionCollection->append(actionRequestAuth);
	actionCollection->append(actionSendAuth);
/*
	actionCollection->append(actionIgnore);
	actionCollection->append(actionVisibleTo);
	actionCollection->append(actionInvisibleTo);
	actionCollection->append(actionReadAwayMessage);
*/
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
