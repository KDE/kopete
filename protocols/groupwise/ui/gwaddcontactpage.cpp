/*
    Kopete GroupWise Protocol
    gweditaccountwidget.cpp - widget for adding GroupWise contacts

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

//#include <qcombobox.h>
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
//#include "gwprotocol.h"
#include "gwsearch.h"
#include "gwaddui.h"
#include "userdetailsmanager.h"

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
 		m_searchUI = new GroupWiseSearch( m_account, QListView::Single,
				 m_gwAddUI->m_tabWidget->page( 1 ), "searchwidget" );
		QHBoxLayout * hb = new QHBoxLayout( m_gwAddUI->m_tabWidget->page( 1 ) );
		hb->addStretch( 2 );
		hb->setAutoAdd( true );
		QPushButton * searchButton = new QPushButton( i18n( "&Search" ), m_gwAddUI->m_tabWidget->page( 1 ), "searchbutton" );

		connect( searchButton, SIGNAL( clicked() ), m_searchUI, SLOT( doSearch() ) );
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
			QValueList< ContactDetails > selected = m_searchUI->selectedResults();
			if ( selected.count() == 1 )
			{
				ContactDetails dt = selected.first();
				m_account->client()->userDetailsManager()->addDetails( dt );
				contactId = dt.dn;
				displayName = dt.givenName + " " + dt.surname;
			}
			else
				return false;
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

#include "gwaddcontactpage.moc"
