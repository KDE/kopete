//////////////////////////////////////////////////////////////////////////////
// gaduprotocol.cpp                                                         //
//                                                                          //
// Copyright (C)  2002  Zack Rusin <zack@kde.org>                           //
//                                                                          //
// This program is free software; you can redistribute it and/or            //
// modify it under the terms of the GNU General Public License              //
// as published by the Free Software Foundation; either version 2           //
// of the License, or (at your option) any later version.                   //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU General Public License for more details.                             //
//                                                                          //
// You should have received a copy of the GNU General Public License        //
// along with this program; if not, write to the Free Software              //
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA                //
// 02111-1307, USA.                                                         //
//////////////////////////////////////////////////////////////////////////////
#include <qapplication.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qstringlist.h>

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
#include "kopetegroup.h"

#include "gaduprotocol.h"
#include "gaducontact.h"
#include "gaduaddcontactpage.h"
#include "gadupreferences.h"
#include "gadusession.h"
#include "gaducommands.h"

#include <libgadu.h>

K_EXPORT_COMPONENT_FACTORY( kopete_gadu, KGenericFactory<GaduProtocol> );

GaduProtocol::GaduProtocol( QObject* parent, const char* name, const QStringList & )
:   KopeteProtocol( parent, name ),
    GaduStatusOffline(        KopeteOnlineStatus::Offline, 25, this, 0x0,                       "gg_offline", i18n( "Go O&ffline" ),   i18n( "Online" ) ),
    GaduStatusNotAvail(       KopeteOnlineStatus::Away,    15, this, GG_STATUS_NOT_AVAIL,       "gg_away",    i18n( "Go A&way" ),      i18n( "Unavailable" ) ),
    GaduStatusNotAvailDescr(  KopeteOnlineStatus::Away,    20, this, GG_STATUS_NOT_AVAIL_DESCR, "gg_away",    i18n( "Go A&way" ),      i18n( "Unavailable" ) ),
    GaduStatusBusy(           KopeteOnlineStatus::Away,    20, this, GG_STATUS_BUSY,            "gg_busy",    i18n( "Go B&usy" ),      i18n( "Busy" ) ),
    GaduStatusBusyDescr(      KopeteOnlineStatus::Away,    25, this, GG_STATUS_BUSY_DESCR,      "gg_busy",    i18n( "Go B&usy" ),      i18n( "Busy" ) ),
    GaduStatusInvisible(      KopeteOnlineStatus::Away,     5, this, GG_STATUS_INVISIBLE,       "gg_invi",    i18n( "Go I&nvisible" ), i18n( "Invisible" ) ),
    GaduStatusInvisibleDescr( KopeteOnlineStatus::Away,    10, this, GG_STATUS_INVISIBLE_DESCR, "gg_invi",    i18n( "Go I&nvisible" ), i18n( "Invisible" ) ),
    GaduStatusAvail(          KopeteOnlineStatus::Online,  20, this, GG_STATUS_AVAIL,           "gg_online",  i18n( "Go O&nline" ),    i18n( "Online" ) ),
    GaduStatusAvailDescr(     KopeteOnlineStatus::Online,  25, this, GG_STATUS_AVAIL_DESCR,     "gg_online",  i18n( "Go O&nline" ),    i18n( "Online" ) )
{
    if ( protocolStatic_ )
        kdDebug(14100)<<"####"<<"GaduProtocol already initialized"<<endl;
    else
        protocolStatic_ = this;

    pingTimer_ = 0;

    session_ = new GaduSession( this, "GaduSession" );

    KGlobal::config()->setGroup("Gadu");
    userUin_ = KGlobal::config()->readEntry("Uin", "0").toUInt();
    password_= KGlobal::config()->readEntry("Password", "");
    nick_    = KGlobal::config()->readEntry("Nick", "");
    myself_ = new GaduContact( pluginId(), userUin_, nick_,
                               new KopeteMetaContact() );

    prefs_ = new GaduPreferences( "gadu_protocol", this );
    QObject::connect( prefs_, SIGNAL(saved()), this, SLOT(settingsChanged()) );

    initActions();
    initConnections();

    setStatusIcon( "gg_connecting" );

    if( KGlobal::config()->readBoolEntry("AutoConnect", false) )
        slotGoOnline();

    addAddressBookField( "messaging/gadu", KopetePlugin::MakeIndexField );
}

GaduProtocol::~GaduProtocol()
{
    protocolStatic_ = 0L;
}

GaduProtocol* GaduProtocol::protocolStatic_ = 0L;

const
QString GaduProtocol::protocolIcon( )
{
	return "gg_online";
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
    busyAction_      = new KAction( i18n("Set &Busy"), "gg_busy", 0, this,
                                    SLOT(slotGoBusy()), this, "actionGaduConnect" );
    invisibleAction_ = new KAction( i18n("Set &Invisible"), "gg_invi", 0, this,
                                    SLOT(slotGoInvisible()), this, "actionGaduConnect" );

    actionMenu_ = new KActionMenu( "Gadu", this );

    actionMenu_->popupMenu()->insertTitle( pluginId() );

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
                      SLOT(slotSessionDisconnect()) );
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

void
GaduProtocol::connect()
{
    slotGoOnline();
}

void
GaduProtocol::disconnect()
{
    slotGoOffline();
}

bool
GaduProtocol::isConnected() const
{
    return session_->isConnected();
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
    return myself_->onlineStatus().status() == KopeteOnlineStatus::Away;
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

bool GaduProtocol::addContactToMetaContact( const QString &contactId, const QString &displayName,
                                            KopeteMetaContact* parentContact )
{
    uin_t uinNumber = contactId.toUInt();

    QStringList l = parentContact->groups().toStringList();

    if ( !parentContact->findContact( pluginId(), myself_->contactId(), contactId ) ) {
        GaduContact *newContact = new GaduContact( pluginId(), uinNumber, displayName, parentContact );
        newContact->setParentIdentity( QString::number( userUin_ ) );
        contactsMap_.insert( uinNumber, newContact );
        addNotify( uinNumber );
    }

    return true;
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
GaduProtocol::slotLogoff()
{
    if ( session_->isConnected() ) {
        status_ = GaduStatusOffline;
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
GaduProtocol::changeStatus( const KopeteOnlineStatus &status, const QString& descr )
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

    setStatusIcon( status_.icon() );
}

void
GaduProtocol::error( const QString& title, const QString& message )
{
    KMessageBox::error( qApp->mainWidget(), message, title );
}

void
GaduProtocol::messageReceived( struct gg_event* e )
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
GaduProtocol::ackReceived( struct gg_event* /* e */ )
{
    //kdDebug(14100)<<"####"<<"Received an ACK from "<<e->event.ack.recipient<<endl;
}

void
GaduProtocol::notify( struct gg_event* e )
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
GaduProtocol::notifyDescription( struct gg_event* e )
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
GaduProtocol::statusChanged( struct gg_event* e )
{
    GaduContact *c = contactsMap_.find( e->event.status.uin ).data();
    if( !c )
        return;
    c->setDescription( e->event.status.descr );
    c->setOnlineStatus( convertStatus( e->event.status.status ) );
}

void
GaduProtocol::pong()
{
    //kdDebug(14100)<<"####"<<" Pong..."<<endl;
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
GaduProtocol::slotSessionDisconnect()
{
    pingTimer_->stop();
    changeStatus( GaduStatusOffline );
}

void
GaduProtocol::userlist( const QStringList& u )
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
GaduProtocol::slotGoOnline()
{
    if ( !session_->isConnected() ) {
        kdDebug(14100)<<"#### Connecting..."<<endl;
        slotLogin();
    } else
        changeStatus( GaduStatusAvail );
}

void
GaduProtocol::slotGoOffline()
{
    slotLogoff();
}

void
GaduProtocol::slotGoInvisible()
{
    changeStatus( GaduStatusInvisible );
}

void
GaduProtocol::slotGoAway()
{
    changeStatus( GaduStatusNotAvail );
}

void
GaduProtocol::slotGoBusy()
{
    changeStatus( GaduStatusBusy );
}

void
GaduProtocol::pingServer()
{
    session_->ping();
}

void
GaduProtocol::deserializeContact( KopeteMetaContact *metaContact,
                                  const QMap<QString, QString> &serializedData,
                                  const QMap<QString, QString> & /* addressBookData */ )
{
    //kdDebug()<<"Adding "<<serializedData[ "contactId" ]<<" || "<< serializedData[ "displayName" ] <<endl;
    addContact( serializedData[ "contactId" ], serializedData[ "displayName" ], metaContact );
}

KopeteOnlineStatus
GaduProtocol::convertStatus( uint status )
{
    switch( status )
    {
    case GG_STATUS_NOT_AVAIL:
        return GaduStatusNotAvail;
    case GG_STATUS_NOT_AVAIL_DESCR:
        return GaduStatusNotAvailDescr;
    case GG_STATUS_BUSY:
        return GaduStatusBusy;
    case GG_STATUS_BUSY_DESCR:
        return GaduStatusBusyDescr;
    case GG_STATUS_INVISIBLE:
        return GaduStatusInvisible;
    case GG_STATUS_INVISIBLE_DESCR:
        return GaduStatusInvisibleDescr;
    case GG_STATUS_AVAIL:
        return GaduStatusAvail;
    case GG_STATUS_AVAIL_DESCR:
        return GaduStatusAvailDescr;
    default:
        return GaduStatusOffline;
    }
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
