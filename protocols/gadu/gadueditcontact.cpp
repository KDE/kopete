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

#include <ktextedit.h>
#include <klocale.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <krestrictedline.h>

GaduEditContact::GaduEditContact( GaduAccount* account, GaduContact* contact,
		    QWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n( "Edit Contacts properties" ),
			 KDialogBase::Ok | KDialogBase::Cancel,
			 KDialogBase::Ok, true ), account_( account ), contact_( contact )
{

	if ( !contact || !account ) {
		return;
	}

	cl = contact->contactDetails();
	if ( !cl ) {
		return;
	}

	ui_ = new gaduAddUI( this );
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
	ui_->addEdit_->setDisabled( true );
	ui_->addEdit_->setText( QString::number( contact_->uin() ) );

	ui_->fornameEdit_->setText( cl->firstname );
	ui_->snameEdit_->setText( cl->surname );
	ui_->nickEdit_->setText( cl->nickname );
	ui_->emailEdit_->setText( cl->email );
	ui_->telephoneEdit_->setText( cl->phonenr );
//	ui_->notAFriend_;

}

void
GaduEditContact::slotApply()
{
	cl->firstname = ui_->fornameEdit_->text();
	cl->surname = ui_->snameEdit_->text();
	cl->nickname = ui_->nickEdit_->text();
	cl->email = ui_->emailEdit_->text();
	cl->phonenr = ui_->telephoneEdit_->text();

	contact_->setContactDetails( cl );
}

#include "gadueditcontact.moc"
