// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
#include "gaduaccount.h"
#include "gadusession.h"
#include "gaducontact.h"
#include "gaduprotocol.h"
#include "gaduaway.h"

#include "kopetemetacontact.h"

#include <kaction.h>
#include <kpassdlg.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include <qapplication.h>
#include <qdialog.h>
#include <qtimer.h>


GaduAccount::GaduAccount( KopeteProtocol* parent, const QString& accountID,
													const char* name )
  : KopeteAccount( parent, accountID, name ), pingTimer_(0)
{
	session_ = new GaduSession( this, "GaduSession" );

	KGlobal::config()->setGroup("Gadu");
	userUin_ = accountID.toUInt();
	nick_    = KGlobal::config()->readEntry("Nick", "");
	myself_ = new GaduContact(  userUin_, nick_, this,
                              new KopeteMetaContact() );

	initActions();
	initConnections();

	setAutoLogin( KGlobal::config()->readBoolEntry("AutoConnect", false) );
}

void
GaduAccount::initActions()
{
	KAction* onlineAction    = new KAction( i18n("Go O&nline"), "gg_online", 0, this,
                                          SLOT(slotGoOnline()), this, "actionGaduConnect" );
	KAction* offlineAction   = new KAction( i18n("Go &Offline"), "gg_offline", 0, this,
                                          SLOT(slotGoOffline()), this, "actionGaduConnect" );
	KAction* busyAction      = new KAction( i18n("Set &Busy"), "gg_busy", 0, this,
                                          SLOT(slotGoBusy()), this, "actionGaduConnect" );
	KAction* invisibleAction = new KAction( i18n("Set &Invisible"), "gg_invi", 0, this,
                                          SLOT(slotGoInvisible()), this, "actionGaduConnect" );
	KAction* descrAction     = new KAction( i18n("Set &Description"), "info", 0, this,
																					SLOT(slotDescription()), this, "actionGaduDescription" );

	actionMenu_ = new KActionMenu( "Gadu-Gadu", this );

	actionMenu_->popupMenu()->insertTitle( protocol()->pluginId() + "(" + myself_->identityId() + ")" );

	actionMenu_->insert( onlineAction );
	actionMenu_->insert( busyAction );
	actionMenu_->insert( invisibleAction );
  actionMenu_->insert( offlineAction );
	actionMenu_->insert( descrAction );

  actionMenu_->popupMenu()->insertSeparator();

  actionMenu_->insert( new KAction( i18n("Change Password"), "", 0, this,
																		SLOT(slotChangePassword()), this, "actionChangePassword" ) );
}

void
GaduAccount::initConnections()
{
	QObject::connect( session_, SIGNAL(error(const QString&,const QString&)),
										SLOT(error(const QString&, const QString&)) );
	QObject::connect( session_, SIGNAL(messageReceived( struct gg_event* )),
										SLOT(messageReceived(struct gg_event*)) );
	QObject::connect( session_, SIGNAL(notify( struct gg_event* )),
										SLOT(notify(struct gg_event*)) );
	QObject::connect( session_, SIGNAL(notifyDescription( struct gg_event* )),
										SLOT(notifyDescription(struct gg_event*)) );
	QObject::connect( session_, SIGNAL(statusChanged(struct gg_event*)),
										SLOT(statusChanged(struct gg_event*)) );
	QObject::connect( session_, SIGNAL(connectionFailed( struct gg_event* )),
										SLOT(connectionFailed(struct gg_event*)) );
	QObject::connect( session_, SIGNAL(connectionSucceed( struct gg_event* )),
										SLOT(connectionSucceed(struct gg_event*)) );
	QObject::connect( session_, SIGNAL(disconnect()),
										SLOT(slotSessionDisconnect()) );
	QObject::connect( session_, SIGNAL(ackReceived(struct gg_event*)),
										SLOT(ackReceived(struct gg_event*)) );
}

void GaduAccount::setAway( bool isAway, const QString& awayMessage )
{
  if ( isAway ) {
    uint status = (awayMessage.isEmpty()) ? GG_STATUS_BUSY : GG_STATUS_BUSY_DESCR;
    changeStatus( GaduProtocol::protocol()->convertStatus( status ), awayMessage  );
  }
}

KopeteContact* GaduAccount::myself() const
{
	return myself_;
}

KActionMenu* GaduAccount::actionMenu()
{
	return actionMenu_;
}

void GaduAccount::connect()
{
	slotGoOnline();
}

void GaduAccount::disconnect()
{
	slotGoOffline();
}

bool GaduAccount::addContactToMetaContact( const QString& contactId, const QString& displayName,
																					 KopeteMetaContact* parentContact )
{
	uin_t uinNumber = contactId.toUInt();

	if ( !parentContact->findContact( protocol()->pluginId(), myself_->contactId(), contactId ) ) {
		GaduContact *newContact = new GaduContact( uinNumber, displayName, this, parentContact );
		newContact->setParentIdentity( QString::number( userUin_ ) );
		contactsMap_.insert( uinNumber, newContact );
		addNotify( uinNumber );
	}

	return true;
}

void
GaduAccount::changeStatus( const KopeteOnlineStatus& status, const QString& descr )
{
  kdDebug()<<"### Status = "<<session_->isConnected()<<endl;
	if ( !session_->isConnected() ) {
		slotLogin();
	}
	if ( descr.isEmpty() ) {
		if ( session_->changeStatus( status.internalStatus() ) != 0 )
      return;
	} else {
		if ( session_->changeStatusDescription( status.internalStatus(), descr ) != 0 )
      return;
	}
	status_ = status;
	myself_->setOnlineStatus( status_ );
  if ( status_.internalStatus() == GG_STATUS_NOT_AVAIL ||
       status_.internalStatus() == GG_STATUS_NOT_AVAIL_DESCR ) {
    pingTimer_->stop();
  }
}

void
GaduAccount::slotLogin()
{
	if ( (userUin_ == 0) || getPassword().isEmpty() ) {
		KMessageBox::error( qApp->mainWidget(),
												i18n("You must fill in UIN and password fields in the preferences dialog before you can login"),
												i18n("Unable to Login") );
		return;
	}
	if ( !session_->isConnected() ) {
		session_->login( userUin_, getPassword(), GG_STATUS_AVAIL );
	} else {
		session_->changeStatus( GG_STATUS_AVAIL );
	}
}

void
GaduAccount::slotLogoff()
{
	if ( session_->isConnected() ) {
		status_ = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
		changeStatus( status_ );
    session_->logoff();

	}
}

void
GaduAccount::slotGoOnline()
{
	if ( !session_->isConnected() ) {
		kdDebug(14100)<<"#### Connecting..."<<endl;
		slotLogin();
	} else
		changeStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_AVAIL ) );
}

void
GaduAccount::slotGoOffline()
{
	slotLogoff();
}

void
GaduAccount::slotGoInvisible()
{
	changeStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_INVISIBLE ) );
}

void
GaduAccount::slotGoBusy()
{
	changeStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_BUSY ) );
}

void
GaduAccount::removeContact( const GaduContact* c )
{
	if ( isConnected() ) {
		const uin_t u = c->uin();
		session_->removeNotify( u );
		contactsMap_.remove( u );
	} else {
		KMessageBox::error( 0l,
												i18n( "Please go online to remove contact" ),
												i18n( "Gadu Plugin" ));
	}
}

void
GaduAccount::addNotify( uin_t uin )
{
	if ( session_->isConnected() )
		session_->addNotify( uin );
}

void
GaduAccount::notify( uin_t* userlist, int count )
{
	if ( session_->isConnected() )
		session_->notify( userlist, count );
}

void
GaduAccount::sendMessage( uin_t recipient, const QString& msg, int msgClass )
{
	if ( session_->isConnected() )
		session_->sendMessage( recipient, msg, msgClass );
}

void
GaduAccount::error( const QString& title, const QString& message )
{
	KMessageBox::error( qApp->mainWidget(), message, title );
}

void
GaduAccount::messageReceived( struct gg_event* e )
{
	//kdDebug(14100)<<"####"<<" Great! Message Received :: "<<((const char*)e->event.msg.message)<<endl;

	if ( !e->event.msg.message )
		return;

	if ( e->event.msg.sender == 0 ) {
		//system message, display them or not?
		kdDebug(14100)<<"####"<<" System Message "<< (const char*)e->event.msg.message << endl;
		return;
	}

  kdDebug(14100)<<"Message from " << e->event.msg.sender <<" = "<< (const char*)e->event.msg.message << endl;
	GaduContact *c = 0;
  if ( contactsMap_.contains( e->event.msg.sender ) )
    c = contactsMap_[ e->event.msg.sender ];
	if ( c ) {
		KopeteContactPtrList tmp;
		tmp.append( myself_ );
		KopeteMessage msg( c, tmp, (const char*)e->event.msg.message, KopeteMessage::Inbound );
		c->messageReceived( msg );
	} else {
		addContact( QString::number(e->event.msg.sender), QString::number(e->event.msg.sender) );
		c = contactsMap_.find( e->event.msg.sender ).data();
		KopeteContactPtrList tmp;
		tmp.append( myself_ );
		KopeteMessage msg( c, tmp, (const char*)e->event.msg.message, KopeteMessage::Inbound );
		c->messageReceived( msg );
	}
}

void
GaduAccount::ackReceived( struct gg_event* e  )
{
  if ( contactsMap_.contains( e->event.ack.recipient ) ) {
    GaduContact *contact = contactsMap_[ e->event.ack.recipient ];
    kdDebug(14100)<<"####"<<"Received an ACK from "<<contact->uin()<<endl;
    contact->messageAck();
  } else {
    kdDebug(14100)<<"####"<<"Received an ACK from an unknown user : "<< e->event.ack.recipient <<endl;
  }
}

void
GaduAccount::notify( struct gg_event* e )
{
	GaduContact *c;

	struct gg_notify_reply *n = e->event.notify;

	while( n && n->uin ) {
		kdDebug(14100)<<"### NOTIFY "<<n->uin<< " " << n->status << endl;
		if ( !(c=contactsMap_.find(n->uin).data()) ) {
			kdDebug(14100)<<"Notify not in the list "<< n->uin << endl;
			session_->removeNotify( n->uin );
			++n;
			continue;
		}
		if ( c->onlineStatus() == GaduProtocol::protocol()->convertStatus( n->status ) ) {
			kdDebug(14100)<<"### " << c->displayName()<<" is INVISIBLE"<<endl;
			++n;
			continue;
		}
		c->setOnlineStatus(  GaduProtocol::protocol()->convertStatus( n->status ) );
		++n;
	}
}

void
GaduAccount::notifyDescription( struct gg_event* e )
{
	GaduContact *c;
	struct gg_notify_reply *n;

	n =  e->event.notify_descr.notify;

	for( ; n->uin ; n++ ) {
		char *descr = (e->type == GG_EVENT_NOTIFY_DESCR) ? e->event.notify_descr.descr : NULL;
		if ( !(c=contactsMap_.find( n->uin ).data()) )
			continue;
		if ( c->onlineStatus() ==  GaduProtocol::protocol()->convertStatus( n->status ) )
			continue;
		c->setDescription( descr );
		c->setOnlineStatus( GaduProtocol::protocol()->convertStatus( n->status ) );
	}
}

void
GaduAccount::statusChanged( struct gg_event* e )
{
	GaduContact *c = contactsMap_.find( e->event.status.uin ).data();
	if( !c )
		return;
	c->setDescription( e->event.status.descr );
	c->setOnlineStatus( GaduProtocol::protocol()->convertStatus( e->event.status.status ) );
}

void
GaduAccount::pong()
{
	//kdDebug(14100)<<"####"<<" Pong..."<<endl;
}

void
GaduAccount::connectionFailed( struct gg_event* /*e*/ )
{
  status_ = GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL );
  myself_->setOnlineStatus( status_ );
	KMessageBox::error( qApp->mainWidget(), i18n("Plugin unable to connect to the Gadu-Gadu server."),
											i18n("Connection Error") );
}

void
GaduAccount::connectionSucceed( struct gg_event* /*e*/ )
{
	kdDebug(14100)<<"#### Gadu-Gadu connected! "<<endl;
	status_ =  GaduProtocol::protocol()->convertStatus( session_->status() );
	myself_->setOnlineStatus( status_ );
	startNotify();
	UserlistGetCommand *cmd = new UserlistGetCommand( this );
	cmd->setInfo( userUin_, getPassword() );
	QObject::connect( cmd, SIGNAL(done(const QStringList&)),
										SLOT(userlist(const QStringList&)) );
	cmd->execute();
	if ( !pingTimer_ ) {
		pingTimer_ = new QTimer( this );
		QObject::connect( pingTimer_, SIGNAL(timeout()),
											SLOT(pingServer()) );
	}
	pingTimer_->start( 180000 );//3 minute timeout
}

void
GaduAccount::startNotify()
{
	int i = 0;
	QValueList<uin_t> l = contactsMap_.keys();

	QValueList<uin_t>::iterator it;
	uin_t *userlist = 0;
	if ( !contactsMap_.empty() ) {
		userlist = new uin_t[contactsMap_.count()];

		for( it = l.begin(); it != l.end(); ++it, ++i ) {
			userlist[i] = (*it);
		}
	}
	session_->notify( userlist, contactsMap_.count() );
}

void
GaduAccount::slotSessionDisconnect()
{
	pingTimer_->stop();
	changeStatus(  GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );
}

void
GaduAccount::userlist( const QStringList& u )
{
	int i;
	QString name, group, uin;
	kdDebug(14100)<<"### Got userlist"<<endl;
	for ( QStringList::ConstIterator it = u.begin(); it != u.end(); ++it ) {
		QStringList user = QStringList::split( ";", *it );
		i = 0;
		for ( QStringList::Iterator it = user.begin(); it != user.end() && !(*it).isEmpty(); ++it,++i ) {
			if ( i == 0 ) {
				name = *it;
			} else if ( i == 4 ) {
				group = *it;
			} else if ( i == 5 ) {
				uin = *it;
			}
		}
		//kdDebug(14100)<<"uin = "<< uin << "; name = "<< name << "; group = " << group <<endl;
		if ( ! contactsMap_.contains( uin.toUInt() ) )
			addContact( uin, name, 0L, group );
	}
}

void
GaduAccount::pingServer()
{
	session_->ping();
}

void
GaduAccount::slotChangePassword()
{
	QCString password;
	int result = KPasswordDialog::getPassword( password, i18n("Enter new password") );

	if ( result == KPasswordDialog::Accepted ) {
		ChangePasswordCommand *cmd = new ChangePasswordCommand( this, "changePassCmd" );
		cmd->setInfo( myself_->uin(), getPassword(), password, "zackrat@att.net" );
		QObject::connect( cmd, SIGNAL(done(const QString&, const QString&)),
											SLOT(slotCommandDone(const QString&, const QString&)) );
		QObject::connect( cmd, SIGNAL(error(const QString&, const QString&)),
											SLOT(slotCommandError(const QString&, const QString&)) );
	}
}

void
GaduAccount::slotCommandDone( const QString& title, const QString& what )
{
	KMessageBox::information( qApp->mainWidget(), title, what );
}

void
GaduAccount::slotCommandError(const QString& title, const QString& what )
{
	KMessageBox::error( qApp->mainWidget(), title, what );
}

void
GaduAccount::slotDescription()
{
	GaduAway *away = new GaduAway( this );

	if( away->exec() == QDialog::Accepted ) {
		changeStatus( GaduProtocol::protocol()->convertStatus( away->status() ),
									away->awayText() );
	}
	delete away;
}

#include "gaduaccount.moc"
