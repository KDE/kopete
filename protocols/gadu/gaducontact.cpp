#include <kpopupmenu.h>
#include <kaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <qstring.h>

#include "kopetemessagemanagerfactory.h"
#include "kopetemessagemanager.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "kopeteaway.h"
#include "userinfodialog.h"
using Kopete::UserInfoDialog;

#include "gaduprotocol.h"
#include "gaducontact.h"

GaduContact::GaduContact( const QString& /*protocolId*/, uin_t uin,
                          const QString& name, KopeteMetaContact* parent )
    : KopeteContact( GaduProtocol::protocol(), QString::number( uin ), parent )
{
    msgManager_ = 0L;
    uin_ = uin;
    protocol_ = GaduProtocol::protocol();
    status_ = 0;

    initActions();

    setDisplayName( name );
    thisContact_.append( this );
}

QString
GaduContact::statusText() const
{
    switch( status_ ) {
    case GG_STATUS_NOT_AVAIL:
    case GG_STATUS_NOT_AVAIL_DESCR:
        return i18n("Unavailable");
        break;
    case GG_STATUS_BUSY:
    case GG_STATUS_BUSY_DESCR:
        return i18n("Busy");
        break;
    case GG_STATUS_INVISIBLE:
    case GG_STATUS_INVISIBLE_DESCR:
        return i18n("Invisible");
        break;
    case GG_STATUS_AVAIL:
    case GG_STATUS_AVAIL_DESCR:
        return i18n("Online");
        break;
    default:
        return i18n("Offline");
        break;
    }
}

QString
GaduContact::statusIcon() const
{
    switch( status_ ) {
    case GG_STATUS_NOT_AVAIL:
        return "gg_away";
        break;
    case GG_STATUS_NOT_AVAIL_DESCR:
        return "gg_away";
        break;
    case GG_STATUS_BUSY:
        return "gg_busy";
        break;
    case GG_STATUS_BUSY_DESCR:
        return "gg_busy";
        break;
    case GG_STATUS_INVISIBLE:
        return "gg_invi";
        break;
    case GG_STATUS_INVISIBLE_DESCR:
        return "gg_invi";
        break;
    case GG_STATUS_AVAIL:
        return "gg_online";
        break;
    case GG_STATUS_AVAIL_DESCR:
        return "gg_online";
        break;
    default:
        return "gg_offline";
        break;
    }
}

int
GaduContact::importance() const
{
    switch( status_ ) {
    case GG_STATUS_NOT_AVAIL:
        return 13;
    case GG_STATUS_NOT_AVAIL_DESCR:
        return 14;
        break;
    case GG_STATUS_BUSY:
        return 15;
    case GG_STATUS_BUSY_DESCR:
        return 16;
        break;
    case GG_STATUS_INVISIBLE:
        return 5;
    case GG_STATUS_INVISIBLE_DESCR:
        return 6;
        break;
    case GG_STATUS_AVAIL:
        return 19;
    case GG_STATUS_AVAIL_DESCR:
        return 20;
        break;
    default:
        return 0;
        break;
    }
}

QString
GaduContact::data() const
{
    return QString::number( uin_ );
}

QString
GaduContact::identityId() const
{
    return parentIdentity_;
}

void
GaduContact::setParentIdentity( const QString& id)
{
    parentIdentity_ = id;
}

void
GaduContact::setGaduStatus( Q_UINT32 status, const QString& descr )
{
    status_ = status;
    description_ = descr;

    switch( status_ )
    {
    case GG_STATUS_NOT_AVAIL:
    case GG_STATUS_NOT_AVAIL_DESCR:
    case GG_STATUS_BUSY:
    case GG_STATUS_BUSY_DESCR:
    case GG_STATUS_INVISIBLE:
    case GG_STATUS_INVISIBLE_DESCR:
        setOnlineStatus( Away );
        break;
    case GG_STATUS_AVAIL:
    case GG_STATUS_AVAIL_DESCR:
        setOnlineStatus( Online );
        break;
    default:
        setOnlineStatus( Offline );
        break;
    }
}

QString
GaduContact::description() const
{
    return description_;
}

Q_UINT32
GaduContact::gaduStatus() const
{
    return status_;
}

uin_t
GaduContact::uin() const
{
    return uin_;
}

KopeteMessageManager*
GaduContact::manager( bool )
{
    if ( msgManager_ ) {
        return msgManager_;
    } else {
        msgManager_ = KopeteMessageManagerFactory::factory()->create( GaduProtocol::protocol()->myself(),
                                                           thisContact_, GaduProtocol::protocol());
        connect( msgManager_, SIGNAL(messageSent(KopeteMessage&, KopeteMessageManager*)),
                 this, SLOT(messageSend(KopeteMessage&, KopeteMessageManager*)) );
	connect( msgManager_, SIGNAL(destroyed()),
                 this, SLOT(slotMessageManagerDestroyed()) );
        return msgManager_;
    }
}

void
GaduContact::slotMessageManagerDestroyed()
{
    msgManager_ = 0L;
}

void
GaduContact::initActions()
{
    actionSendMessage_ = KopeteStdAction::sendMessage(this, SLOT(execute()),
                                                      this, "actionMessage" );
    actionInfo_ = KopeteStdAction::contactInfo( this, SLOT(slotUserInfo()),
                                                this, "actionInfo" );
    /*actionRemove_ = KopeteStdAction::deleteContact( this, SLOT(removeThisUser()),
      this, "actionDelete" );*/
}

void
GaduContact::messageReceived( KopeteMessage& msg )
{
    manager()->appendMessage( msg );
}

void
GaduContact::messageSend( KopeteMessage& msg, KopeteMessageManager* mgr )
{
    if ( msg.body().isEmpty() )
        return;
    //FIXME: handle colors
    GaduProtocol::protocol()->sendMessage( uin_, msg.plainBody().local8Bit() );
    mgr->appendMessage( msg );
}

bool
GaduContact::isReachable()
{
    return false;
}

KActionCollection *
GaduContact::customContextMenuActions()
{
    return 0L;
}

void
GaduContact::slotUserInfo()
{
    UserInfoDialog *dlg = new UserInfoDialog( i18n("Gadu contact") );

    dlg->setName( metaContact()->displayName() );
    dlg->setId( QString::number( uin_ ) );
    dlg->setStatus( statusText() );
    dlg->setAwayMessage( description_ );
    dlg->show();
}

void
GaduContact::slotDeleteContact()
{
    GaduProtocol::protocol()->removeContact( this );
}

#include "gaducontact.moc"

/*
 * Local variables:
 * c-indentation-style: bsd
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 *
 * vim: set et ts=4 sts=4 sw=4:
 */
