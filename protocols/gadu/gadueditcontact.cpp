// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
//
// gadueditcontact.cpp
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

#include "gaduaccount.h"
#include "gaducontact.h"
#include "gadueditcontact.h"
#include "kopeteonlinestatus.h"

#include "gaducontactlist.h"
#include "gaduadd.h"

#include <ktextedit.h>
#include <klocale.h>
#include <kdebug.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <krestrictedline.h>

GaduEditContact::GaduEditContact( GaduAccount* account, GaduContact* contact,
		    QWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n( "Edit Contact's Properties" ),
			 KDialogBase::Ok | KDialogBase::Cancel,
			 KDialogBase::Ok, true ), account_( account ), contact_( contact )
{

	if ( !account ) {
		return;
	}

	if ( contact ) {
		cl_ = contact->contactDetails();
	}
	else {
		return;
	}

	init();
}

GaduEditContact::GaduEditContact( GaduAccount* account,  GaduContactsList::ContactLine* clin,
		    QWidget* parent , const char* name  )
: KDialogBase( parent, name, true, i18n( "Edit Contact's Properties" ),
			 KDialogBase::Ok | KDialogBase::Cancel,
			 KDialogBase::Ok, true ), account_( account ), contact_( NULL )
{

	if ( !account ) {
		return;
	}
	cl_ = clin;
	init();
}

void
GaduEditContact::init()
{
	ui_ = new GaduAddUI( this );
	setMainWidget( ui_ );

	// fill values from cl into proper fields on widget
	fillIn();

	show();
	connect( this, SIGNAL( okClicked() ), SLOT( slotApply() ) );
}

void
GaduEditContact::fillIn()
{
// grey it out, it shouldn't be editable
	ui_->addEdit_->setReadOnly( true );
	ui_->addEdit_->setText( cl_->uin );

	ui_->fornameEdit_->setText( cl_->firstname );
	ui_->snameEdit_->setText( cl_->surname );
	ui_->nickEdit_->setText( cl_->nickname );
	ui_->emailEdit_->setText( cl_->email );
	ui_->telephoneEdit_->setText( cl_->phonenr );
//	ui_->notAFriend_;

}

void
GaduEditContact::slotApply()
{
	cl_->firstname = ui_->fornameEdit_->text().stripWhiteSpace();
	cl_->surname = ui_->snameEdit_->text().stripWhiteSpace();
	cl_->nickname = ui_->nickEdit_->text().stripWhiteSpace();
	cl_->email = ui_->emailEdit_->text().stripWhiteSpace();
	cl_->phonenr = ui_->telephoneEdit_->text().stripWhiteSpace();

	if ( contact_ == NULL ) {
		// contact doesn't exists yet, create it and set all the details
		bool s = account_->addMetaContact( cl_->uin, GaduContact::findBestContactName( cl_ ), 0L, Kopete::Account::DontChangeKABC);
		if ( s == false ) {
			kdDebug(14100) << "There was a problem adding UIN "<< cl_->uin << "to users list" << endl;
			return;
		}
		contact_ = static_cast<GaduContact*>( account_->contacts()[ cl_->uin ] );
		if ( contact_ == NULL ) {
 			kdDebug(14100) << "oops, no Kopete::Contact in contacts()[] for some reason, for \"" << cl_->uin << "\"" << endl;
			return;
		}
	}

	contact_->setContactDetails( cl_ );
}

#include "gadueditcontact.moc"
