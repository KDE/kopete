/*
    gwaddcontactpage.cpp - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwaddcontactpage.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qvaluelist.h>

#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopetemetacontact.h"

#include "client.h"
#include "gwaccount.h"
#include "gwerror.h"
#include "gwprotocol.h"
#include "gwsearchwidget.h"
#include "tasks/searchtask.h"
#include "gwaddui.h"

class GWSearchResultsLVI : public QListViewItem
{
public:
	GWSearchResultsLVI( QListView * parent, int status, const QPixmap & statusPM, const QString & givenName,
		const QString & surname, const QString & userId, const QString & dn )
	: QListViewItem( parent, QString::null, givenName, surname, userId ), m_status( status ), m_dn( dn )
	{
		setPixmap( 0, statusPM );
	}
	QString key( int column, bool ascending ) const
	{
		if ( column == 0 )
			return QString::number( 99 - m_status );
		else
			return QListViewItem::key( column, ascending );
	}
	int m_status;
	QString m_dn;
};

GroupWiseAddContactPage::GroupWiseAddContactPage( KopeteAccount * owner, QWidget* parent, const char* name )
		: AddContactPage(parent, name)
{
	m_account = static_cast<GroupWiseAccount *>( owner );
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << endl;
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	if (owner->isConnected ())
	{
		m_gwAddUI = new GroupWiseAddUI( this );
		connect( m_gwAddUI->rb_userId, SIGNAL( toggled( bool ) ), SLOT( slotAddMethodChanged() ) );
		connect( m_gwAddUI->rb_userName, SIGNAL( toggled( bool ) ), SLOT( slotAddMethodChanged() ) );
		
		// add search widget
		( new QVBoxLayout( m_gwAddUI->m_tabWidget->page( 1 ) ) )->setAutoAdd( true );
		m_searchUI = new GWSearchWidget( m_gwAddUI->m_tabWidget->page( 1 ) );
		m_searchUI->m_results->setAllColumnsShowFocus( true );
		connect( m_searchUI->m_search, SIGNAL( clicked() ), SLOT( slotDoSearch() ) );
		connect( m_searchUI->m_details, SIGNAL( clicked() ), SLOT( slotShowDetails() ) );
		m_gwAddUI->show ();

		m_canadd = true;
	}
	else
	{
		m_noaddMsg1 = new QLabel (i18n ("You need to be connected to be able to add contacts."), this);
		m_noaddMsg2 = new QLabel (i18n ("Connect to the GroupWise server and try again."), this);
		m_canadd = false;
	}
}

GroupWiseAddContactPage::~GroupWiseAddContactPage()
{
// , i18n( "The search was cancelled" )
// , i18n( "There was an error while carrying out your search.  Please change your search terms or try again later." )
// i18n( "There was an error while carrying out your search.  Please change your search terms or try again later." )
}

void GroupWiseAddContactPage::slotAddMethodChanged()
{
	if ( m_gwAddUI->rb_userId->isChecked() )
		m_gwAddUI->m_userId->setFocus();
	else
		m_gwAddUI->m_userName->setFocus();
}

bool GroupWiseAddContactPage::apply( KopeteAccount* account, KopeteMetaContact* parentContact )
{
	if ( m_canadd && validateData() )
	{
		QString contactId;
		QString displayName;
		// take the entered userId if that tab is at the front, otherwise the selected search results.
		displayName = parentContact->displayName();
		if ( m_gwAddUI->m_tabWidget->currentPageIndex() == 0 )
		{	contactId = m_gwAddUI->m_userId->text();
			if ( displayName.isEmpty() )
				displayName = contactId;
		}
		else
		{
			GWSearchResultsLVI * selection = static_cast< GWSearchResultsLVI * >( m_searchUI->m_results->selectedItem() );
			contactId = selection->m_dn;
			if ( displayName.isEmpty() )
				displayName = selection->text( 1 ) + " " + selection->text( 3 );
		}

		return ( account->addContact ( contactId, displayName, parentContact, KopeteAccount::ChangeKABC ) );
	}
	else
		return false;
}

bool GroupWiseAddContactPage::validateData()
{
	if ( m_gwAddUI->m_tabWidget->currentPageIndex() == 0 )
		return ( !m_gwAddUI->m_userId->text().isEmpty() );
	else
		return ( m_searchUI->m_results->selectedItem() );
}

void GroupWiseAddContactPage::slotDoSearch()
{
	// build a query
	QValueList< GroupWise::UserSearchQueryTerm > searchTerms;
	if ( !m_searchUI->m_firstName->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_searchUI->m_firstName->text();
		arg.field = "Given Name";
		arg.operation = searchOperation( m_searchUI->m_firstNameOperation->currentItem() );
		searchTerms.append( arg );
	}
	if ( !m_searchUI->m_lastName->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_searchUI->m_lastName->text();
		arg.field = "Surname";
		arg.operation = searchOperation( m_searchUI->m_lastNameOperation->currentItem() );
		searchTerms.append( arg );
	}
	if ( !m_searchUI->m_userId->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_searchUI->m_userId->text();
		arg.field = "UserID";
		arg.operation = searchOperation( m_searchUI->m_userIdOperation->currentItem() );
		searchTerms.append( arg );
	}
	if ( !m_searchUI->m_title->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_searchUI->m_title->text();
		arg.field = "Title";
		arg.operation = searchOperation( m_searchUI->m_titleOperation->currentItem() );
		searchTerms.append( arg );
	}
	if ( !m_searchUI->m_dept->text().isEmpty() )
	{
		GroupWise::UserSearchQueryTerm arg;
		arg.argument = m_searchUI->m_dept->text();
		arg.field = "Department";
		arg.operation = searchOperation( m_searchUI->m_deptOperation->currentItem() );
		searchTerms.append( arg );
	}
	if ( !searchTerms.isEmpty() )
	{
		// start a search task
		SearchTask * st = new SearchTask( m_account->client()->rootTask() );
		st->search( searchTerms );
		connect( st, SIGNAL( finished() ), SLOT( slotGotSearchResults() ) );
		st->go( true );
		m_searchUI->m_matchCount->setText( i18n( "Searching" ) );
	}
	else
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "no query to perform!" << endl;
	}
	
}

void GroupWiseAddContactPage::slotShowDetails()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "NOT IMPLEMENTED" << endl;

}

void GroupWiseAddContactPage::slotGotSearchResults()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	SearchTask * st = ( SearchTask * )sender();
	m_searchResults = st->results();
	
	m_searchUI->m_matchCount->setText( i18n( "%1 matching users found" ).arg( m_searchResults.count() ) );

	m_searchUI->m_results->clear();
	QValueList< GroupWise::ContactDetails >::Iterator it = m_searchResults.begin();
	QValueList< GroupWise::ContactDetails >::Iterator end = m_searchResults.end();
	for ( ; it != end; ++it )
	{
		int statusOrdered;
		switch ( (*it).status )
		{
			case 0: //unknown
				statusOrdered = 0;
				break;
			case 1: //offline
				statusOrdered = 1;
				break;
			case 2: //online
				statusOrdered = 5;
				break;
			case 3: //busy
				statusOrdered = 2;
				break;
			case 4: // away
				statusOrdered = 3;
				break;
			case 5: //idle
				statusOrdered = 4;
				break;
		}

		new GWSearchResultsLVI( m_searchUI->m_results, statusOrdered,
				m_account->protocol()->gwStatusToKOS( (*it).status ).iconFor( m_account ),
				(*it).givenName, (*it).surname, m_account->protocol()->dnToDotted( (*it).dn ), (*it).dn );
	}
}

unsigned char GroupWiseAddContactPage::searchOperation( int comboIndex )
{
	switch ( comboIndex )
	{
		case 0:
			return NMFIELD_METHOD_SEARCH;
		case 1:
			return NMFIELD_METHOD_MATCHBEGIN;
		case 2:
			return NMFIELD_METHOD_EQUAL;
	}
	return NMFIELD_METHOD_IGNORE;
}

#include "gwaddcontactpage.moc"
