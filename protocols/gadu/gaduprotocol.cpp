#include <qapplication.h>
#include <qcursor.h>
#include <qtimer.h>

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"

#include "gaduprotocol.h"
#include "gaducontact.h"
#include "gaduaddcontactpage.h"
#include "gadupreferences.h"
#include "gadusession.h"
#include "gaducommands.h"

#include <libgadu.h>

K_EXPORT_COMPONENT_FACTORY( kopete_gadu, KGenericFactory<GaduProtocol> );

GaduProtocol::GaduProtocol( QObject* parent, const char* name, const QStringList & )
    : KopeteProtocol( parent, name )
{
    if ( protocolStatic_ )
        kdDebug()<<"####"<<"GaduProtocol already initialized"<<endl;
    else
        protocolStatic_ = this;

    pingTimer_ = 0;

    session_ = new GaduSession( this, "GaduSession" );

    KGlobal::config()->setGroup("Gadu");
    userUin_ = KGlobal::config()->readEntry("Uin", "0").toUInt();
    password_= KGlobal::config()->readEntry("Password", "");
    nick_    = KGlobal::config()->readEntry("Nick", "");
    myself_ = new GaduContact( this->id(), userUin_, nick_,
                               new KopeteMetaContact() );

    prefs_ = new GaduPreferences( "gadu_protocol", this );
    connect( prefs_, SIGNAL(saved()), this, SLOT(settingsChanged()) );

    initActions();
    initConnections();

    setStatusIcon( "gg_connecting" );

    if( KGlobal::config()->readBoolEntry("AutoConnect", false) )
        slotGoOnline();
}

GaduProtocol::~GaduProtocol()
{
    protocolStatic_ = 0L;
}

GaduProtocol* GaduProtocol::protocolStatic_ = 0L;

void
GaduProtocol::initActions()
{
    onlineAction_    = new KAction( i18n("Go O&nline"), "gg_online", 0, this,
                                    SLOT(slotGoOnline()), this, "actionGaduConnect" );
    offlineAction_   = new KAction( i18n("Go &Offline"), "gg_offline", 0, this,
                                    SLOT(slotGoOffline()), this, "actionGaduConnect" );
    awayAction_      = new KAction( i18n("Set &Away"), "gg_away", 0, this,
                                    SLOT(slotGoAway()), this, "actionGaduConnect" );
    busyAction_      = new KAction( i18n("Set &Busy"), "gg_busy", 0, this,
                                    SLOT(slotGoBusy()), this, "actionGaduConnect" );
    invisibleAction_ = new KAction( i18n("Set &Invisible"), "gg_invi", 0, this,
                                    SLOT(slotGoInvisible()), this, "actionGaduConnect" );

    actionMenu_ = new KActionMenu( "Gadu", this );

    actionMenu_->popupMenu()->insertTitle( id() );

    actionMenu_->insert( onlineAction_ );
    actionMenu_->insert( offlineAction_ );
    actionMenu_->insert( awayAction_ );
    actionMenu_->insert( busyAction_ );
    actionMenu_->insert( invisibleAction_ );
}

void
GaduProtocol::initConnections()
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
                      SLOT(disconnect()) );
    QObject::connect( session_, SIGNAL(ackReceived(struct gg_event*)),
                      SLOT(ackReceived(struct gg_event*)) );

}

GaduProtocol*
GaduProtocol::protocol()
{
    return protocolStatic_;
}

void
GaduProtocol::init()
{
}

QString
GaduProtocol::protocolIcon() const
{
    return "gadu_protocol";
}

void
GaduProtocol::Connect()
{
    slotGoOnline();
}

void
GaduProtocol::Disconnect()
{
    slotGoOffline();
}

bool
GaduProtocol::isConnected() const
{
    return session_->isConnected();
}

KopeteContact*
GaduProtocol::createContact( KopeteMetaContact* /*parent*/, const QString& serializedData )
{
    kdDebug()<<"####"<<"createContact() called with \""<<serializedData<<"\""<<endl;
    return 0L;
}

void
GaduProtocol::setAway()
{
    slotGoAway();
}

void
GaduProtocol::setAvailable()
{
    slotGoOnline();
}

bool
GaduProtocol::isAway(void) const
{
    return (myself_->status() == KopeteContact::Away);
}

AddContactPage*
GaduProtocol::createAddContactWidget( QWidget* parent )
{
    return new GaduAddContactPage( this, parent );
}

KopeteContact*
GaduProtocol::myself() const
{
    return myself_;
}

void
GaduProtocol::addContact( const QString& uin, const QString& nick,
                          KopeteMetaContact* parent, const QString& group )
{
	KopeteMetaContact *m=0l;

	if ( !parent )
	{
		m = KopeteContactList::contactList()->findContact( this->id(),  QString::number( userUin_ ), uin );
		if( !m )
		{
			m = new KopeteMetaContact();
			m->addToGroup(KopeteContactList::contactList()->getGroup(group));
			KopeteContactList::contactList()->addMetaContact(m);
		}
	}
	else
		m = parent;


    KopeteContact *c = m->findContact( this->id(), QString::number( userUin_ ) , uin );

    if( !c ) {
        uin_t uinNumber = uin.toUInt();
        QString uins;
        if ( m->addressBookField( this, "messaging/gadu" ).isEmpty() )
            uins = uin;
        else
            uins = m->addressBookField( this, "messaging/gadu" )
                   + "\n" + uin;
        m->setAddressBookField( this, "messaging/gadu", uins );
        GaduContact *contact = new GaduContact( this->id(), uinNumber,
                                                nick, m );
        contact->setParentIdentity( QString::number( userUin_ ) );
        m->addContact( contact );
        contactsMap_.insert( uinNumber, contact );
        addNotify( uinNumber );
    }
}

void
GaduProtocol::removeContact( const GaduContact* c )
{
    if ( isConnected() ) {
        const uin_t u = c->uin();
        session_->removeNotify( u );
        contactsMap_.remove( u );
        delete c;
    } else {
        KMessageBox::error( 0l,
                            i18n( "<qt>Please go online to remove contact</qt>" ),
                            i18n( "Gadu Plugin" ));
    }
}

KActionMenu *
GaduProtocol::protocolActions()
{
    return actionMenu_;
}

void
GaduProtocol::settingsChanged()
{
    userUin_ = prefs_->uin();
    password_ = prefs_->password();
}

void
GaduProtocol::slotLogin()
{
    if ( (userUin_ == 0) || password_.isEmpty() ) {
        KMessageBox::error( qApp->mainWidget(),
                            i18n("You must fill in UIN and password fields in the preferences dialog before you can login"),
                            i18n("Unable to Login") );
        return;
    }
    if ( !session_->isConnected() ) {
        session_->login( userUin_, password_, GG_STATUS_AVAIL );
        status_ = GG_STATUS_AVAIL;
        myself_->setGaduStatus( status_);
        changeStatus( status_ );
    } else {
        session_->changeStatus( GG_STATUS_AVAIL );
        status_ = GG_STATUS_AVAIL;
        myself_->setGaduStatus( status_);
        changeStatus( status_ );
    }
}

void
GaduProtocol::slotLogoff()
{
    if ( session_->isConnected() ) {
        status_ = 0;
        changeStatus( status_ );
    } else
        setStatusIcon( "gg_offline" );
}

void
GaduProtocol::addNotify( uin_t uin )
{
    if ( session_->isConnected() )
        session_->addNotify( uin );
}

void
GaduProtocol::notify( uin_t* userlist, int count )
{
    if ( session_->isConnected() )
        session_->notify( userlist, count );
}

void
GaduProtocol::sendMessage( uin_t recipient, const QString& msg, int msgClass )
{
    if ( session_->isConnected() )
        session_->sendMessage( recipient, msg, msgClass );
}

void
GaduProtocol::changeStatus( int status, const QString& descr )
{
    if ( !session_->isConnected() ) {
        slotLogin();
    }
    if ( descr.isEmpty() ) {
        session_->changeStatus( status );
    } else {
        session_->changeStatusDescription( status, descr );
    }
    status_ = status;
    myself_->setGaduStatus( status_ );

    switch( status_ ) {
    case GG_STATUS_NOT_AVAIL:
    case GG_STATUS_NOT_AVAIL_DESCR:
        setStatusIcon( "gg_away" );
        break;
    case GG_STATUS_AVAIL:
    case GG_STATUS_AVAIL_DESCR:
        setStatusIcon( "gg_online" );
        break;
    case GG_STATUS_BUSY:
    case GG_STATUS_BUSY_DESCR:
        setStatusIcon( "gg_busy" );
        break;
    case GG_STATUS_INVISIBLE:
    case GG_STATUS_INVISIBLE_DESCR:
        setStatusIcon( "gg_invi" );
        break;
    default:
        session_->logoff();
        setStatusIcon( "gg_offline" );
        break;
    }
}

void
GaduProtocol::error( const QString& title, const QString& message )
{
    KMessageBox::error( qApp->mainWidget(), message, title );
}

void
GaduProtocol::messageReceived( struct gg_event* e )
{
    kdDebug()<<"####"<<" Great! Message Received :: "<<((const char*)e->event.msg.message)<<endl;

    if ( !e->event.msg.message )
        return;

    if ( e->event.msg.sender == 0 ) {
        //system message, display them or not?
        kdDebug()<<"####"<<" System Message "<<endl;
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
GaduProtocol::ackReceived( struct gg_event* e )
{
    kdDebug()<<"####"<<"Received an ACK from "<<e->event.ack.recipient<<endl;
}

void
GaduProtocol::notify( struct gg_event* e )
{
    GaduContact *c;

    struct gg_notify_reply *n = e->event.notify;

    while( n && n->uin ) {
        kdDebug()<<"### NOTIFY "<<n->uin<<endl;
        if ( !(c=contactsMap_.find(n->uin).data()) ) {
            ++n;
            continue;
        }
        if ( c->gaduStatus() == (Q_UINT32)n->status )
            continue;
        c->setGaduStatus( n->status );
        ++n;
    }
}

void
GaduProtocol::notifyDescription( struct gg_event* e )
{
    GaduContact *c;
    struct gg_notify_reply *n;

    n =  e->event.notify_descr.notify;

    for( ; n->uin ; n++ ) {
        char *descr = (e->type == GG_EVENT_NOTIFY_DESCR) ? e->event.notify_descr.descr : NULL;
        if ( !(c=contactsMap_.find( n->uin ).data()) )
            continue;
        if ( c->gaduStatus() == (Q_UINT32)n->status )
            continue;
        c->setGaduStatus( n->status, descr );
    }
}

void
GaduProtocol::statusChanged( struct gg_event* e )
{
    GaduContact *c = contactsMap_.find( e->event.status.uin ).data();
    kdDebug()<<"### status changed"<<endl;
    if( !c )
        return;
    c->setGaduStatus( e->event.status.status, e->event.status.descr );
}

void
GaduProtocol::pong()
{
    kdDebug()<<"####"<<" Pong..."<<endl;
}

void
GaduProtocol::connectionFailed( struct gg_event* /*e*/ )
{
    KMessageBox::error( qApp->mainWidget(), i18n("Plugin unable to connect to the Gadu-Gadu server."),
                        i18n("Connection Error") );
    setStatusIcon( "gg_offline" );
}

void
GaduProtocol::connectionSucceed( struct gg_event* /*e*/ )
{
    kdDebug()<<"#### Gadu-Gadu connected!"<<endl;
    UserlistGetCommand *cmd = new UserlistGetCommand( this );
    cmd->setInfo( userUin_, password_ );
    connect( cmd, SIGNAL(done(const QStringList&)),
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
GaduProtocol::disconnect()
{
    pingTimer_->stop();
    changeStatus( 0 );
}

void
GaduProtocol::userlist( const QStringList& u )
{
    int i;
    QString name, group, uin;
    kdDebug()<<"### Got userlist"<<endl;
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
        addContact( uin, name, 0L, group );
    }
}

void
GaduProtocol::slotGoOnline()
{
    if ( !session_->isConnected() ) {
        kdDebug()<<"#### Connecting..."<<endl;
        slotLogin();
    } else
        changeStatus( GG_STATUS_AVAIL );
}

void
GaduProtocol::slotGoOffline()
{
    slotLogoff();
}

void
GaduProtocol::slotGoInvisible()
{
    changeStatus( GG_STATUS_INVISIBLE );
}

void
GaduProtocol::slotGoAway()
{
    changeStatus( GG_STATUS_NOT_AVAIL );
}

void
GaduProtocol::slotGoBusy()
{
    changeStatus( GG_STATUS_BUSY );
}

void
GaduProtocol::pingServer()
{
    session_->ping();
}

void GaduProtocol::serialize( KopeteMetaContact *metaContact)
{
	QStringList strList;
	for( KopeteContact *c = metaContact->contacts().first(); c ; c = metaContact->contacts().next() )
	{
		if ( c->protocol()->id() == this->id() )
		{
			GaduContact *g = static_cast<GaduContact*>(c);
			strList << g->name();
		}
	}
	metaContact->setPluginData(this , strList);
}

void
GaduProtocol::deserialize( KopeteMetaContact *metaContact,
                           const QStringList &strList )
{
    QString protocolId = this->id();

    QString uin, nick;
    int numContacts = strList.size();
    int idx = 0;
    QStringList uins = QStringList::split( "\n",
                                           metaContact->addressBookField( this, "messaging/gadu" ) );

    while( numContacts-- ) {
        nick = strList[ idx ];
        uin = uins[ idx++ ];
        addContact( uin, nick, metaContact );
    }
}

QStringList
GaduProtocol::addressBookFields() const
{
    return QStringList::split( "|", "|messaging/gadu|" );
}

#include "gaduprotocol.moc"

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
