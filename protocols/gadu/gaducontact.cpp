// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 	2002-2003	 Zack Rusin 	<zack@kde.org>
//
// gaducontact.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include <klocale.h>
#include <kaction.h>
#include <kdebug.h>

#include "gaduprotocol.h"
#include "gaducontact.h"
#include "gadupubdir.h"

#include "kopetemessagemanagerfactory.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"

#include "userinfodialog.h"

using Kopete::UserInfoDialog;

GaduContact::GaduContact( uin_t uin, const QString& name, KopeteAccount* account, KopeteMetaContact* parent )
: KopeteContact( account, QString::number( uin ), parent )
{
	msgManager_ = 0L;
	uin_ = uin;
	account_ = static_cast<GaduAccount*>( account );

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
	}
	else {
		msgManager_ = KopeteMessageManagerFactory::factory()->create( account_->myself(),
												thisContact_, GaduProtocol::protocol());
		connect( msgManager_, SIGNAL( messageSent( KopeteMessage&, KopeteMessageManager*) ),
						 this, SLOT( messageSend( KopeteMessage&, KopeteMessageManager*) ) );
		connect( msgManager_, SIGNAL( destroyed() ),  this, SLOT( slotMessageManagerDestroyed() ) );
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
	actionSendMessage_	= KopeteStdAction::sendMessage( this, SLOT( execute() ), this, "actionMessage" );
	actionInfo_		= KopeteStdAction::contactInfo( this, SLOT( slotUserInfo() ), this, "actionInfo" );
}

void
GaduContact::messageReceived( KopeteMessage& msg )
{
	manager()->appendMessage( msg );
}

void
GaduContact::messageSend( KopeteMessage& msg, KopeteMessageManager* mgr )
{
	if ( msg.plainBody().isEmpty() ) {
		return;
	}
	//FIXME: handle ritch text
	account_->sendMessage( uin_, msg.plainBody() );
	mgr->appendMessage( msg );
}

bool
GaduContact::isReachable()
{
	return account_->isConnected();
}

QPtrList<KAction>*
GaduContact::customContextMenuActions()
{
	QPtrList<KAction> *fakeCollection = new QPtrList<KAction>();
	//show profile
	KAction* actionShowProfile = new KAction( i18n("Show Profile") , "info", 0,  this, SLOT( slotShowPublicProfile() ),
										this, "actionShowPublicProfile" );

	fakeCollection->append( actionShowProfile );

	return fakeCollection;
}

void
GaduContact::slotShowPublicProfile()
{
	account_->slotSearch( uin_ );
}

void
GaduContact::slotUserInfo()
{
	/// FIXME: use more decent information here
	UserInfoDialog *dlg = new UserInfoDialog( i18n( "Gadu contact" ) );

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

void
GaduContact::serialize( QMap<QString, QString>& serializedData, QMap<QString, QString>& )
{
	serializedData[ "email" ]	= property( "emailAddress" ).value().toString();
	serializedData[ "FirstName"  ]	= property( "firstName" ).value().toString();
	serializedData[ "SecondName" ]	= property( "lastName" ).value().toString();
	serializedData[ "telephone" ]	= property( "privPhoneNum" ).value().toString();
	serializedData[ "ignored" ]	= property( "ignored" ).value().toString();
	serializedData[ "nickname" ]	= property( "nickName" ).value().toString();
}

contactLine*
GaduContact::contactDetails()
{
	KopeteGroupList		groupList;
	QString			groups;

	contactLine* cl	= new contactLine;

	cl->firstname	= property( "firstName" ).value().toString();
	cl->surname	= property( "lastName" ).value().toString();
	cl->nickname	= property( "nickName" ).value().toString();
	
	cl->email	= property( "emailAddress" ).value().toString();
	cl->phonenr	= property( "privPhoneNum" ).value().toString();
	
	cl->ignored	= ( property( "ignored" ).value().toString() == "true" );
	
	cl->uin		= QString::number( uin_ );
	cl->displayname	= metaContact()->displayName();

	groupList = metaContact()->groups();

	KopeteGroup* gr;
	for ( gr = groupList.first (); gr ; gr = groupList.next () ) {
// if present in any group, don't export to top level
// FIXME: again, probably bug in libkopete
// in case of topLevel group, KopeteGroup::displayName() returns "TopLevel" ineasted of just " " or "/"
// imo TopLevel group should be detected like i am doing that below
		if ( gr!=KopeteGroup::topLevel() ) {
			groups += gr->displayName()+",";
		}
	}

	if ( groups.length() ) {
		groups.truncate( groups.length()-1 );
	}
	cl->group = groups;

	return cl;
}


void GaduContact::messageAck()
{
	manager()->messageSucceeded();
}

#include "gaducontact.moc"
