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
#include <qtextcodec.h>



GaduAccount::GaduAccount( KopeteProtocol* parent, const QString& accountID,const char* name )
  : KopeteAccount( parent, accountID, name ), pingTimer_(0)
{

	textcodec_ = QTextCodec::codecForName("CP1250");

	session_ = new GaduSession( this, "GaduSession" );

	KGlobal::config()->setGroup("Gadu");

// Instead of nick i will probably use e-mail

        kdDebug()<<"ID = "<< accountId() <<endl;

	myself_ = new GaduContact(  accountId().toInt(),  accountId(), this,
                              new KopeteMetaContact() );

	initActions();
	initConnections();
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

//  actionMenu_->insert( new KAction( i18n("Change Password"), "", 0, this,
//		SLOT(slotChangePassword()), this, "actionChangePassword" ) );

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
	uint status;

	if ( isAway ) {
		status = (awayMessage.isEmpty()) ? GG_STATUS_BUSY : GG_STATUS_BUSY_DESCR;
	}
	else{
		status = (awayMessage.isEmpty()) ? GG_STATUS_AVAIL : GG_STATUS_AVAIL_DESCR;
	}
	changeStatus( GaduProtocol::protocol()->convertStatus( status ), awayMessage );
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
		newContact->setParentIdentity( accountId() );
		contactsMap_.insert( uinNumber, newContact );
		addNotify( uinNumber );
	}

	return true;
}

void
GaduAccount::changeStatus( const KopeteOnlineStatus& status, const QString& descr )
{
  kdDebug()<<"### Status = "<<session_->isConnected()<<endl;

	if ( GG_S_NA( status.internalStatus() ) ) {
		if ( !session_->isConnected() ){
			return;//already logged off
		}
		else if ( status.internalStatus() == GG_STATUS_NOT_AVAIL_DESCR ) {
			if ( session_->changeStatusDescription( status.internalStatus(), descr ) != 0 )
				return;
		}
		session_->logoff();
	} else {
		if ( !session_->isConnected() ) {
			slotLogin( status.internalStatus(), descr );
			status_ = status;
			return;
		} else {
			if ( descr.isEmpty() ) {
				if ( session_->changeStatus( status.internalStatus() ) != 0 )
					return;
			} else {
				if ( session_->changeStatusDescription( status.internalStatus(), descr ) != 0 )
					return;
			}
		}
	}
	status_ = status;
	myself_->setOnlineStatus( status_ );
  if ( status_.internalStatus() == GG_STATUS_NOT_AVAIL ||
       status_.internalStatus() == GG_STATUS_NOT_AVAIL_DESCR ) {
    if (pingTimer_){
	pingTimer_->stop();
    }

  }
}

void
GaduAccount::slotLogin( int status, const QString& dscr  )
{

// this will never happend
/*
	if ( (accountId().toInt() == 0) || getPassword().isEmpty() ) {
		KMessageBox::error( qApp->mainWidget(),
			i18n("You must fill in UIN and password fields in the preferences dialog before you can login"),
			i18n("Unable to Login") );
		return;
	}
*/
	myself_->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_CONNECTING ) );
	if ( !session_->isConnected() ) {
		session_->login( accountId().toInt(), getPassword(), status, dscr );
	} else {
		session_->changeStatus( status );
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
	}

// no reason for me to be online to delete

//	} else {
//		KMessageBox::error( 0l,	i18n( "Please go online to remove contact" ),
//			    i18n( "Gadu Plugin" ));
//	}

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
    QString sendMsg, cpmsg;
	if ( session_->isConnected() ){
	    sendMsg = msg;
	    sendMsg.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "\r\n" ) );
            cpmsg = textcodec_->fromUnicode(sendMsg);
	    session_->sendMessage( recipient, cpmsg, msgClass );
	}
}

void
GaduAccount::error( const QString& title, const QString& message )
{
	KMessageBox::error( qApp->mainWidget(), message, title );
}

void
GaduAccount::messageReceived( struct gg_event* e )
{
    GaduContact *c = 0;
    QString message;

    if ( !e->event.msg.message )
	return;

    message = textcodec_->toUnicode((const char*)e->event.msg.message);

    if ( e->event.msg.sender == 0 ) {
	//system message, display them or not?
	kdDebug(14100)<<"####"<<" System Message "<< message << endl;
	return;
    }

    if ( contactsMap_.contains( e->event.msg.sender ) )
	c = contactsMap_[ e->event.msg.sender ];

    if ( c ) {
	KopeteContactPtrList tmp;
	tmp.append( myself_ );
	KopeteMessage msg( c, tmp, message, KopeteMessage::Inbound );
	c->messageReceived( msg );
    } else {
	addContact( QString::number(e->event.msg.sender), QString::number(e->event.msg.sender) );
	c = contactsMap_.find( e->event.msg.sender ).data();
	KopeteContactPtrList tmp;
	tmp.append( myself_ );
	KopeteMessage msg( c, tmp, message, KopeteMessage::Inbound );
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
    kdDebug(14100)<<"####"<<" status changed, uin:"<< e->event.status.uin <<endl;
    GaduContact *c = contactsMap_.find( e->event.status.uin ).data();
    if( !c )
	return;
    c->setDescription( e->event.status.descr );
    c->setOnlineStatus( GaduProtocol::protocol()->convertStatus( e->event.status.status ) );
}

void
GaduAccount::pong()
{
    kdDebug(14100)<<"####"<<" Pong..."<<endl;
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
    cmd->setInfo( accountId().toInt(), getPassword() );
    QObject::connect( cmd, SIGNAL(done(const QString&)),
    			SLOT(userlist(const QString&)) );
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
    kdDebug(14100)<<"Disconnecting"<<endl;

    if (pingTimer_){
	pingTimer_->stop();
    }

    for ( ContactsMap::iterator it = contactsMap_.begin(); it != contactsMap_.end(); ++it) {
	it.data()->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );
    }

    if ( myself_->onlineStatus().internalStatus() != GG_STATUS_NOT_AVAIL ||
	 myself_->onlineStatus().internalStatus() != GG_STATUS_NOT_AVAIL_DESCR ){

	myself_->setOnlineStatus( GaduProtocol::protocol()->convertStatus( GG_STATUS_NOT_AVAIL ) );
    }
}

void
GaduAccount::userlist( const QString& u)
{
	QStringList result;
	QString name;
	QString group;
	QString uin;
	QString firstname;
	QString surname;
	QString nickname;
	QString phonenr;
	QString cline;
	QString contactname;
	QStringList::iterator it;
	QStringList strList ;


	kdDebug(14100)<<"### Got userlist"<<endl;

	QStringList ln  = QStringList::split( QChar('\n') , u, true );
	QStringList::iterator lni=ln.begin();

	while(lni != ln.end()){
		cline=(*lni);
		if (cline.isNull()){
			break;
		}

		strList  = QStringList::split( QChar(';'), cline, true );
		if (strList.count()!=8){
			kdDebug(14100)<< "fishy, maybe contact format was changed. Contact author/update software"<<endl;
			kdDebug(14100)<<"nr of elements should be 8, but is "<<strList.count() << " LINE:" << cline <<endl;
			++lni;
			continue;
		}

		it = strList.begin();
//each line ((firstname);(secondname);(nickname).;(name);(tel);(group);(uin);

		firstname	= (*it);
		surname		= (*++it);
		nickname	= (*++it);
		name		= (*++it);
		phonenr		= (*++it);
		group		= (*++it);
		uin		= (*++it);

		++lni;

		if (uin.isNull()){
		    kdDebug(14100) << "no Uin, strange "<<endl;
		    kdDebug(14100) << "LINE:" << cline <<endl;
		    continue;
		}

		// if exists, don't add it
		// FIXME: this does not work, i don't know reason why :/
		// 	  contacts map didn't work either
		if (contacts()[uin]){
		    continue;
		}

		// if there is no nicname
		if (nickname.isNull()){
			// no name either
			if (name.isNull()){
			// maybe we can use fistname + surname ?
				if (firstname.isNull() && surname.isNull()){
					contactname=uin;
				}
				// what a shame, i have to use UIN than :/
				else{
					if (firstname.isNull()){
						contactname=surname;
					}
					else{
						if (surname.isNull()){
						contactname=firstname;
						}
						else{
							contactname=firstname+" "+surname;
						}
					}
				}
			}
			else{
				contactname=name;
			}
		}
		else{
			contactname=nickname;
		}
		addContact( uin, contactname, 0L, QString::null);
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
/*
	QCString password;
	int result = KPasswordDialog::getPassword( password, i18n("Enter new password") );

//	if ( result == KPasswordDialog::Accepted ) {
//		ChangePasswordCommand *cmd = new ChangePasswordCommand( this, "changePassCmd" );
//		cmd->setInfo( myself_->uin(), getPassword(), password, "zackrat@att.net" );
//		QObject::connect( cmd, SIGNAL(done(const QString&, const QString&)),
//											SLOT(slotCommandDone(const QString&, const QString&)) );
//		QObject::connect( cmd, SIGNAL(error(const QString&, const QString&)),
//											SLOT(slotCommandError(const QString&, const QString&)) );
//	}
*/
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
