#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>

#include "kopete.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "kopetemessagemanager.h"
#include "kopetewindow.h"
#include "kopetemessagemanagerfactory.h"
#include "statusbaricon.h"
#include "systemtray.h"

#include <qcursor.h>
#include <qtimer.h>

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
    userUin_ = KGlobal::config()->readEntry("Uin", "").toUInt();
    password_= KGlobal::config()->readEntry("Password", "");
    nick_    = KGlobal::config()->readEntry("Nick", "");
    myself_ = new GaduContact( this->id(), userUin_, nick_,
                               new KopeteMetaContact() );

    statusBarIcon_ = new StatusBarIcon();
    prefs_ = new GaduPreferences( "gadu_protocol", this );
    connect( prefs_, SIGNAL(saved()), this, SIGNAL(settingsChanged()) );

    initIcons();
    initActions();
    initConnections();

    statusBarIcon_->setPixmap( connectingIcon_ );
}

GaduProtocol::~GaduProtocol()
{
    protocolStatic_ = 0L;
}

GaduProtocol* GaduProtocol::protocolStatic_ = 0L;

void
GaduProtocol::initIcons()
{
    KIconLoader *loader = KGlobal::iconLoader();
    KStandardDirs dir;

    onlineIcon_ = QPixmap( loader->loadIcon("gg_online", KIcon::User) );
    offlineIcon_ = QPixmap( loader->loadIcon("gg_offline", KIcon::User) );
    awayIcon_ = QPixmap( loader->loadIcon("gg_away", KIcon::User) );
    busyIcon_ = QPixmap( loader->loadIcon("gg_away", KIcon::User) );
    invisibleIcon_ = QPixmap( loader->loadIcon("gg_ignored", KIcon::User) );
    connectingIcon_= QPixmap( loader->loadIcon("gg_connecting", KIcon::User) );
}

void
GaduProtocol::initActions()
{
    onlineAction_    = new KAction( i18n("Go O&nline"), "gg_online", 0, this,
                                    SLOT(slotGoOnline()), this, "actionGaduConnect" );
    offlineAction_   = new KAction( i18n("Go &Offline"), "gg_offline", 0, this,
                                    SLOT(slotGoOffline()), this, "actionGaduConnect" );
    awayAction_      = new KAction( i18n("Set &Away"), "gg_away", 0, this,
                                    SLOT(slotGoAway()), this, "actionGaduConnect" );
    busyAction_      = new KAction( i18n("Set &Busy"), "gg_away", 0, this,
                                    SLOT(slotGoBusy()), this, "actionGaduConnect" );
    invisibleAction_ = new KAction( i18n("Set &Invisible"), "gg_ignored", 0, this,
                                    SLOT(slotGoInvisible()), this, "actionGaduConnect" );

    actionMenu_ = new KActionMenu( "Gadu", this );

    actionMenu_->insert( onlineAction_ );
    actionMenu_->insert( offlineAction_ );
    actionMenu_->insert( awayAction_ );
    actionMenu_->insert( busyAction_ );
    actionMenu_->insert( invisibleAction_ );

    actionMenu_->plug( kopeteapp->systemTray()->contextMenu(), 1 );
}

void
GaduProtocol::initConnections()
{
    QObject::connect( statusBarIcon_, SIGNAL(rightClicked(const QPoint&)),
                      this, SLOT(slotIconRightClicked(const QPoint&)) );
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

bool
GaduProtocol::unload()
{
    if( kopeteapp->statusBar() ) {
        kopeteapp->statusBar()->removeWidget( statusBarIcon_ );
        delete statusBarIcon_;
    }
    return true;
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
GaduProtocol::addContact( const QString& uin, const QString& nick, KopeteMetaContact* parent )
{
    KopeteContactList *l = KopeteContactList::contactList();
    KopeteMetaContact *m;

    if ( !parent ) {
        m = l->findContact( this->id(), nick );
    } else
        m = parent;

    KopeteContact *c = m->findContact( this->id(), uin );

    if( !c ) {
        uin_t uinNumber = uin.toUInt();
        m->setAddressBookField( this, "Name", nick );
        m->setAddressBookField( this, "uin", uin );
        GaduContact *contact = new GaduContact( this->id(), uinNumber,
                                                nick, m );
        m->addContact( contact, m->groups().first() );
        contactsMap_.insert( uinNumber, contact );
        addNotify( uinNumber );
    }
}

void
GaduProtocol::removeContact( const GaduContact* c )
{
    const uin_t u = c->uin();
    contactsMap_.remove( u );
}

void
GaduProtocol::slotIconRightClicked( const QPoint& /*p*/ )
{
    KPopupMenu *popup = new KPopupMenu( statusBarIcon_ );
    popup->insertTitle( this->id() );
    onlineAction_->plug( popup );
    busyAction_->plug( popup );
    awayAction_->plug( popup );
    invisibleAction_->plug( popup );
    offlineAction_->plug( popup );

    popup->popup( QCursor::pos() );
}

void
GaduProtocol::slotLogin()
{
    if ( !session_->isConnected() ) {
        session_->login( userUin_, password_ );
    } else {
        session_->changeStatus( GG_STATUS_AVAIL );
        status_ = GG_STATUS_AVAIL;
        myself_->setGaduStatus( status_ );
        changeStatus( status_ );
    }
}

void
GaduProtocol::slotLogoff()
{
    if ( session_->isConnected() ) {
        status_ = 0;
        changeStatus( 0 );
        session_->logoff();
    }
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
    if ( session_->isConnected() ) {
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
            statusBarIcon_->setPixmap( awayIcon_ );
            break;
        case GG_STATUS_AVAIL:
        case GG_STATUS_AVAIL_DESCR:
            statusBarIcon_->setPixmap( onlineIcon_ );
            break;
        case GG_STATUS_BUSY:
        case GG_STATUS_BUSY_DESCR:
            statusBarIcon_->setPixmap( busyIcon_ );
            break;
        case GG_STATUS_INVISIBLE:
        case GG_STATUS_INVISIBLE_DESCR:
            statusBarIcon_->setPixmap( invisibleIcon_ );
            break;
        default:
            statusBarIcon_->setPixmap( offlineIcon_ );
            break;
        }
    } else
        statusBarIcon_->setPixmap( offlineIcon_ );
}

void
GaduProtocol::error( const QString& title, const QString& message )
{
    KMessageBox::error( kopeteapp->mainWindow(), message, title );
}

void
GaduProtocol::messageReceived( struct gg_event* e )
{
    kdDebug()<<"####"<<" Great! Message Received :: "<<((const char*)e->event.msg.message)<<endl;

    if ( !e->event.msg.message )
        return;

    if ( e->event.msg.sender == 0 ) {
        //system message
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
        addContact( QString::number(e->event.msg.sender), (const char*)e->event.msg.message );
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

    while( n->uin ) {
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
    struct gg_notify_reply *n = e->event.notify_descr.notify;

    while( n->uin ) {
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
GaduProtocol::statusChanged( struct gg_event* e )
{
    GaduContact *c = contactsMap_.find( e->event.status.uin ).data();
    kdDebug()<<"### status changed"<<endl;
    if( !c )
        return;
    c->setGaduStatus( e->event.status.status );
}

void
GaduProtocol::pong()
{
    kdDebug()<<"####"<<" Pong..."<<endl;
}

void
GaduProtocol::connectionFailed( struct gg_event* /*e*/ )
{
    KMessageBox::error( kopeteapp->mainWindow(), i18n("Plugin couldn't connect to the Gadu-Gadu server."),
                        i18n("Connection error") );
}

void
GaduProtocol::connectionSucceed( struct gg_event* /*e*/ )
{
    kdDebug()<<"#### Gadu-Gadu connected!"<<endl;
    //FIXME: remember last state and set it appropriately
    changeStatus( GG_STATUS_INVISIBLE );
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
    kdDebug()<<"### Disconnected!"<<endl;
    pingTimer_->stop();
    changeStatus( 0 );
}

void
GaduProtocol::userlist( const QStringList& u )
{
    int i;
    QString name, group, uin;
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
        addContact( uin, name );
    }
}

void
GaduProtocol::slotGoOnline()
{
    if ( !session_->isConnected() ) {
        kdDebug()<<"#### Connecting..."<<endl;
        slotLogin();
    }
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

bool
GaduProtocol::serialize( KopeteMetaContact *metaContact,
                             QStringList &strList ) const
{
    KopeteContact *c;
    bool done = false;
    for( c = metaContact->contacts().first(); c ; c = metaContact->contacts().next() ) {
        if ( c->protocol() == this->id() ) {
            kdDebug() << "*** Do it!" << endl;
            GaduContact *g = static_cast<GaduContact*>(c);
            strList << g->name() << QString::number(g->uin());
            done = true;
        }
    }

    return done;
}

void
GaduProtocol::deserialize( KopeteMetaContact *metaContact,
                           const QStringList &strList )
{
    QString protocolId = this->id();

    QString uin, nick;
    int numContacts = strList.size() / 2;
    int idx = 0;
    while( numContacts ) {
        for( int i = 0; i < 2; ++i,++idx ) {
            switch( i ) {
            case 0:
                nick = strList[idx];
                break;
            case 1:
                uin = strList[idx];
                break;
            }
        }
        --numContacts;
        addContact( uin, nick, metaContact );
    }
}

QStringList
GaduProtocol::addressBookFields() const
{
    return QStringList::split( "|", "|Name|messaging/gadu|" );
}

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
