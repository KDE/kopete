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

#include <QStandardItemModel>

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtextcodec.h>
#include <QTabWidget>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

#include "kopeteuiglobal.h"

#include "icqaccount.h"
#include "icqaddcontactpage.h"
#include "icqprotocol.h"
#include "ui_icqsearchbase.h"
#include "oscartypes.h"
#include "icquserinfowidget.h"

ICQSearchDialog::ICQSearchDialog( ICQAccount* account, QWidget* parent )
: KDialog( parent )
{
	setCaption( i18n( "ICQ User Search" ) );
	setButtons( KDialog::Ok | KDialog::Close );

	setButtonText( KDialog::Ok, i18n( "Add" ) );
	setButtonWhatsThis( KDialog::Ok, i18n( "Add the selected user to your contact list" ) );
	enableButton( KDialog::Ok, false );

	m_account = account;
	QWidget* w = new QWidget( this );
	m_searchUI = new Ui::ICQSearchBase();
	m_searchUI->setupUi( w );
	setMainWidget( w );
	
	m_searchResultsModel = new QStandardItemModel( 0, 6 );
	m_searchResultsModel->setHeaderData( 0, Qt::Horizontal, i18n( "UIN" ) );
	m_searchResultsModel->setHeaderData( 1, Qt::Horizontal, i18n( "Nickname" ) );
	m_searchResultsModel->setHeaderData( 2, Qt::Horizontal, i18n( "First Name" ) );
	m_searchResultsModel->setHeaderData( 3, Qt::Horizontal, i18n( "Last Name" ) );
	m_searchResultsModel->setHeaderData( 4, Qt::Horizontal, i18n( "Email" ) );
	m_searchResultsModel->setHeaderData( 5, Qt::Horizontal, i18n( "Requires Authorization" ) );
	m_searchUI->searchResults->setModel( m_searchResultsModel );
	m_searchUI->searchResults->setEditTriggers( QAbstractItemView::NoEditTriggers );
	
	connect( m_searchUI->searchButton, SIGNAL(clicked()), this, SLOT(startSearch()) );
	connect( m_searchUI->searchResults->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
	         this, SLOT(selectionChanged(QItemSelection)) );
	connect( m_searchUI->clearButton, SIGNAL(clicked()), this, SLOT(clear()) );
	connect( m_searchUI->stopButton, SIGNAL(clicked()), this, SLOT(stopSearch()) );
	connect( m_searchUI->userInfoButton, SIGNAL(clicked()), this, SLOT(userInfo()) );
	
	ICQProtocol *p = ICQProtocol::protocol();
	p->fillComboFromTable( m_searchUI->gender, p->genders() );
	p->fillComboFromTable( m_searchUI->country, p->countries() );
	p->fillComboFromTable( m_searchUI->language, p->languages() );
	
	m_infoWidget = NULL;
}


ICQSearchDialog::~ICQSearchDialog()
{
	delete m_searchUI;
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
		m_searchUI->clearButton->setEnabled( false );
	
		connect( m_account->engine(), SIGNAL(gotSearchResults(ICQSearchResult)),
				this, SLOT(newResult(ICQSearchResult)) );
		connect( m_account->engine(), SIGNAL(endOfSearch(int)),
				this, SLOT(searchFinished(int)) );

		const QWidget* currentPage = m_searchUI->tabWidget3->currentWidget();

		if ( currentPage == m_searchUI->tabUIN )
		{
			if( m_searchUI->uin->text().isEmpty() || m_searchUI->uin->text().toULong() == 0 )
			{
				// Invalid UIN
				stopSearch();
				clearResults();
				KMessageBox::sorry( this, i18n("You must enter a valid UIN."), i18n("ICQ Plugin") );
				kDebug(OSCAR_RAW_DEBUG) << "Search aborted: invalid UIN " << m_searchUI->uin->text();
			}
			else
			{
				//doing a uin search
				m_account->engine()->uinSearch( m_searchUI->uin->text() );
			}
		}
		else if ( currentPage == m_searchUI->tabWhitepages )
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
				kDebug(OSCAR_ICQ_DEBUG) << "Search aborted: all fields were blank";
			}
			else
			{
				// Start the search
				m_account->engine()->whitePagesSearch( info );
				kDebug(OSCAR_RAW_DEBUG) << "Starting whitepage search";
			}
		}
	}
}

void ICQSearchDialog::stopSearch()
{
	disconnect( m_account->engine(), SIGNAL(gotSearchResults(ICQSearchResult)),
	         this, SLOT(newResult(ICQSearchResult)) );
	disconnect( m_account->engine(), SIGNAL(endOfSearch(int)),
	         this, SLOT(searchFinished(int)) );

	m_searchUI->stopButton->setEnabled( false );
	m_searchUI->searchButton->setEnabled( true );
	m_searchUI->clearButton->setEnabled( true );
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
		QModelIndexList indexList = m_searchUI->searchResults->selectionModel()->selectedIndexes();
		if ( indexList.count() > 0 )
		{
			const QAbstractItemModel *model = m_searchUI->searchResults->selectionModel()->model();
			QModelIndex index = model->index( indexList.at( 0 ).row(), 0, QModelIndex() );
			QString uin = model->data( index ).toString();

			m_infoWidget = new ICQUserInfoWidget( m_account, uin, Kopete::UI::Global::mainWidget() );
			QObject::connect( m_infoWidget, SIGNAL(finished()), this, SLOT(closeUserInfo()) );

			m_infoWidget->setModal(true);
			m_infoWidget->show();
			kDebug(OSCAR_ICQ_DEBUG) << "Displaying user info";
		}
	}
}

void ICQSearchDialog::closeUserInfo()
{
	// Free the ICQUserInfoWidget
	QObject::disconnect( this, 0, m_infoWidget, 0 );
	m_infoWidget->delayedDestruct();
	m_infoWidget = NULL;
}

void ICQSearchDialog::clearResults()
{
	stopSearch();
	m_searchResultsModel->removeRows( 0, m_searchResultsModel->rowCount() );
	enableButton( KDialog::Ok, false );
	m_searchUI->userInfoButton->setEnabled( false );
	m_searchUI->searchButton->setEnabled( true );
}

void ICQSearchDialog::selectionChanged( const QItemSelection & selected )
{
	if ( selected.isEmpty() )
	{
		enableButton( KDialog::Ok, false );
		m_searchUI->userInfoButton->setEnabled( false );
	}
	else
	{
		enableButton( KDialog::Ok, true );
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

	const int row = m_searchResultsModel->rowCount( QModelIndex() );
	m_searchResultsModel->insertRows( row, 1, QModelIndex());
	
	QModelIndex index;
	
	index = m_searchResultsModel->index( row, 0, QModelIndex());
	m_searchResultsModel->setData( index, QString::number( info.uin ) );
	
	if ( info.online )
		m_searchResultsModel->setData( index, SmallIcon( "icq_online" ), Qt::DecorationRole );
	else
		m_searchResultsModel->setData( index, SmallIcon( "icq_offline" ), Qt::DecorationRole );
	
	index = m_searchResultsModel->index( row, 1, QModelIndex());
	m_searchResultsModel->setData( index, codec->toUnicode( info.nickName ) );
	
	index = m_searchResultsModel->index( row, 2, QModelIndex());
	m_searchResultsModel->setData( index, codec->toUnicode( info.firstName ) );
	
	index = m_searchResultsModel->index( row, 3, QModelIndex());
	m_searchResultsModel->setData( index, codec->toUnicode( info.lastName ) );
	
	index = m_searchResultsModel->index( row, 4, QModelIndex());
	m_searchResultsModel->setData( index, codec->toUnicode( info.email ) );
	
	index = m_searchResultsModel->index( row, 5, QModelIndex());
	m_searchResultsModel->setData( index, info.auth ? i18n( "Yes" ) : i18n( "No" ) );
}

void ICQSearchDialog::searchFinished( int numLeft )
{
	kWarning(OSCAR_ICQ_DEBUG) << "There are " << numLeft << "contact left out of this search";
	m_searchUI->stopButton->setEnabled( false );
	m_searchUI->clearButton->setEnabled( true );
	m_searchUI->searchButton->setEnabled( true );
}

void ICQSearchDialog::clearFields()
{
	m_searchUI->uin->setText( QString() );
	
	m_searchUI->firstName->setText( QString() );
	m_searchUI->lastName->setText( QString() );
	m_searchUI->nickName->setText( QString() );
	m_searchUI->email->setText( QString() );
	m_searchUI->city->setText( QString() );
	m_searchUI->gender->setCurrentIndex( 0 ); // Unspecified
	m_searchUI->country->setCurrentIndex( 0 );
	m_searchUI->language->setCurrentIndex( 0 );
	m_searchUI->onlyOnline->setChecked( false );
}

void ICQSearchDialog::clear()
{
	clearResults();
	clearFields();
}

void ICQSearchDialog::slotButtonClicked( int button )
{
	if ( button == KDialog::Ok )
	{
		ICQAddContactPage* iacp = dynamic_cast<ICQAddContactPage*>( parent() );
		if ( !iacp )
		{
			kDebug(OSCAR_ICQ_DEBUG) << "The ICQ ACP is not our parent!!";
		}
		else
		{
			QModelIndexList indexList = m_searchUI->searchResults->selectionModel()->selectedIndexes();
			if ( indexList.count() > 0 )
			{
				const QAbstractItemModel *model = m_searchUI->searchResults->selectionModel()->model();
				QModelIndex index = model->index( indexList.at( 0 ).row(), 0, QModelIndex() );
				QString uin = model->data( index ).toString();

				kDebug(OSCAR_ICQ_DEBUG) << "Passing " << uin << " back to the ACP";
				iacp->setUINFromSearch( uin );
			}
		}
		accept();
	}
	else if ( button == KDialog::Close )
	{
		stopSearch();
		close();
	}
	else
	{
		KDialog::slotButtonClicked( button );
	}
}

//kate: indent-mode csands; space-indent off; replace-tabs off; tab-width 4;

#include "icqsearchdialog.moc"
