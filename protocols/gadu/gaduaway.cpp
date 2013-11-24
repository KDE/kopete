// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
//
// gaduaway.cpp
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

#include "gaduaway.h"
#include "gaduaccount.h"
#include "gaduprotocol.h"
#include "ui_gaduawayui.h"

#include "kopeteonlinestatus.h"

#include <ktextedit.h>
#include <klocale.h>

#include <qradiobutton.h>
#include <qlineedit.h>

GaduAway::GaduAway( GaduAccount* account, QWidget* parent )
: KDialog( parent ), account_( account )
{
	setCaption(  i18n( "Away Dialog" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	showButtonSeparator( true );

	Kopete::OnlineStatus ks;
	int s;

	QWidget* w = new QWidget( this );
	ui_ = new Ui::GaduAwayUI;
	ui_->setupUi( w );
	setMainWidget( w );

	ui_->buttonGroup_->setId(ui_->onlineButton_, GG_STATUS_AVAIL_DESCR);
	ui_->buttonGroup_->setId(ui_->awayButton_, GG_STATUS_BUSY_DESCR);
	ui_->buttonGroup_->setId(ui_->invisibleButton_, GG_STATUS_INVISIBLE_DESCR);
	ui_->buttonGroup_->setId(ui_->offlineButton_, GG_STATUS_NOT_AVAIL_DESCR);
	
	
	ks = account->myself()->onlineStatus();
	s  = GaduProtocol::protocol()->statusToWithDescription( ks );


	if ( s == GG_STATUS_NOT_AVAIL_DESCR ) {
		ui_->buttonGroup_->button( GG_STATUS_NOT_AVAIL_DESCR )->setDisabled( true );
		ui_->buttonGroup_->button( GG_STATUS_AVAIL_DESCR)->setChecked(true);
	}
	else {
		ui_->buttonGroup_->button( s )->setChecked(true);
	}

	ui_->textEdit_->setText( account->myself()->property( "statusMessage" ).value().toString() );
	connect( this, SIGNAL(applyClicked()), SLOT(slotApply()) );
}

GaduAway::~GaduAway()
{
	delete ui_;
}

int
GaduAway::status() const
{
	return ui_->buttonGroup_->checkedId();
}

QString
GaduAway::awayText() const
{
	return ui_->textEdit_->text();
}


void
GaduAway::slotApply()
{
	if ( account_ ) {
		account_->changeStatus( GaduProtocol::protocol()->convertStatus( status() ),awayText() );
	}
}

#include "gaduaway.moc"
