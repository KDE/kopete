/*
    gweditaccountwidget.h - Kopete GroupWise Protocol

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

#include "gweditaccountwidget.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <kdebug.h>
#include "kopeteaccount.h"
#include "gwaccountpreferences.h"
#include "gwaccount.h"
#include "gwprotocol.h"

GroupWiseEditAccountWidget::GroupWiseEditAccountWidget( QWidget* parent, KopeteAccount* account)
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	kdDebug(14210) << k_funcinfo << endl;
	m_layout = new QVBoxLayout( this );
	m_preferencesDialog = new GroupWiseAccountPreferences( this );
	m_layout->addWidget( m_preferencesDialog );
}

GroupWiseEditAccountWidget::~GroupWiseEditAccountWidget()
{
}

KopeteAccount* GroupWiseEditAccountWidget::apply()
{
	QString accountName;
	if ( m_preferencesDialog->m_acctName->text().isEmpty() )
		accountName = "GroupWise Account";
	else
		accountName = m_preferencesDialog->m_acctName->text();
	
	if ( account() )
		account()->setAccountId( accountName );
	else
		setAccount( new GroupWiseAccount( GroupwiseProtocol::protocol(), accountName ) );

	return account();
}

bool GroupwiseEditAccountWidget::validateData()
{
    //return !( m_preferencesDialog->m_acctName->text().isEmpty() );
	return true;
}

#include "gweditaccountwidget.moc"
