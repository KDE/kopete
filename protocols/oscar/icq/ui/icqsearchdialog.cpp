/*
 Kopete Oscar Protocol
 icqsearchdialog.cpp - search for people

 Copyright (c) 2005 Matt Rogers <mattr@kde.org>

 Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

 *************************************************************************
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU Lesser General Public            *
 * License as published by the Free Software Foundation; either          *
 * version 2 of the License, or (at your option) any later version.      *
 *                                                                       *
 *************************************************************************
*/

#include "icqsearchdialog.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kpushbutton.h>

#include "icqaccount.h"
#include "icqaddcontactpage.h"
#include "icqprotocol.h"
#include "icqsearchbase.h"
#include "oscartypes.h"

ICQSearchDialog::ICQSearchDialog( ICQAccount* account, QWidget* parent, const char* name )
: KDialogBase( parent, name, true, i18n( "ICQ User Search" ), 0, NoDefault )
{
	m_account = account;
	m_searchUI = new ICQSearchBase( this, name );
	setMainWidget( m_searchUI );
	connect( m_searchUI->searchButton, SIGNAL( clicked() ), this, SLOT( startSearch() ) );
	connect( m_searchUI->searchResults, SIGNAL( selectionChanged() ), this, SLOT( resultSelectionChanged() ) );
	connect( m_searchUI->addButton, SIGNAL( clicked() ), this, SLOT( addContact() ) );
	connect( m_searchUI->clearButton, SIGNAL( clicked() ), this, SLOT( clearResults() ) );
	connect( m_searchUI->stopButton, SIGNAL( clicked() ), this, SLOT( stopSearch() ) );
	connect( m_searchUI->closeButton, SIGNAL( clicked() ), this, SLOT( closeDialog() ) );
	
	ICQProtocol *p = ICQProtocol::protocol();
	p->fillComboFromTable( m_searchUI->gender, p->genders() );
	p->fillComboFromTable( m_searchUI->country, p->countries() );
	p->fillComboFromTable( m_searchUI->language, p->languages() );
	m_searchUI->gender->setCurrentItem( 2 ); //unspecified gender
}


ICQSearchDialog::~ICQSearchDialog()
{
}

void ICQSearchDialog::startSearch()
{
	m_searchUI->stopButton->setEnabled( true );
	connect( m_account->engine(), SIGNAL( gotSearchResults( const ICQSearchResult& ) ),
	         this, SLOT( newResult( const ICQSearchResult& ) ) );
	connect( m_account->engine(), SIGNAL( endOfSearch( int ) ),
	         this, SLOT( searchFinished( int ) ) );
	
	if ( !m_searchUI->uin->text().isEmpty() )
	{
		//doing a uin search
		m_account->engine()->uinSearch( m_searchUI->uin->text() );
	}
	else
	{
		//create a ICQWPSearchInfo struct and send it
		ICQProtocol* p = ICQProtocol::protocol();
		ICQWPSearchInfo info;
		info.firstName = m_searchUI->firstName->text();
		info.lastName = m_searchUI->lastName->text();
		info.nickName = m_searchUI->nickName->text();
		info.email = m_searchUI->email->text();
		info.city = m_searchUI->city->text(); // City
		switch ( m_searchUI->gender->currentItem() )
		{
		case 0: //female
			info.gender = 1;
			break;
		case 1: //male 
			info.gender = 2;
			break;
		case 2: //don't care
			info.gender = 0;
			break;
		}
		info.language = p->getCodeForCombo(m_searchUI->language, p->languages()); // Lang
		info.country =p->getCodeForCombo(m_searchUI->country, p->countries()); // country code
		info.onlineOnly = m_searchUI->onlyOnline->isChecked();
		m_account->engine()->whitePagesSearch( info );
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Starting whitepage search" << endl;
	}
}

void ICQSearchDialog::stopSearch()
{
	disconnect( m_account->engine(), SIGNAL( gotSearchResults( const ICQSearchResult& ) ),
	         this, SLOT( newResult( const ICQSearchResult& ) ) );
	disconnect( m_account->engine(), SIGNAL( endOfSearch( int ) ),
	         this, SLOT( searchFinished( int ) ) );
	m_searchUI->stopButton->setEnabled( false );
}

void ICQSearchDialog::addContact()
{
	ICQAddContactPage* iacp = dynamic_cast<ICQAddContactPage*>( parent() );
	if ( !iacp )
	{
		kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "The ICQ ACP is not our parent!!" << endl;
	}
	else
	{
		QString uin = m_searchUI->searchResults->selectedItem()->text( 0 );
		kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "Passing " << uin << " back to the ACP" << endl;
		iacp->setUINFromSearch( uin );
	}
}

void ICQSearchDialog::clearResults()
{
	stopSearch();
	m_searchUI->searchResults->clear();
	m_searchUI->addButton->setEnabled( false );
}

void ICQSearchDialog::closeDialog()
{
	stopSearch();
	clearResults();
	slotYes();
}

void ICQSearchDialog::resultSelectionChanged()
{
	if ( !m_searchUI->searchResults->selectedItem() )
		m_searchUI->addButton->setEnabled( false );
	else
		m_searchUI->addButton->setEnabled( true );
}

void ICQSearchDialog::newResult( const ICQSearchResult& info )
{
	if ( info.uin == '1' )
	{
		//TODO update progress
		return;
	}
		
	QListViewItem *item = new QListViewItem( m_searchUI->searchResults, QString::number( info.uin ),
	                                         info.nickName, info.firstName, info.lastName, info.email,
	                                         info.auth ? i18n( "Yes" ) : i18n( "No" ) );
	
	if ( !item )
		return;
	
	if ( info.online )
		item->setPixmap( 0, SmallIcon( "icq_online" ) );
	else
		item->setPixmap( 0, SmallIcon( "icq_offline" ) );

}

void ICQSearchDialog::searchFinished( int numLeft )
{
	kdWarning(OSCAR_ICQ_DEBUG) << k_funcinfo << "There are " << numLeft << "contact left out of this search" << endl;
	m_searchUI->stopButton->setEnabled( false );
	m_searchUI->clearButton->setEnabled( true );
}

//kate: indent-mode csands; space-indent off; replace-tabs off; tab-width 4;

#include "icqsearchdialog.moc"
