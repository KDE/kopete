#include <klocale.h>
#include <kdebug.h>


#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "userinfodialog.h"
using Kopete::UserInfoDialog;

#include "gaduprotocol.h"
#include "gaduaccount.h"
#include "gaducontact.h"

GaduContact::GaduContact( uin_t uin, const QString& name, GaduAccount *account,
                          KopeteMetaContact* parent )
	: KopeteContact( account, QString::number( uin ), parent )
{
	msgManager_ = 0L;
	uin_ = uin;
	account_ = account;
	//offline
	setOnlineStatus( GaduProtocol::protocol()->convertStatus( 0 ) );

	initActions();

	setDisplayName( name );
	thisContact_.append( this );
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
GaduContact::setDescription( const QString& descr )
{
	description_ = descr;
}

QString
GaduContact::description() const
{
	return description_;
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
		msgManager_ = KopeteMessageManagerFactory::factory()->create( account_->myself(),
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
}

void
GaduContact::messageReceived( KopeteMessage& msg )
{
	manager()->appendMessage( msg );
}

void
GaduContact::messageSend( KopeteMessage& msg, KopeteMessageManager* mgr )
{
	if ( msg.plainBody().isEmpty() )
		return;
	//FIXME: handle colors
	account_->sendMessage( uin_, msg.plainBody() );
	mgr->appendMessage( msg );
}

bool
GaduContact::isReachable()
{
	return true;
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
	dlg->setStatus( onlineStatus().description() );
	dlg->setAwayMessage( description_ );
	dlg->show();
}

void
GaduContact::slotDeleteContact()
{
	account_->removeContact( this );
	deleteLater();
}

#include "gaducontact.moc"
