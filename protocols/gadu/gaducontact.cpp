// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
#include <klocale.h>
#include <kdebug.h>

#include "kopetemessagemanagerfactory.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "userinfodialog.h"
using Kopete::UserInfoDialog;

#include "gaduprotocol.h"
#include "gaducontact.h"

GaduContact::GaduContact( uin_t uin, const QString& name, KopeteAccount *account,
					KopeteMetaContact* parent )
	: KopeteContact( account, QString::number( uin ), parent )
{
	msgManager_ = 0L;
	uin_ = uin;
	account_ = static_cast<GaduAccount *>(account);
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

/*
*/
void
GaduContact::setInfo( const QString &email, const QString &firstName, 
			 const QString &secondName,
			 const QString &nickName, const QString &phonenr )
{
	if (email.length())
	    email_	= email;
	if (firstName.length())
	    firstName_	= firstName;
	if (secondName.length())
	    secondName_	= secondName;
	if (nickName.length())
	    nickName_	= nickName;
	if (phonenr.length())
	    phonenr_	= phonenr;
}


void
GaduContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &)
{

    serializedData["email"]	= email_;
    serializedData["FirstName"]	= firstName_;
    serializedData["SecondName"]= secondName_;
    serializedData["NickName"]	= nickName_;
    serializedData["telephone"]	= phonenr_;
    
}

contactLine *
GaduContact::contactDetails()
{
	KopeteGroupList		groupList;
	QString			groups;

	contactLine *cl	= new contactLine;

	cl->firstname	= firstName_;
	cl->surname	= secondName_;
	cl->nickname	= nickName_;
	cl->name		= firstName_+" "+secondName_;
	cl->phonenr	= phonenr_;
	cl->uin		= QString::number( uin_ );
	cl->email		= email_;
	cl->name		= displayName();
	
	groupList = metaContact ()->groups ();
	
	KopeteGroup * gr;
	for ( gr = groupList.first (); gr ; gr = groupList.next ()){
		if ( gr!=KopeteGroup::topLevel() ){
			groups += gr->displayName()+",";
		}
	}
	
	if (groups.length()){
		groups.truncate( groups.length()-1 );
	}
	cl->group	= groups;
	
	return cl;
}


void GaduContact::messageAck()
{
	manager()->messageSucceeded();
}

#include "gaducontact.moc"
