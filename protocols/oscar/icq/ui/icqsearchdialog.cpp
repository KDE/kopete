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
#include <qtextcodec.h>
#include <qtabwidget.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

#include "kopeteuiglobal.h"

#include "icqaccount.h"
#include "icqaddcontactpage.h"
#include "icqprotocol.h"
#include "icqsearchbase.h"
#include "oscartypes.h"
#include "icqcontact.h"
#include "icquserinfowidget.h"

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
	connect( m_searchUI->userInfoButton, SIGNAL( clicked() ), this, SLOT( userInfo() ) );
	connect( m_searchUI->newSearchButton, SIGNAL( clicked() ), this, SLOT( newSearch() ) );
	
	ICQProtocol *p = ICQProtocol::protocol();
	p->fillComboFromTable( m_searchUI->gender, p->genders() );
	p->fillComboFromTable( m_searchUI->country, p->countries() );
	p->fillComboFromTable( m_searchUI->language, p->languages() );
	
	m_contact = NULL;
	m_infoWidget = NULL;
	
	m_contact = NULL;
	m_infoWidget = NULL;
}


ICQSearchDialog::~ICQSearchDialog()
{
}

void ICQSearchDialog::startSearch()
{
	// Doing the search only if the account is online, otherwise warn the user
	if(!m_account->isConnected())
	{
		// Account currently offline
		m_searchUI->searchButton->setEnabled( false );
		KMessageBox::sorry( this, i18n("You must be online to search the ICQ Whitepages."), i18n("ICQ Plugin") );
	}
	else
	{
		// Account is online
		clearResults();
	
		m_searchUI->stopButton->setEnabled( true );
		m_searchUI->searchButton->setEnabled( false );
		m_searchUI->newSearchButton->setEnabled( false );
	
		connect( m_account->engine(), SIGNAL( gotSearchResults( const ICQSearchResult& ) ),
				this, SLOT( newResult( const ICQSearchResult& ) ) );
		connect( m_account->engine(), SIGNAL( endOfSearch( int ) ),
				this, SLOT( searchFinished( int ) ) );

		const QWidget* currentPage = m_searchUI->tabWidget3->currentPage();

		if ( currentPage == m_searchUI->tab )
		{
			if( m_searchUI->uin->text().isEmpty() || m_searchUI->uin->text().toULong() == 0 )
			{
				// Invalid UIN
				stopSearch();
				clearResults();
				KMessageBox::sorry( this, i18n("You must enter a valid UIN."), i18n("ICQ Plugin") );
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Search aborted: invalid UIN " << m_searchUI->uin->text() << endl;
			}
			else
			{
				//doing a uin search
				m_account->engine()->uinSearch( m_searchUI->uin->text() );
			}
		}
		else if ( currentPage == m_searchUI->tab_2 )
		{
			//create a ICQWPSearchInfo struct and send it
			ICQProtocol* p = ICQProtocol::protocol();
			ICQWPSearchInfo info;
			QTextCodec* codec = m_account->defaultCodec();
			info.firstName = codec->fromUnicode( m_searchUI->firstName->text() );
			info.lastName = codec->fromUnicode( m_searchUI->lastName->text() );
			info.nickName = codec->fromUnicode( m_searchUI->nickName->text() );
			info.email = codec->fromUnicode( m_searchUI->email->text() );
			info.city = codec->fromUnicode( m_searchUI->city->text() ); // City
			info.gender = p->getCodeForCombo(m_searchUI->gender, p->genders()); // Gender
			info.language = p->getCodeForCombo(m_searchUI->language, p->languages()); // Lang
			info.country =p->getCodeForCombo(m_searchUI->country, p->countries()); // country code
			info.onlineOnly = m_searchUI->onlyOnline->isChecked();
	
			// Check if the user has actually entered things to search
			if( info.firstName.isEmpty()			&&
				info.lastName.isEmpty()				&&
				info.nickName.isEmpty()				&&
				info.email.isEmpty()				&&
				info.city.isEmpty()					&&
				(info.gender == 0)					&&
				(info.language == 0)				&&
				(info.country == 0)
			)
			{
				// All fields were blank
				stopSearch();
				clearResults();
				KMessageBox::information(this, i18n("You must enter search criteria."), i18n("ICQ Plugin") );
				kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "Search aborted: all fields were blank" << endl;
			}
			else
			{
				// Start the search
				m_account->engine()->whitePagesSearch( info );
				kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Starting whitepage search" << endl;
			}
		}
	}
}

void ICQSearchDialog::stopSearch()
{
	disconnect( m_account->engine(), SIGNAL( gotSearchResults( const ICQSearchResult& ) ),
	         this, SLOT( newResult( const ICQSearchResult& ) ) );
	disconnect( m_account->engine(), SIGNAL( endOfSearch( int ) ),
	         this, SLOT( searchFinished( int ) ) );

	m_searchUI->stopButton->setEnabled( false );
	m_searchUI->searchButton->setEnabled( true );
	m_searchUI->newSearchButton->setEnabled( true );
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
		
		// Closing the dialog
		closeDialog();
	}
}

void ICQSearchDialog::userInfo()
{
	// Lookup user info only if the account is online, otherwise warn the user
	if(!m_account->isConnected())
	{
		// Account currently offline
		KMessageBox::sorry( this, i18n("You must be online to display user info."), i18n("ICQ Plugin") );
	}
	else
	{
		// Account currently online
		m_contact = new ICQContact( m_account,
									m_searchUI->searchResults->selectedItem()->text( 0 ),
									NULL);
	
		m_infoWidget = new ICQUserInfoWidget( Kopete::UI::Global::mainWidget(), "icq info" );
		QObject::connect( m_infoWidget, SIGNAL( finished() ), this, SLOT( closeUserInfo() ) );
	
		m_infoWidget->setContact( m_contact );
		m_infoWidget->setModal(true);
		m_infoWidget->show();
			if ( m_contact->account()->isConnected() )
			m_account->engine()->requestFullInfo( m_contact->contactId() );
		kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "Displaying user info" << endl;
	}
}

void ICQSearchDialog::closeUserInfo()
{
	// Free the ICQUserInfoWidget
	QObject::disconnect( this, 0, m_infoWidget, 0 );
	m_infoWidget->delayedDestruct();
	m_infoWidget = NULL;
	
	// Free the ICQContact
	delete m_contact;
	m_contact = NULL;
}

void ICQSearchDialog::clearResults()
{
	stopSearch();
	m_searchUI->searchResults->clear();
	m_searchUI->addButton->setEnabled( false );
	m_searchUI->userInfoButton->setEnabled( false );
	m_searchUI->searchButton->setEnabled( true );
}

void ICQSearchDialog::closeDialog()
{
	stopSearch();
	clearResults();
	clearFields();

	slotClose();
}

void ICQSearchDialog::resultSelectionChanged()
{
	if ( !m_searchUI->searchResults->selectedItem() )
	{
		m_searchUI->addButton->setEnabled( false );
		m_searchUI->userInfoButton->setEnabled( false );
	}
	else
	{
		m_searchUI->addButton->setEnabled( true );
		m_searchUI->userInfoButton->setEnabled( true );
	}
}

void ICQSearchDialog::newResult( const ICQSearchResult& info )
{
	if ( info.uin == 1 )
	{
		//TODO update progress
		return;
	}

	QTextCodec* codec = m_account->defaultCodec();

	QListViewItem *item = new QListViewItem( m_searchUI->searchResults, QString::number( info.uin ),
	                                         codec->toUnicode( info.nickName ),
	                                         codec->toUnicode( info.firstName ),
	                                         codec->toUnicode( info.lastName ),
	                                         codec->toUnicode( info.email ),
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
	m_searchUI->searchButton->setEnabled( true );
	m_searchUI->newSearchButton->setEnabled( true );
}

void ICQSearchDialog::clearFields()
{
	m_searchUI->uin->setText( QString::null );
	
	m_searchUI->firstName->setText( QString::null );
	m_searchUI->lastName->setText( QString::null );
	m_searchUI->nickName->setText( QString::null );
	m_searchUI->email->setText( QString::null );
	m_searchUI->city->setText( QString::null );
	m_searchUI->gender->setCurrentItem( 0 ); // Unspecified
	m_searchUI->country->setCurrentItem( 0 );
	m_searchUI->language->setCurrentItem( 0 );
	m_searchUI->onlyOnline->setChecked( false );
}

void ICQSearchDialog::newSearch()
{
	clearResults();
	clearFields();
}

//kate: indent-mode csands; space-indent off; replace-tabs off; tab-width 4;

#include "icqsearchdialog.moc"
