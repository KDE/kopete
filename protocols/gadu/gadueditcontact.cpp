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

GaduEditContact::GaduEditContact( GaduAccount* account, GaduContact* contact,
		    QWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n( "Edit Contacts properties" ),
			 KDialogBase::Ok | KDialogBase::Cancel,
			 KDialogBase::Ok, true ), account_( account ), contact_( contact )
{

	if ( !contact || !account ) {
		return;
	}

	ui_ = new gaduAddUI( this );
	setMainWidget( ui_ );
	show();
	connect( this, SIGNAL( applyClicked() ), SLOT( slotApply() ) );
}

void
GaduEditContact::slotApply()
{
}

#include "gadueditcontact.moc"
