#include "gaduaccount.h"

GaduAccount::GaduAccount( KopeteProtocol* parent, const QString& accountID,
													const char* name )
{
	pingTimer_ = 0;

	session_ = new GaduSession( this, "GaduSession" );

	KGlobal::config()->setGroup("Gadu");
	setAccountId( KGlobal::config()->readEntry("Uin", "0") );
	userUin_ = accountID().toUInt();
	setPassword( KGlobal::config()->readEntry("Password", "") );
	nick_    = KGlobal::config()->readEntry("Nick", "");
	myself_ = new GaduContact( pluginId(), userUin_, nick_,
														 new KopeteMetaContact() );

	initActions();
	initConnections();

	setAutoLogin( KGlobal::config()->readBoolEntry("AutoConnect", false) );
}

void
GaduAccount::initActions()
{
	KAction* onlineAction_    = new KAction( i18n("Go O&nline"), "gg_online", 0, this,
																					 SLOT(slotGoOnline()), this, "actionGaduConnect" );
	KAction* offlineAction_   = new KAction( i18n("Go &Offline"), "gg_offline", 0, this,
																					 SLOT(slotGoOffline()), this, "actionGaduConnect" );
	KAction* awayAction_      = new KAction( i18n("Set &Away"), "gg_away", 0, this,
																					 SLOT(slotGoAway()), this, "actionGaduConnect" );
	KAction* busyAction_      = new KAction( i18n("Set &Busy"), "gg_busy", 0, this,
																					 SLOT(slotGoBusy()), this, "actionGaduConnect" );
	KAction* invisibleAction_ = new KAction( i18n("Set &Invisible"), "gg_invi", 0, this,
																					 SLOT(slotGoInvisible()), this, "actionGaduConnect" );

	actionMenu_ = new KActionMenu( "Gadu-Gadu", this );

	actionMenu_->popupMenu()->insertTitle( pluginId() );

	actionMenu_->insert( onlineAction );
	actionMenu_->insert( offlineAction );
	actionMenu_->insert( awayAction );
	actionMenu_->insert( busyAction );
	actionMenu_->insert( invisibleAction );
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

	if ( !parentContact->findContact( pluginId(), myself_->contactId(), contactId ) ) {
		GaduContact *newContact = new GaduContact( pluginId(), uinNumber, displayName, parentContact );
		newContact->setParentIdentity( QString::number( userUin_ ) );
		contactsMap_.insert( uinNumber, newContact );
		addNotify( uinNumber );
	}

	return true;
}

void
GaduAccount::changeStatus( const KopeteOnlineStatus& status, const QString& descr )
{
	if ( !session_->isConnected() ) {
		slotLogin();
	}
	if ( descr.isEmpty() ) {
		session_->changeStatus( status.internalStatus() );
	} else {
		session_->changeStatusDescription( status.internalStatus(), descr );
	}
	status_ = status;
	myself_->setOnlineStatus( status_ );
}

void
GaduAccount::slotLogin()
{
	if ( (userUin_ == 0) || password_.isEmpty() ) {
		KMessageBox::error( qApp->mainWidget(),
												i18n("You must fill in UIN and password fields in the preferences dialog before you can login"),
												i18n("Unable to Login") );
		return;
	}
	if ( !session_->isConnected() ) {
		session_->login( userUin_, password_, GG_STATUS_AVAIL );
		status_ = GaduStatusAvail;
		myself_->setOnlineStatus( status_ );
		changeStatus( status_ );
	} else {
		session_->changeStatus( GG_STATUS_AVAIL );
		status_ = GaduStatusAvail;
		myself_->setOnlineStatus( status_);
		changeStatus( status_ );
	}
}

void
GaduAccount::slotLogoff()
{
	if ( session_->isConnected() ) {
		status_ = GaduStatusOffline;
		changeStatus( status_ );
	} else
		setStatusIcon( "gg_offline" );
}

void
GaduAccount::slotGoOnline()
{
	if ( !session_->isConnected() ) {
		kdDebug(14100)<<"#### Connecting..."<<endl;
		slotLogin();
	} else
		changeStatus( GaduStatusAvail );
}

void
GaduAccount::slotGoOffline()
{
	slotLogoff();
}

void
GaduAccount::slotGoInvisible()
{
	changeStatus( GaduStatusInvisible );
}

void
GaduAccount::slotGoAway()
{
	changeStatus( GaduStatusNotAvail );
}

void
GaduAccount::slotGoBusy()
{
	changeStatus( GaduStatusBusy );
}

void
GaduAccount::removeContact( const GaduContact* c )
{
	if ( isConnected() ) {
		const uin_t u = c->uin();
		session_->removeNotify( u );
		contactsMap_.remove( u );
		delete c;
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
		kdDebug(14100)<<"####"<<" System Message "<<endl;
		return;
	}

	GaduContact *c = contactsMap_.find( e->event.msg.sender ).data();
	if ( c ) {
		KopeteContactPtrList tmp;
		tmp.append( myself_ );
		KopeteMessage msg( c, tmp, (const char*)e->event.msg.message, KopeteMessage::Inbound );
		c->messageReceived( msg );
	} else {
		addContact( QString::number(e->event.msg.sender), QString::number(e->event.msg.sender) );
		GaduContact *c = contactsMap_.find( e->event.msg.sender ).data();
		KopeteContactPtrList tmp;
		tmp.append( myself_ );
		KopeteMessage msg( c, tmp, (const char*)e->event.msg.message, KopeteMessage::Inbound );
		c->messageReceived( msg );
	}
}

void
GaduAccount::ackReceived( struct gg_event* /* e */ )
{
	//kdDebug(14100)<<"####"<<"Received an ACK from "<<e->event.ack.recipient<<endl;
}

void
GaduAccount::notify( struct gg_event* e )
{
	GaduContact *c;

	struct gg_notify_reply *n = e->event.notify;

	while( n && n->uin ) {
		kdDebug(14100)<<"### NOTIFY "<<n->uin<<endl;
		if ( !(c=contactsMap_.find(n->uin).data()) ) {
			++n;
			continue;
		}
		if ( c->onlineStatus() == convertStatus( n->status ) )
			continue;
		c->setOnlineStatus( convertStatus( n->status ) );
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
		if ( c->onlineStatus() == convertStatus( n->status ) )
			continue;
		c->setDescription( descr );
		c->setOnlineStatus( convertStatus( n->status ) );
	}
}

void
GaduAccount::statusChanged( struct gg_event* e )
{
	GaduContact *c = contactsMap_.find( e->event.status.uin ).data();
	if( !c )
		return;
	c->setDescription( e->event.status.descr );
	c->setOnlineStatus( convertStatus( e->event.status.status ) );
}

void
GaduAccount::pong()
{
	//kdDebug(14100)<<"####"<<" Pong..."<<endl;
}

void
GaduAccount::connectionFailed( struct gg_event* /*e*/ )
{
	KMessageBox::error( qApp->mainWidget(), i18n("Plugin unable to connect to the Gadu-Gadu server."),
											i18n("Connection Error") );
	setStatusIcon( "gg_offline" );
}

void
GaduAccount::connectionSucceed( struct gg_event* /*e*/ )
{
	kdDebug(14100)<<"#### Gadu-Gadu connected!"<<endl;
	UserlistGetCommand *cmd = new UserlistGetCommand( this );
	cmd->setInfo( userUin_, password_ );
	QObject::connect( cmd, SIGNAL(done(const QStringList&)),
										SLOT(userlist(const QStringList&)) );
	cmd->execute();
	if ( !pingTimer_ ) {
		pingTimer_ = new QTimer( this );
		QObject::connect( pingTimer_, SIGNAL(timeout()),
											SLOT(pingServer()) );
	}
	pingTimer_->start( 180000 );//3 minute timeout
	ContactsMap::Iterator it;
	for ( it = contactsMap_.begin(); it != contactsMap_.end(); ++it ) {
		addNotify( it.key() );
	}
}

void
GaduAccount::slotSessionDisconnect()
{
	pingTimer_->stop();
	changeStatus( GaduStatusOffline );
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



#include "gaduaccount.moc"
