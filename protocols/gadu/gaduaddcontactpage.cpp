// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 2003 Grzegorz Jaskiewicz 	<gj at pointblue.com.pl>
// Copyright (C) 2002-2003 Zack Rusin 	<zack@kde.org>
//
// gaduaddconectpage.cpp
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

#include "kopetemetacontact.h"

#include "gaduadd.h"
#include "gaduprotocol.h"
#include "gaduaccount.h"
#include "gaduaddcontactpage.h"
#include "gaducontact.h"
#include "gaducontactlist.h"

#include <klocale.h>
#include <kdebug.h>
#include <kopetecontactlist.h>
#include <kopetegroup.h>

#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qcombobox.h>
#include <krestrictedline.h>

GaduAddContactPage::GaduAddContactPage( GaduAccount* owner, QWidget* parent, const char* name )
: AddContactPage( parent, name )
{
	account_	= owner;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	addUI_	= new GaduAddUI( this );
	connect( addUI_->addEdit_, SIGNAL( textChanged( const QString & ) ), SLOT( slotUinChanged( const QString & ) ) );
	addUI_->addEdit_->setValidChars( "1234567890" );
	addUI_->addEdit_->setText( "" );
	addUI_->groups->setDisabled( TRUE );

	kdDebug(14100) << "filling gropus" << endl;

	fillGroups();
}

GaduAddContactPage::~GaduAddContactPage()
{
	delete addUI_;
}

void
GaduAddContactPage::fillGroups()
{
  /*
	Kopete::Group *g;
	QPtrList<Kopete::Group> gl = Kopete::ContactList::self()->groups();
	for( g = gl.first(); g; g = gl.next() ) {
		QCheckListItem* item = new QCheckListItem( addUI_->groups, g->displayName(), QCheckListItem::CheckBox );
		kdDebug(14100) << g->displayName() << " " << g->groupId() << endl;
	}
  */
}

void
GaduAddContactPage::showEvent( QShowEvent* e )
{
	slotUinChanged( QString::null );
	AddContactPage::showEvent( e );
}

void
GaduAddContactPage::slotUinChanged( const QString & )
{
	emit dataValid( this, validateData() );
}

bool
GaduAddContactPage::validateData()
{
	bool ok;
	long u;

	u = addUI_->addEdit_->text().toULong( &ok );
	if ( u == 0 ) {
		return false;
	}

	return ok;
}

bool
GaduAddContactPage::apply( Kopete::Account* a , Kopete::MetaContact* mc )
{
	if ( validateData() ) {
		QString userid	= addUI_->addEdit_->text().stripWhiteSpace();
		QString name	= addUI_->nickEdit_->text().stripWhiteSpace();
		if ( a != account_ ) {
			kdDebug(14100) << "Problem because accounts differ: " << a->accountId()
							<< " , " << account_->accountId() << endl;
		}
		if ( !a->addContact( userid,  mc, Kopete::Account::ChangeKABC )  ) {
			return false;
		}
		GaduContact *contact = static_cast<GaduContact*>( a->contacts()[ userid ] );

		contact->setProperty( GaduProtocol::protocol()->propEmail, addUI_->emailEdit_->text().stripWhiteSpace() );
		contact->setProperty( GaduProtocol::protocol()->propFirstName, addUI_->fornameEdit_->text().stripWhiteSpace() );
		contact->setProperty( GaduProtocol::protocol()->propLastName, addUI_->snameEdit_->text().stripWhiteSpace() );
		contact->setProperty( GaduProtocol::protocol()->propPhoneNr, addUI_->telephoneEdit_ ->text().stripWhiteSpace() );
		/*
		contact->setProperty( "ignored", i18n( "ignored" ), "false" );
		contact->setProperty( "nickName", i18n( "nick name" ), name );
		*/
	}
	return true;
}

#include "gaduaddcontactpage.moc"
