/*
  icqaccount.cpp  -  ICQ Account Class

  Copyright (c) 2002 by Chris TenHarmsel            <tenharmsel@staticmethod.net>
  Copyright (c) 2004 by Richard Smith               <kde@metafoo.co.uk>
  Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kicon.h>
#include <knotification.h>
#include <ktoggleaction.h>

#include  <QtGui/QTextDocument> // Qt::escape

#include "kopetemessage.h"
#include "kopetecontactlist.h"
#include "kopeteuiglobal.h"

#include "client.h"
#include "icquserinfo.h"
#include "oscarsettings.h"
#include "oscarutils.h"
#include "contactmanager.h"

#include "icqcontact.h"
#include "aimcontact.h"
#include "icqprotocol.h"
#include "icqaccount.h"
#include "icquserinfowidget.h"
#include "oscarstatusmanager.h"
#include "oscarpresencesdataclasses.h"
#include "xtrazicqstatusdialog.h"
#include "xtrazicqstatuseditor.h"
#include "xtrazstatusaction.h"
#include "icqstatusmanager.h"
#include "icqauthreplydialog.h"

ICQMyselfContact::ICQMyselfContact( ICQAccount *acct ) : OscarMyselfContact( acct )
{
	QObject::connect( acct->engine(), SIGNAL( loggedIn() ), this, SLOT( fetchShortInfo() ) );
	QObject::connect( acct->engine(), SIGNAL( receivedIcqShortInfo( const QString& ) ),
	                  this, SLOT( receivedShortInfo( const QString& ) ) );
}

void ICQMyselfContact::userInfoUpdated()
{
	Oscar::DWORD extendedStatus = details().extendedStatus();
	kDebug( OSCAR_ICQ_DEBUG ) << "extendedStatus is " << QString::number( extendedStatus, 16 );

	ICQProtocol* p = static_cast<ICQProtocol *>(protocol());
	Oscar::Presence presence = p->statusManager()->presenceOf( extendedStatus, details().userClass() );

	ICQAccount* icqAccount = static_cast<ICQAccount*>( account() );
	if ( details().xtrazStatus() != -1 )
	{
		presence.setFlags( presence.flags() | Oscar::Presence::XStatus );
		presence.setDescription( icqAccount->engine()->statusDescription() );
		presence.setXtrazStatus( details().xtrazStatus() );
	}
	else if ( !icqAccount->engine()->statusDescription().isEmpty() )
	{
		presence.setFlags( presence.flags() | Oscar::Presence::ExtStatus );
		presence.setDescription( icqAccount->engine()->statusDescription() );
	}

	setProperty( Kopete::Global::Properties::self()->statusMessage(), icqAccount->engine()->statusMessage() );
	setOnlineStatus( p->statusManager()->onlineStatusOf( presence ) );
}

void ICQMyselfContact::receivedShortInfo( const QString& contact )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	ICQAccount* icqAccount = static_cast<ICQAccount*>( account() );
	ICQShortInfo shortInfo = icqAccount->engine()->getShortInfo( contact );
	if ( !shortInfo.nickname.isEmpty() )
		setProperty( Kopete::Global::Properties::self()->nickName(), icqAccount->defaultCodec()->toUnicode( shortInfo.nickname ) );

	//Sync server settings with local
	QList<ICQInfoBase*> infoList;

	ICQShortInfo* info = new ICQShortInfo( shortInfo );

	Oscar::Settings* oscarSettings = icqAccount->engine()->clientSettings();
	info->needsAuth.set( oscarSettings->requireAuth() );
	info->webAware.set( oscarSettings->webAware() );

	infoList.append( info );
	if ( !icqAccount->engine()->updateProfile( infoList ) )
		qDeleteAll( infoList );
}

void ICQMyselfContact::fetchShortInfo()
{
	static_cast<ICQAccount*>( account() )->engine()->requestShortInfo( contactId() );
}

ICQAccount::ICQAccount(Kopete::Protocol *parent, QString accountID)
	: OscarAccount(parent, accountID, true)
{
	kDebug(14152) << accountID << ": Called.";
	setMyself( new ICQMyselfContact( this ) );
	myself()->setOnlineStatus( protocol()->statusManager()->onlineStatusOf( Oscar::Presence( Oscar::Presence::Offline ) ) );

	QString nickName = configGroup()->readEntry("NickName", QString() );
	mWebAware = configGroup()->readEntry( "WebAware", false );
	mHideIP = configGroup()->readEntry( "HideIP", true );
	mInfoContact = 0L;
	mInfoWidget = 0L;
	mInitialStatusMessage.clear();

	QObject::connect( engine(), SIGNAL(userReadsStatusMessage(const QString&)),
	                  this, SLOT(userReadsStatusMessage(const QString&)) );
	QObject::connect( engine(), SIGNAL( authRequestReceived( const QString&, const QString& ) ),
	                  this, SLOT( slotGotAuthRequest( const QString&, const QString& ) ) );

	// Create actions
	mEditInfoAction = new KAction( KIcon("identity"), i18n( "Edit User Info..." ), this );
	QObject::connect( mEditInfoAction, SIGNAL(triggered(bool)), this, SLOT(slotUserInfo()) );
	
	mActionInvisible = new KToggleAction( i18n( "In&visible" ), this );
	QObject::connect( mActionInvisible, SIGNAL(triggered(bool)), this, SLOT(slotToggleInvisible()) );

	//setIgnoreUnknownContacts(pluginData(protocol(), "IgnoreUnknownContacts").toUInt() == 1);

	/* FIXME: need to do this when web aware or hide ip change
	if(isConnected() && (oldhideip != mHideIP || oldwebaware != mWebAware))
	{
		kDebug(14153) <<
			"sending status to reflect HideIP and WebAware settings" << endl;
		//setStatus(mStatus, QString());
	}*/
}

ICQAccount::~ICQAccount()
{
}

ICQProtocol* ICQAccount::protocol()
{
	return static_cast<ICQProtocol*>(OscarAccount::protocol());
}


Oscar::Presence ICQAccount::presence()
{
	return protocol()->statusManager()->presenceOf( myself()->onlineStatus() );
}


KActionMenu* ICQAccount::actionMenu()
{
	KActionMenu* actionMenu = Kopete::Account::actionMenu();

	actionMenu->addSeparator();

	actionMenu->addAction( mEditInfoAction );

	Oscar::Presence pres( presence().type(), presence().flags() | Oscar::Presence::Invisible );
	pres.setXtrazStatus( presence().xtrazStatus() );
	mActionInvisible->setIcon( KIcon( protocol()->statusManager()->onlineStatusOf( pres ).iconFor( this ) ) );
	mActionInvisible->setChecked( (presence().flags() & Oscar::Presence::Invisible) == Oscar::Presence::Invisible );
	actionMenu->addAction( mActionInvisible );

	/*
	actionMenu->popupMenu()->insertSeparator();
	//actionMenu->insert( new KToggleAction( i18n( "Send &SMS..." ), 0, 0, this, SLOT( slotSendSMS() ), this, "ICQAccount::mActionSendSMS") );
	*/

	KActionMenu *xtrazStatusMenu = new KActionMenu( i18n( "Set Xtraz Status" ), actionMenu );
	
	KAction* xtrazStatusSetAction = new KAction( i18n( "Set Status..." ), xtrazStatusMenu );
	QObject::connect( xtrazStatusSetAction, SIGNAL(triggered(bool)), this, SLOT(setXtrazStatus()) );
	xtrazStatusMenu->addAction( xtrazStatusSetAction );

	KAction* xtrazStatusEditAction = new KAction( i18n( "Edit Statuses..." ), xtrazStatusMenu );
	QObject::connect( xtrazStatusEditAction, SIGNAL(triggered(bool)), this, SLOT(editXtrazStatuses()) );
	xtrazStatusMenu->addAction( xtrazStatusEditAction );

	ICQStatusManager* mgr = static_cast<ICQStatusManager *>(protocol()->statusManager());
	QList<Xtraz::Status> xtrazStatusList = mgr->xtrazStatuses();

	if ( !xtrazStatusList.isEmpty() )
		xtrazStatusMenu->addSeparator();

	for ( int i = 0; i < xtrazStatusList.count(); i++ )
	{
		Xtraz::StatusAction* xtrazAction = new Xtraz::StatusAction( xtrazStatusList.at(i), xtrazStatusMenu );
		QObject::connect( xtrazAction, SIGNAL(triggered(const Oscar::Presence&, const QString&)),
		                  this, SLOT(setPresenceTarget(const Oscar::Presence&, const QString&)) );
		xtrazStatusMenu->addAction( xtrazAction );
	}

	actionMenu->addAction( xtrazStatusMenu );

	return actionMenu;
}


void ICQAccount::connectWithPassword( const QString &password )
{
	if ( password.isNull() )
		return;

	kDebug(14153) << "accountId='" << accountId() << "'";

	Kopete::OnlineStatus status = initialStatus();
	if ( status == Kopete::OnlineStatus() && status.status() == Kopete::OnlineStatus::Unknown )
		//use default online in case of invalid online status for connecting
		status = Kopete::OnlineStatus( Kopete::OnlineStatus::Online );

	Oscar::Presence pres = protocol()->statusManager()->presenceOf( status );
	bool accountIsOffline = ( presence().type() == Oscar::Presence::Offline ||
	                          myself()->onlineStatus() == protocol()->statusManager()->connectingStatus() );

	if ( accountIsOffline )
	{
		myself()->setOnlineStatus( protocol()->statusManager()->connectingStatus() );
		QString icqNumber = accountId();
		kDebug(14153) << "Logging in as " << icqNumber;
		QString server = configGroup()->readEntry( "Server", QString::fromLatin1( "login.oscar.aol.com" ) );
		uint port = configGroup()->readEntry( "Port", 5190 );

		//set up the settings for the account
		Oscar::Settings* oscarSettings = engine()->clientSettings();
		oscarSettings->setWebAware( configGroup()->readEntry( "WebAware", false ) );
		oscarSettings->setHideIP( configGroup()->readEntry( "HideIP", true ) );
		oscarSettings->setRequireAuth( configGroup()->readEntry( "RequireAuth", false ) );
		oscarSettings->setRespectRequireAuth( configGroup()->readEntry( "RespectRequireAuth", true ) );
		oscarSettings->setFileProxy( configGroup()->readEntry( "FileProxy", false ) );
		oscarSettings->setFirstPort( configGroup()->readEntry( "FirstPort", 5190 ) );
		oscarSettings->setLastPort( configGroup()->readEntry( "LastPort", 5199 ) );
		oscarSettings->setTimeout( configGroup()->readEntry( "Timeout", 10 ) );
		//FIXME: also needed for the other call to setStatus (in setPresenceTarget)
		Oscar::DWORD status = protocol()->statusManager()->oscarStatusOf( pres );

		if ( !mHideIP )
			status |= Oscar::StatusCode::SHOWIP;
		if ( mWebAware )
			status |= Oscar::StatusCode::WEBAWARE;

		engine()->setStatus( status, mInitialStatusMessage, pres.xtrazStatus(), pres.description() );
		updateVersionUpdaterStamp();

		Connection* c = setupConnection();
		engine()->start( server, port, accountId(), password.left(8) );
		engine()->connectToServer( c, server, port, true /* doAuth */ );

		mInitialStatusMessage.clear();
	}
}

void ICQAccount::loginActions()
{
	OscarAccount::loginActions();

	// Set default privacy settings to make "invisible to" and "visible to" work
	engine()->setPrivacyTLVs( 0x04 );
}

void ICQAccount::disconnected( DisconnectReason reason )
{
	kDebug(14153) << "Attempting to set status offline";
	Oscar::Presence pres( Oscar::Presence::Offline, presence().flags() );
	myself()->setOnlineStatus( protocol()->statusManager()->onlineStatusOf( pres ) );

	QHash<QString, Kopete::Contact*> contactList = contacts();
	foreach( Kopete::Contact* c, contactList )
	{
		OscarContact* oc = dynamic_cast<OscarContact*>( c );
		if ( oc )
		{
			if ( oc->ssiItem().waitingAuth() )
				oc->setOnlineStatus( protocol()->statusManager()->waitingForAuth() );
			else
				oc->setPresenceTarget( Oscar::Presence( Oscar::Presence::Offline ) );
		}
	}

	OscarAccount::disconnected( reason );
}


void ICQAccount::slotToggleInvisible()
{
	using namespace Oscar;
	if ( (presence().flags() & Presence::Invisible) == Presence::Invisible )
		setPresenceFlags( presence().flags() & ~Presence::Invisible );
	else
		setPresenceFlags( presence().flags() | Presence::Invisible );
}

void ICQAccount::slotUserInfo()
{
	if ( mInfoWidget )
	{
		mInfoWidget->raise();
	}
	else
	{
		if ( !this->isConnected() )
			return;

		mInfoContact = new ICQContact( this, engine()->userId(), NULL );

		mInfoWidget = new ICQUserInfoWidget( Kopete::UI::Global::mainWidget(), true );
		QObject::connect( mInfoWidget, SIGNAL( finished() ), this, SLOT( closeUserInfoDialog() ) );
		QObject::connect( mInfoWidget, SIGNAL( okClicked() ), this, SLOT( storeUserInfoDialog() ) );
		mInfoWidget->setContact( mInfoContact );
		mInfoWidget->show();
		engine()->requestFullInfo( engine()->userId() );
	}
}

void ICQAccount::storeUserInfoDialog()
{
	QList<ICQInfoBase*> infoList = mInfoWidget->getInfoData();
	if ( !engine()->updateProfile( infoList ) )
		qDeleteAll( infoList );
}

void ICQAccount::closeUserInfoDialog()
{
	QObject::disconnect( this, 0, mInfoWidget, 0 );
	mInfoWidget->delayedDestruct();
	delete mInfoContact;
	mInfoContact = 0L;
	mInfoWidget = 0L;
}

void ICQAccount::userReadsStatusMessage( const QString& contact )
{
	QString name;

	Kopete::Contact * ct = contacts()[ Oscar::normalize( contact ) ];
	if ( ct )
		name = ct->nickName();
	else
		name = contact;

	KNotification* notification = new KNotification( "icq_user_reads_status_message" );
	notification->setText( i18n( "User %1 is reading your status message", name ) );
	notification->sendEvent();
}

void ICQAccount::setXtrazStatus()
{
	Xtraz::ICQStatusDialog dialog;
	if ( dialog.exec() == QDialog::Accepted )
	{
		Xtraz::Status status = dialog.xtrazStatus();
		setPresenceTarget( status.presence(), status.message() );

		if ( dialog.append() )
		{
			ICQStatusManager* mgr = static_cast<ICQStatusManager*>( protocol()->statusManager() );
			mgr->appendXtrazStatus( status );
		}
	}
}

void ICQAccount::editXtrazStatuses()
{
	ICQStatusManager* icqStatusManager = static_cast<ICQStatusManager*>( protocol()->statusManager() );
	Xtraz::ICQStatusEditor dialog( icqStatusManager );
	dialog.exec();
}

void ICQAccount::setPresenceFlags( Oscar::Presence::Flags flags, const QString &message )
{
	Oscar::Presence pres = presence();
	pres.setFlags( flags );
	kDebug(OSCAR_ICQ_DEBUG) << "new flags=" << (int)flags << ", old type="
		<< (int)pres.flags() << ", new message=" << message << endl;
	setPresenceTarget( pres, message );
}

void ICQAccount::setPresenceTarget( const Oscar::Presence &newPres, const QString &message )
{
	bool targetIsOffline = (newPres.type() == Oscar::Presence::Offline);
	bool accountIsOffline = ( presence().type() == Oscar::Presence::Offline ||
	                          myself()->onlineStatus() == protocol()->statusManager()->connectingStatus() );

	if ( targetIsOffline )
	{
		OscarAccount::disconnect();
		// allow toggling invisibility when offline
		myself()->setOnlineStatus( protocol()->statusManager()->onlineStatusOf( newPres ) );
	}
	else if ( accountIsOffline )
	{
		mInitialStatusMessage = message;
		OscarAccount::connect( protocol()->statusManager()->onlineStatusOf( newPres ) );
	}
	else
	{
		Oscar::DWORD status = protocol()->statusManager()->oscarStatusOf( newPres );
		engine()->setStatus( status, message, newPres.xtrazStatus(), newPres.description() );
	}
}


void ICQAccount::setOnlineStatus( const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason )
{
	if ( status.status() == Kopete::OnlineStatus::Invisible )
	{
		// called from outside, i.e. not by our custom action menu entry...

		if ( presence().type() == Oscar::Presence::Offline )
		{
			// ...when we are offline go online invisible.
			setPresenceTarget( Oscar::Presence( Oscar::Presence::Online, Oscar::Presence::Invisible ) );
		}
		else
		{
			// ...when we are not offline set invisible.
			setPresenceFlags( presence().flags() | Oscar::Presence::Invisible );
		}
	}
	else
	{
		setPresenceTarget( protocol()->statusManager()->presenceOf( status ), reason.message() );
	}
}

void ICQAccount::setStatusMessage( const Kopete::StatusMessage &statusMessage )
{
	Q_UNUSED(statusMessage);
}

OscarContact *ICQAccount::createNewContact( const QString &contactId, Kopete::MetaContact *parentContact, const OContact& ssiItem )
{
	if ( QRegExp("[\\d]+").exactMatch( contactId ) )
	{
		ICQContact* contact = new ICQContact( this, contactId, parentContact, QString() );
		contact->setSSIItem( ssiItem );

		if ( engine()->isActive() )
			contact->loggedIn();

		return contact;
	}
	else
	{
		AIMContact* contact = new AIMContact( this, contactId, parentContact, QString() );
		contact->setSSIItem( ssiItem );

		return contact;
	}
}

QString ICQAccount::sanitizedMessage( const QString& message ) const
{
	QString sanitizedMsg = Qt::escape( message );

	sanitizedMsg.replace( QRegExp(QString::fromLatin1("[\r]?[\n]")), QString::fromLatin1("<br />") );

	return sanitizedMsg;
}

void ICQAccount::slotGotAuthRequest( const QString& contact, const QString& reason )
{
	ICQAuthReplyDialog *replyDialog = new ICQAuthReplyDialog();
	QObject::connect( this, SIGNAL(destroyed()), replyDialog, SLOT(deleteLater()) );
	QObject::connect( replyDialog, SIGNAL(okClicked()), this, SLOT(slotAuthReplyDialogOkClicked()) );

	Kopete::Contact * ct = contacts()[ Oscar::normalize( contact ) ];	
	replyDialog->setUser( ( ct ) ? ct->nickName() : contact );
	replyDialog->setContact( contact );
	replyDialog->setRequestReason( reason );
	replyDialog->show();
}

void ICQAccount::slotAuthReplyDialogOkClicked()
{
    // Do not need to delete will delete itself automatically
	ICQAuthReplyDialog *replyDialog = (ICQAuthReplyDialog*)sender();
	
	if ( replyDialog )
		engine()->sendAuth( replyDialog->contact(), replyDialog->reason(), replyDialog->grantAuth() );
}

#include "icqaccount.moc"

//kate: tab-width 4; indent-mode csands;
