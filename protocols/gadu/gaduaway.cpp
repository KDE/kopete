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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include "gaduaccount.h"
#include "gaduprotocol.h"
#include "gaduawayui.h"
#include "gaduaway.h"

#include "kopeteonlinestatus.h"

#include <ktextedit.h>
#include <klocale.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>

#include "gaduawayui.h"
#include "gaduaway.h"

GaduAway::GaduAway( GaduAccount* account, QWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n( "Away Dialog" ),
			 KDialogBase::Ok | KDialogBase::Cancel,
			 KDialogBase::Ok, true ), account_( account )
{
	Kopete::OnlineStatus ks;
	int s;

	ui_ = new GaduAwayUI( this );
	setMainWidget( ui_ );

	ks = account->myself()->onlineStatus();
	s  = GaduProtocol::protocol()->statusToWithDescription( ks );

	if ( s == GG_STATUS_NOT_AVAIL_DESCR ) {
		ui_->statusGroup_->find( GG_STATUS_NOT_AVAIL_DESCR )->setDisabled( TRUE );
		ui_->statusGroup_->setButton( GG_STATUS_AVAIL_DESCR );
	}
	else {
		ui_->statusGroup_->setButton( s );
	}

	ui_->textEdit_->setText( account->myself()->property( "awayMessage" ).value().toString() );
	connect( this, SIGNAL( applyClicked() ), SLOT( slotApply() ) );
}

int
GaduAway::status() const
{
	return ui_->statusGroup_->id( ui_->statusGroup_->selected() );
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
