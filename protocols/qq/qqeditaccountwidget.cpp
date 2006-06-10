/*
    qqeditaccountwidget.h - Kopete QQ Protocol

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

#include "qqeditaccountwidget.h"

#include <qlayout.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <kdebug.h>
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "ui_qqeditaccountui.h"
#include "qqaccount.h"
#include "qqprotocol.h"

QQEditAccountWidget::QQEditAccountWidget( QWidget* parent, Kopete::Account* account)
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
				kDebug(14210) << k_funcinfo << endl;
	m_preferencesWidget = new Ui::QQEditAccountUI();
	m_preferencesWidget->setupUi( this );

	/* TODO: load the default value from configuration */

}

QQEditAccountWidget::~QQEditAccountWidget()
{
	delete m_preferencesWidget;
}

Kopete::Account* QQEditAccountWidget::apply()
{
	QString accountName;
	if ( m_preferencesWidget->m_login->text().isEmpty() )
		accountName = "QQ Account";
	else
		accountName = m_preferencesWidget->m_login->text();
	
	if ( account() )
		// FIXME: ? account()->setAccountLabel(accountName);
		account()->myself()->setProperty( Kopete::Global::Properties::self()->nickName(), accountName );
	else
		setAccount( new QQAccount( QQProtocol::protocol(), accountName ) );

	kDebug(14210) << k_funcinfo << accountName << endl;
	return account();
}

bool QQEditAccountWidget::validateData()
{
    //return !( m_preferencesWidget->m_acctName->text().isEmpty() );
	return true;
}

#include "qqeditaccountwidget.moc"
