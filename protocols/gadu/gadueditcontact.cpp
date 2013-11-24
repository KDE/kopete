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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.

#include "gadueditcontact.h"
#include "gaduaccount.h"
#include "gaducontact.h"
#include "kopeteonlinestatus.h"

#include "gaducontactlist.h"
#include "ui_gaduadd.h"

#include <ktextedit.h>
#include <klocale.h>
#include <kdebug.h>
#include <kopetegroup.h>
#include <kopetecontactlist.h>
#include <kopetemetacontact.h>

#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <q3listview.h>

#include <krestrictedline.h>

// FIXME: this and gaduadcontactpage should have one base class, with some code duplicated in both.

GaduEditContact::GaduEditContact( GaduAccount* account, GaduContact* contact,
		    QWidget* parent )
: KDialog( parent ), account_( account ), contact_( contact )
{
	setCaption( i18n( "Edit Contact's Properties" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	showButtonSeparator( true );

	if ( contact && account ) {
		cl_ = contact->contactDetails();
	}
	else {
		return;
	}

	init();
	fillGroups();
	fillIn();
}

GaduEditContact::GaduEditContact( GaduAccount* account,  GaduContactsList::ContactLine* clin,
		    QWidget* parent )
: KDialog( parent ), account_( account ), contact_( NULL )
{
	setCaption( i18n( "Edit Contact's Properties" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton( KDialog::Ok );
	showButtonSeparator( true );

	if ( !account ) {
		return;
	}
	cl_ = clin;
	init();
	fillGroups();
	fillIn();
}

GaduEditContact::~GaduEditContact()
{
	delete ui_;
}

void
GaduEditContact::fillGroups()
{
	Kopete::Group *g, *cg;
	QList<Kopete::Group*> cgl;
	QList<Kopete::Group*> gl;

	if ( contact_ ) {
		cgl = contact_->metaContact()->groups();
	}

	gl = Kopete::ContactList::self()->groups();

	foreach (g, gl)  {
		if ( g->type() == Kopete::Group::Temporary ) {
			continue;
		}
		Q3CheckListItem* item = new Q3CheckListItem( ui_->groups, g->displayName(), Q3CheckListItem::CheckBox );
		// FIXME: optimize this O(2) search
		foreach( cg , cgl ) {
			if ( cg->groupId() == g->groupId() ) {
				item->setOn( true );
				break;
			}
		}
		kDebug(14100) << g->displayName() << " " << g->groupId();
	}
}

void
GaduEditContact::init()
{
	QWidget* w = new QWidget( this );
	ui_ = new Ui::GaduAddUI;
	ui_->setupUi( w );
	setMainWidget( w );
	ui_->addEdit_->setValidChars( "1234567890" );

	// fill values from cl into proper fields on widget

	show();
	connect( this, SIGNAL(okClicked()), SLOT(slotApply()) );
	connect( ui_->groups, SIGNAL(clicked(Q3ListViewItem*)), SLOT(listClicked(Q3ListViewItem*)) );
}

void
GaduEditContact::listClicked( Q3ListViewItem* /*item*/ )
{

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
	QList<Kopete::Group*> gl;
	Kopete::Group* group;

	cl_->firstname = ui_->fornameEdit_->text().trimmed();
	cl_->surname = ui_->snameEdit_->text().trimmed();
	cl_->nickname = ui_->nickEdit_->text().trimmed();
	cl_->email = ui_->emailEdit_->text().trimmed();
	cl_->phonenr = ui_->telephoneEdit_->text().trimmed();

	if ( contact_ == NULL ) {
		// contact doesn't exists yet, create it and set all the details
		bool s = account_->addContact( cl_->uin, GaduContact::findBestContactName( cl_ ), 0L, Kopete::Account::DontChangeKABC);
		if ( s == false ) {
			kDebug(14100) << "There was a problem adding UIN "<< cl_->uin << "to users list";
			return;
		}
		contact_ = static_cast<GaduContact*>( account_->contacts().value( cl_->uin ) );
		if ( contact_ == NULL ) {
			kDebug(14100) << "oops, no Kopete::Contact in contacts()[] for some reason, for \"" << cl_->uin << "\"";
			return;
		}
	}

	contact_->setContactDetails( cl_ );

	gl = Kopete::ContactList::self()->groups();
	for ( Q3ListViewItemIterator it( ui_->groups ); it.current(); ++it ) {
		Q3CheckListItem *check = dynamic_cast<Q3CheckListItem *>( it.current() );
		
		if ( !check ) {
			continue;
		}

		if ( check->isOn() ) {
			foreach( group, gl )  {
				if ( group->displayName() == check->text() ) {
					contact_->metaContact()->addToGroup( group );
				}
			}
		}
		else {
			// check metacontact's in the group, and if so, remove it from
			foreach( group, gl ) {
				if ( group->displayName() == check->text() ) {
					contact_->metaContact()->removeFromGroup( group );
				}
			}
		}
	}
	
	if( contact_->metaContact()->groups().isEmpty() == true )
		contact_->metaContact()->addToGroup( Kopete::Group::topLevel() );
}
#include "gadueditcontact.moc"
