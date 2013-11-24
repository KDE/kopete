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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#include "gaduaddcontactpage.h"

#include "kopetemetacontact.h"

#include "ui_gaduadd.h"
#include "gaduprotocol.h"
#include "gaduaccount.h"
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
#include <q3listview.h>
#include <qcombobox.h>
#include <QShowEvent>
#include <QVBoxLayout>
#include <krestrictedline.h>

GaduAddContactPage::GaduAddContactPage( GaduAccount* owner, QWidget* parent )
: AddContactPage( parent )
{
	account_	= owner;

	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;
	addUI_	= new Ui::GaduAddUI;
	addUI_->setupUi( w );
	l->addWidget( w );

	connect( addUI_->addEdit_, SIGNAL(textChanged(QString)), SLOT(slotUinChanged(QString)) );
	addUI_->addEdit_->setValidChars( "1234567890" );
	addUI_->addEdit_->setText( "" );
	addUI_->groups->setDisabled( true );
	addUI_->addEdit_->setFocus();

	kDebug(14100) << "filling gropus";

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
		kDebug(14100) << g->displayName() << " " << g->groupId();
	}
  */
}

void
GaduAddContactPage::showEvent( QShowEvent* e )
{
	slotUinChanged( QString() );
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
		QString userid	= addUI_->addEdit_->text().trimmed();
		QString name	= addUI_->nickEdit_->text().trimmed();
		if ( a != account_ ) {
			kDebug(14100) << "Problem because accounts differ: " << a->accountId()
							<< " , " << account_->accountId() << endl;
		}
		if ( !a->addContact( userid,  mc, Kopete::Account::ChangeKABC )  ) {
			return false;
		}
		GaduContact *contact = static_cast<GaduContact*>( a->contacts().value( userid ) );

		contact->setProperty( GaduProtocol::protocol()->propEmail, addUI_->emailEdit_->text().trimmed() );
		contact->setProperty( GaduProtocol::protocol()->propFirstName, addUI_->fornameEdit_->text().trimmed() );
		contact->setProperty( GaduProtocol::protocol()->propLastName, addUI_->snameEdit_->text().trimmed() );
		contact->setProperty( GaduProtocol::protocol()->propPhoneNr, addUI_->telephoneEdit_ ->text().trimmed() );
		/*
		contact->setProperty( "ignored", i18n( "ignored" ), "false" );
		contact->setProperty( "nickName", i18n( "nickname" ), name );
		*/
	}
	return true;
}

#include "gaduaddcontactpage.moc"
