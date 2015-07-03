/*
    Kopete GroupWise Protocol
    gweditaccountwidget.cpp - widget for adding GroupWise contacts

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
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
#include <QPainter>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <q3valuelist.h>
#include <QVBoxLayout>

#include <kdebug.h>
#include <klocale.h>

#include "kopeteaccount.h"
#include "kopetemetacontact.h"

#include "client.h"
#include "gwaccount.h"
#include "gwerror.h"
#include "gwprotocol.h"
#include "gwsearch.h"
#include "ui_gwaddui.h"
#include "userdetailsmanager.h"

GroupWiseAddContactPage::GroupWiseAddContactPage( Kopete::Account * owner, QWidget* parent )
		: AddContactPage(parent)
{
	m_account = static_cast<GroupWiseAccount *>( owner );
	kDebug() ;
	QVBoxLayout * layout = new QVBoxLayout( this );
	if (owner->isConnected ())
	{
		m_searchUI = new GroupWiseContactSearch( m_account, QAbstractItemView::SingleSelection, false,
				 this );
		connect(m_searchUI, SIGNAL(selectionValidates(bool)),SLOT(searchResult(bool)));
		layout->addWidget( m_searchUI );
		m_canadd = false;
	}
	else
	{
		m_noaddMsg1 = new QLabel (i18n ("You need to be connected to be able to add contacts."), this);
		m_noaddMsg2 = new QLabel (i18n ("Connect to GroupWise Messenger and try again."), this);
		layout->addWidget( m_noaddMsg1 );
		layout->addWidget( m_noaddMsg2 );
		m_canadd = false;
	}
	setLayout( layout );
	show();
}

GroupWiseAddContactPage::~GroupWiseAddContactPage()
{
// , i18n( "The search was cancelled" )
// , i18n( "There was an error while carrying out your search.  Please change your search terms or try again later." )
// i18n( "There was an error while carrying out your search.  Please change your search terms or try again later." )
}

bool GroupWiseAddContactPage::apply( Kopete::Account* account, Kopete::MetaContact* parentContact )
{
	if ( validateData() )
	{
		QString contactId;

		ContactDetails dt;
		QList< ContactDetails > selected = m_searchUI->selectedResults();
		if ( selected.count() == 1 )
		{
			dt = selected.first();
			m_account->client()->userDetailsManager()->addDetails( dt );
		}
		else
			return false;

		return ( account->addContact ( dt.dn, parentContact, Kopete::Account::ChangeKABC ) );
	}
	else
		return false;
}

bool GroupWiseAddContactPage::validateData()
{
	return m_canadd;
}

void GroupWiseAddContactPage::searchResult(bool valid)
{
	m_canadd = valid;
}

#include "gwaddcontactpage.moc"
