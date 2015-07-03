//
// Copyright (C) 2003	 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
//
// gadupubdir.cpp
//  Gadu-Gadu Public directory contains people data, using it you can search friends
// different criteria
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
//

#include "gadupubdir.h"
#include "gadueditcontact.h"
#include "gaducontactlist.h"
#include "gaduaccount.h"
#include "gaduprotocol.h"

#include <q3widgetstack.h>
#include <q3listview.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <QPixmap>

#include <kcombobox.h>
#include <krestrictedline.h>
#include <klineedit.h>
#include <klocale.h>

GaduPublicDir::GaduPublicDir( GaduAccount* account, QWidget* parent )
: KDialog( parent )
{
	setButtons( KDialog::User1 | KDialog::User2 | KDialog::User3 | KDialog::Cancel );
	setDefaultButton( KDialog::User2 );

	mAccount = account;
	createWidget();
	initConnections();

	show();
}

GaduPublicDir::GaduPublicDir( GaduAccount* account, int searchFor, QWidget* parent )
: KDialog( parent )
{
	setButtons( KDialog::User1 | KDialog::User2 | KDialog::User3 | KDialog::Cancel );
	setDefaultButton( KDialog::User2 );

	ResLine rs;

	mAccount = account;
	createWidget();
	initConnections();

	kDebug( 14100 ) << "search for Uin: " << searchFor;

	mMainWidget->listFound->clear();
	show();

	if ( searchFor == 0 ) {
		return;
	}

	mMainWidget->pubsearch->raiseWidget( 1 );
	mMainWidget->radioByUin->setChecked( true );

	setButtonGuiItem( KDialog::User2, KGuiItem( i18n( "Search &More..." ) ) );
	showButton( KDialog::User3, true );
	showButton( KDialog::User1, true );
	enableButton( KDialog::User3, false );
	enableButton( KDialog::User2, false );

	// now it is time to switch to Right Page(tm)
	rs.uin = searchFor;
	
	fName.clear();
  fSurname.clear();
  fNick.clear();
  fCity.clear();
	fUin	= searchFor;
	fGender	= fAgeFrom = fAgeTo = 0;
	fOnlyOnline = false;

	mAccount->pubDirSearch( rs, fAgeFrom, fAgeTo, fOnlyOnline );

}

GaduPublicDir::~GaduPublicDir()
{
	delete mMainWidget;
}

void
GaduPublicDir::createWidget()
{
	setCaption( i18n( "Gadu-Gadu Public Directory" ) );

	QWidget* w = new QWidget( this );
	mMainWidget = new Ui::GaduPublicDirectory;
	mMainWidget->setupUi( this );
	setMainWidget( w );

	mMainWidget->UIN->setValidChars( "1234567890" );

	setButtonGuiItem( KDialog::User1, KGuiItem( i18n( "&New Search" ) ) );
	setButtonGuiItem( KDialog::User2, KGuiItem( i18n( "S&earch" ) ) );
	setButtonGuiItem( KDialog::User3, KGuiItem( i18n( "&Add User..." ) ) );
	setButtonGuiItem( KDialog::Cancel, KGuiItem( i18n( "&Close" ) ) );

	showButton( KDialog::User1, false );
	showButton( KDialog::User3, false );
	enableButton( KDialog::User2, false );

	mMainWidget->radioByData->setChecked( true );

	mAccount->pubDirSearchClose();

}

void
GaduPublicDir::slotAddContact()
{
	GaduContactsList::ContactLine* cl = new GaduContactsList::ContactLine;
	Q3ListViewItem* item = mMainWidget->listFound->currentItem();

	cl->ignored	= false;
	cl->firstname	= item->text( 1 );
	cl->uin		= item->text( 5 );
	cl->nickname 	= item->text( 2 );

	cl->surname	= fSurname;

//	GaduEditContact *ed = 
	new GaduEditContact( mAccount, cl, this );
}

void
GaduPublicDir::slotListSelected(  )
{
	Q3ListViewItem* item = mMainWidget->listFound->currentItem();
	if ( item ) {
		enableButton( KDialog::User3, true );
	}
	else {
		enableButton( KDialog::User3, false );
	}
}

void
GaduPublicDir::initConnections()
{
	connect( this, SIGNAL(user2Clicked()), SLOT(slotSearch()) );
	connect( this, SIGNAL(user1Clicked()), SLOT(slotNewSearch()) );
	connect( this, SIGNAL(user3Clicked()), SLOT(slotAddContact()) );

	connect( mAccount, SIGNAL(pubDirSearchResult(SearchResult,uint)),
				SLOT(slotSearchResult(SearchResult,uint)) );

	connect( mMainWidget->nameS,		SIGNAL(textChanged(QString)), SLOT(inputChanged(QString)) );
	connect( mMainWidget->surname,		SIGNAL(textChanged(QString)), SLOT(inputChanged(QString)) );
	connect( mMainWidget->nick,		SIGNAL(textChanged(QString)), SLOT(inputChanged(QString)) );
	connect( mMainWidget->UIN,		SIGNAL(textChanged(QString)), SLOT(inputChanged(QString)) );
	connect( mMainWidget->cityS,		SIGNAL(textChanged(QString)), SLOT(inputChanged(QString)) );
	connect( mMainWidget->gender,		SIGNAL(activated(QString)), SLOT(inputChanged(QString)) );
	connect( mMainWidget->ageFrom,		SIGNAL(valueChanged(QString)), SLOT(inputChanged(QString)) );
	connect( mMainWidget->ageTo,		SIGNAL(valueChanged(QString)), SLOT(inputChanged(QString)) );
	connect( mMainWidget->radioByData,	SIGNAL(toggled(bool)), SLOT(inputChanged(bool)) );

	connect( mMainWidget->listFound,	SIGNAL(selectionChanged()), SLOT(slotListSelected()) );

}

void
GaduPublicDir::inputChanged( bool )
{
	inputChanged( QString() );
}

void
GaduPublicDir::inputChanged( const QString& )
{
	if ( validateData() == false ) {
		enableButton( User2, false );
	}
	else {
		enableButton( User2, true );
	}
}

void
GaduPublicDir::getData()
{
	fName		= mMainWidget->nameS->text();
	fSurname	= mMainWidget->surname->text();
	fNick		= mMainWidget->nick->text();
	fUin		= mMainWidget->UIN->text().toInt();
	fGender		= mMainWidget->gender->currentIndex();
	fOnlyOnline	= mMainWidget->onlyOnline->isChecked();
	fAgeFrom	= mMainWidget->ageFrom->value();
	fAgeTo		= mMainWidget->ageTo->value();
	fCity		= mMainWidget->cityS->text();
}

// return true if not empty
#define CHECK_STRING(A) { if ( !A.isEmpty() ) { return true; } }
#define CHECK_INT(A) { if ( A ) { return true; } }

bool
GaduPublicDir::validateData()
{
	getData();

	if ( mMainWidget->radioByData->isChecked() ) {
		CHECK_STRING( fCity );
		CHECK_STRING( fName );
		CHECK_STRING( fSurname );
		CHECK_STRING( fNick );
		CHECK_INT( fGender );
		CHECK_INT( fAgeFrom );
		CHECK_INT( fAgeTo );
	}
	else {
		fSurname.clear();
		CHECK_INT( fUin );
	}
	return false;
}

// Move to GaduProtocol someday
QPixmap
GaduPublicDir::iconForStatus( uint status )
{
	QPixmap n;

	if ( GaduProtocol::protocol() ) {
		return GaduProtocol::protocol()->convertStatus( status ).protocolIcon(KIconLoader::SizeSmall);
	}
	return n;
}

void
GaduPublicDir::slotSearchResult( const SearchResult& result, unsigned int )
{
	Q3ListView* list = mMainWidget->listFound;

	kDebug(14100) << "searchResults(" << result.count() <<")";

	Q3ListViewItem* sl;

	SearchResult::const_iterator r;

	for ( r = result.begin(); r != result.end() ; ++r ){
		kDebug(14100) << "adding" << (*r).uin;
		sl= new Q3ListViewItem(
					list, QString::fromAscii(""),
					(*r).firstname,
					(*r).nickname,
					(*r).age,
					(*r).city,
					QString::number( (*r).uin ).toAscii()
						);
		sl->setPixmap( 0, iconForStatus( (*r).status ) );
	}

	// if not found anything, obviously we don't want to search for more
	// if we are looking just for one UIN, don't allow search more - it is pointless

	if ( result.count() && fUin==0 ) {
		enableButton( KDialog:: User2, true );
	}

	enableButton( KDialog::User1, true );
	enableButton( KDialog::User3, false );
	mMainWidget->pubsearch->setDisabled( false );

}

void
GaduPublicDir::slotNewSearch()
{
	mMainWidget->pubsearch->raiseWidget( 0 );

	setButtonGuiItem( KDialog::User2, KGuiItem( i18n( "S&earch" ) ) );

	showButton( KDialog::User1, false );
	showButton( KDialog::User3, false );
	enableButton( KDialog::User2, false );
 	inputChanged( QString() );
 	mAccount->pubDirSearchClose();
}

void
GaduPublicDir::slotSearch()
{

	mMainWidget->listFound->clear();
	QString empty;

	// search more, or search ?
	if ( mMainWidget->pubsearch->id( mMainWidget->pubsearch->visibleWidget() ) == 0 ) {
		kDebug(14100) << "start search... ";
		getData();

		// validate data
		if ( validateData() == false ) {
			return;
		}

		// go on
		mMainWidget->pubsearch->raiseWidget( 1 );

	}
	else{
		kDebug(14100) << "search more... ";
		// Search for more
	}
	mMainWidget->pubsearch->setDisabled( true );
	setButtonGuiItem( KDialog::User2, KGuiItem( i18n( "Search &More..." ) ) );
	showButton( KDialog::User3, true );
	showButton( KDialog::User1, true );
	enableButton( KDialog::User3, false );
	enableButton( KDialog::User2, false );

	ResLine rs;
	rs.firstname	= fName;
	rs.surname	= fSurname;
	rs.nickname	= fNick;
	rs.uin		= fUin;
	rs.city		= fCity;
	
	if ( fGender == 1 ) {
		rs.gender =  GG_PUBDIR50_GENDER_MALE;
	}
	if ( fGender == 2 ) {
		rs.gender = GG_PUBDIR50_GENDER_FEMALE;
	}
	
	
	if ( mMainWidget->radioByData->isChecked() ) {
		mAccount->pubDirSearch( rs, fAgeFrom, fAgeTo, fOnlyOnline );
	}
	else {
		mAccount->pubDirSearch( rs, 0, 0, fOnlyOnline );
	}
}

#include "gadupubdir.moc"
