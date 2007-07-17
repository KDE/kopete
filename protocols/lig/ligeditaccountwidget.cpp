/*
    ligeditaccountwidget.h - Kopete Lig Protocol

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

#include "ligeditaccountwidget.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <kdebug.h>
#include "kopeteaccount.h"
#include "ligaccountpreferences.h"
#include "ligaccount.h"
#include "ligprotocol.h"

LigEditAccountWidget::LigEditAccountWidget( QWidget* parent, Kopete::Account* account)
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
				kdDebug(14210) << k_funcinfo << endl;
	m_preferencesWidget = new LigAccountPreferences( this );
}

LigEditAccountWidget::~LigEditAccountWidget()
{
}

Kopete::Account* LigEditAccountWidget::apply()
{
	QString accountName;
	if ( m_preferencesWidget->m_login->text().isEmpty() )
		accountName = "Lig Account";
	else
		accountName = m_preferencesWidget->m_login->text();
	
	if ( account() )
		// FIXME: ? account()->setAccountLabel(accountName);
		account()->myself()->setProperty( Kopete::Global::Properties::self()->nickName(), accountName );
	else
		setAccount( new LigAccount( LigProtocol::protocol(), accountName ) );

	return account();
}

bool LigEditAccountWidget::validateData()
{
    //return !( m_preferencesWidget->m_acctName->text().isEmpty() );
	return true;
}

#include "ligeditaccountwidget.moc"
