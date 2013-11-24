// -*- Mode: c++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#include "gaducontact.h"

#include <klocale.h>
#include <kaction.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kicon.h>

#include "gaduaccount.h"
#include "gaduprotocol.h"
#include "gadupubdir.h"
#include "gadueditcontact.h"
#include "gaducontactlist.h"
#include "gadusession.h"

#include "kopetechatsessionmanager.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"
#include "kopetestdaction.h"
#include "kopeteuiglobal.h"

//#include "userinfodialog.h"

//using Kopete::UserInfoDialog;

GaduContact::GaduContact( uin_t uin, Kopete::Account* account, Kopete::MetaContact* parent )
: Kopete::Contact( account, QString::number( uin ), parent ), uin_( uin )
{
	msgManager_ = 0L;
	account_ = static_cast<GaduAccount*>( account );

	remote_port	= 0;
	version		= 0;
	image_size	= 0;
	// let us not ignore the contact by default right? causes ugly bug if
	// setContactDetails is not run on a contact right after it is added
	ignored_	= false;

	thisContact_.append( this );

	initActions();

	// don't call libkopete functions like these until the object is fully
	// constructed. all GaduContact construction must be above this point.
	setFileCapable( true );

	//offline
	setOnlineStatus( GaduProtocol::protocol()->convertStatus( 0 ) );
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

uin_t
GaduContact::uin() const
{
	return uin_;
}

void
GaduContact::sendFile( const KUrl &sourceURL, const QString &/*fileName*/, uint /*fileSize*/ )
{
	QString filePath;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
		filePath = KFileDialog::getOpenFileName( KUrl(), "*", 0l  , i18n("Kopete File Transfer"));
	else
		filePath = sourceURL.path(KUrl::RemoveTrailingSlash);

	kDebug(14120) << "File chosen to send:" << filePath;

	account_->sendFile( this, filePath );
}


void
GaduContact::changedStatus( KGaduNotify* newstatus )
{
	setOnlineStatus( GaduProtocol::protocol()->convertStatus( newstatus->status ) );
	setStatusMessage( newstatus->description );

	remote_ip	= newstatus->remote_ip;
	remote_port	= newstatus->remote_port;
	version		= newstatus->version;
	image_size	= newstatus->image_size;

	setFileCapable( newstatus->fileCap );

	kDebug(14100) << "uin:" << uin() << " port: " << remote_port << " remote ip: " <<  remote_ip.toIPv4Address() << " image size: " << image_size << "  version: "  << version;

}

QHostAddress&
GaduContact::contactIp()
{
	return remote_ip;
}

unsigned short
GaduContact::contactPort()
{
	return remote_port;
}

Kopete::ChatSession*
GaduContact::manager( Kopete::Contact::CanCreateFlags canCreate )
{
	if ( !msgManager_ && canCreate ) {
		msgManager_ = Kopete::ChatSessionManager::self()->create( account_->myself(), thisContact_,
				GaduProtocol::protocol() );
		connect( msgManager_, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
			 this, SLOT(messageSend(Kopete::Message&,Kopete::ChatSession*)) );
		connect( msgManager_, SIGNAL(destroyed()),  this, SLOT(slotChatSessionDestroyed()) );

	}
	kDebug(14100) << "GaduContact::manager returning:  " << msgManager_;
	return msgManager_;
}

void
GaduContact::slotChatSessionDestroyed()
{
	msgManager_ = 0L;
}

void
GaduContact::initActions()
{
}

void
GaduContact::messageReceived( Kopete::Message& msg )
{
	manager(Kopete::Contact::CanCreate)->appendMessage( msg );
}

void
GaduContact::messageSend( Kopete::Message& msg, Kopete::ChatSession* mgr )
{
	if ( msg.plainBody().isEmpty() ) {
		return;
	}
	mgr->appendMessage( msg );
	account_->sendMessage( uin_, msg );
}

bool
GaduContact::isReachable()
{
	return account_->isConnected();
}

QList<KAction*>*
GaduContact::customContextMenuActions()
{
	QList<KAction*> *fakeCollection = new QList<KAction*>();
	//show profile
	KAction* actionShowProfile = new KAction( KIcon("help-about"), i18n("Show Profile"), this );
	//, "actionShowPublicProfile" );
	connect( actionShowProfile, SIGNAL(triggered(bool)), this, SLOT(slotShowPublicProfile()) );

	fakeCollection->append( actionShowProfile );

	KAction* actionEditContact = new KAction( KIcon("document-properties"), i18n("Edit..."), this );
	//, "actionEditContact" );
	connect( actionEditContact, SIGNAL(triggered(bool)), this, SLOT(slotEditContact()) );

	fakeCollection->append( actionEditContact );

	return fakeCollection;
}

void
GaduContact::slotEditContact()
{
	new GaduEditContact( static_cast<GaduAccount*>(account()), this, Kopete::UI::Global::mainWidget() );
}

void
GaduContact::slotShowPublicProfile()
{
	account_->slotSearch( uin_ );
}

void
GaduContact::slotUserInfo()
{
// FIXME: there is no UserInfoDialog anymore

	/// FIXME: use more decent information here
//	UserInfoDialog *dlg = new UserInfoDialog( i18n( "Gadu contact" ) );

//	dlg->setName( metaContact()->displayName() );
//	dlg->setId( QString::number( uin_ ) );
//	dlg->setStatus( onlineStatus().description() );
//	dlg->setAwayMessage( description_ );
//	dlg->show();
}

void
GaduContact::deleteContact()
{
	if ( account_->isConnected() ) {
		account_->removeContact( this );
		deleteLater();
	}
	else {
		KMessageBox::error( Kopete::UI::Global::mainWidget(),
				i18n( "<qt>You need to go online to remove a contact from your contact list.</qt>" ),
				i18n( "Gadu-Gadu Plugin" ));
	}
}

void
GaduContact::serialize( QMap<QString, QString>& serializedData, QMap<QString, QString>& )
{
	serializedData[ "email" ]	= property( GaduProtocol::protocol()->propEmail ).value().toString();
	serializedData[ "FirstName"  ]	= property( GaduProtocol::protocol()->propFirstName ).value().toString();
	serializedData[ "SecondName" ]	= property( GaduProtocol::protocol()->propLastName ).value().toString();
	serializedData[ "telephone" ]	= property( GaduProtocol::protocol()->propPhoneNr ).value().toString();
	serializedData[ "ignored" ]	= ignored_ ? "true" : "false";
}

bool
GaduContact::setContactDetails( const GaduContactsList::ContactLine* cl )
{
	setProperty( GaduProtocol::protocol()->propEmail, cl->email );
	setProperty( GaduProtocol::protocol()->propFirstName, cl->firstname );
	setProperty( GaduProtocol::protocol()->propLastName, cl->surname );
	setProperty( GaduProtocol::protocol()->propPhoneNr, cl->phonenr );
	//setProperty( "ignored", i18n( "ignored" ), cl->ignored ? "true" : "false" );
	ignored_ = cl->ignored;
	//setProperty( "nickName", i18n( "nickname" ), cl->nickname );

	return true;
}

GaduContactsList::ContactLine*
GaduContact::contactDetails()
{
	Kopete::GroupList		groupList;
	QString			groups;

	GaduContactsList::ContactLine* cl = new GaduContactsList::ContactLine;

	cl->firstname	= property( GaduProtocol::protocol()->propFirstName ).value().toString();
	cl->surname	= property( GaduProtocol::protocol()->propLastName ).value().toString();
	//cl->nickname	= property( "nickName" ).value().toString();
	cl->email	= property( GaduProtocol::protocol()->propEmail ).value().toString();
	cl->phonenr	= property( GaduProtocol::protocol()->propPhoneNr ).value().toString();
	cl->ignored	= ignored_; //( property( "ignored" ).value().toString() == "true" );

	cl->uin		= QString::number( uin_ );
	cl->displayname	= metaContact()->displayName();

	cl->offlineTo	= false;
	cl->landline	= QString("");

	groupList = metaContact()->groups();

	Kopete::Group* gr;
	foreach ( gr, groupList ) {
// if present in any group, don't export to top level
// FIXME: again, probably bug in libkopete
// in case of topLevel group, Kopete::Group::displayName() returns "TopLevel" ineasted of just " " or "/"
// imo TopLevel group should be detected like i am doing that below
		if ( gr!=Kopete::Group::topLevel() ) {
			groups += gr->displayName()+',';
		}
	}

	if ( groups.length() ) {
		groups.truncate( groups.length()-1 );
	}
	cl->group = groups;

	return cl;
}

QString
GaduContact::findBestContactName( const GaduContactsList::ContactLine* cl )
{
	QString name;

	if ( cl == NULL ) {
		return name;
	}

	if ( cl->uin.isEmpty() ) {
		return name;
	}

	name = cl->uin;

	if ( cl->displayname.length() ) {
		name = cl->displayname;
	}
	else {
		// no name either
		if ( cl->nickname.isEmpty() ) {
			// maybe we can use fistname + surname ?
			if ( cl->firstname.isEmpty() && cl->surname.isEmpty() ) {
				name = cl->uin;
			}
			// what a shame, i have to use UIN than :/
			else {
				if ( cl->firstname.isEmpty() ) {
					name = cl->surname;
				}
				else {
					if ( cl->surname.isEmpty() ) {
						name = cl->firstname;
					}
					else {
						name = cl->firstname + ' ' + cl->surname;
					}
				}
			}
		}
		else {
			name = cl->nickname;
		}
	}

	return name;
}

void
GaduContact::messageAck()
{
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void
GaduContact::setIgnored( bool val )
{
	ignored_ = val;
}

bool
GaduContact::ignored()
{
	return ignored_;
}

#include "gaducontact.moc"
