// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
//
// gaduregisteraccount.cpp
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

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <klineedit.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpixmap.h>

#include "gaduregisteraccount.h"

GaduRegisterAccount::GaduRegisterAccount( QWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n( "Register New Account" ), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true )
{
	ui = new GaduRegisterAccountUI( this );
	setMainWidget( ui );

	ui->textToken->setDisabled( true );
	enableButton( Ok, false );

	cRegister = new RegisterCommand( this );

	connect( this, SIGNAL( applyClicked() ), SLOT( slotApply() ) );
	connect( cRegister, SIGNAL( tokenRecieved( QPixmap, QString ) ), SLOT( displayToken( QPixmap, QString ) ) );

	cRegister->requestToken();

	show();
}

void GaduRegisterAccount::displayToken( QPixmap image, QString tokenId )
{
	ui->textToken->setDisabled( false );
	ui->pixmapToken->setPixmap( image );
}

void GaduRegisterAccount::slotApply()
{
}

GaduRegisterAccount::~GaduRegisterAccount( )
{
}
unsigned int GaduRegisterAccount::registered_number()
{
// return 0 if failed
	return 0;
}
