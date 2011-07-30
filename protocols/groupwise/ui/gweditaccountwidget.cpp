/*
    Kopete GroupWise Protocol
    gweditaccountwidget.cpp - widget for adding or editing GroupWise accounts

    Copyright (c) 2006,2007 Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003-2007 by Will Stephenson		 <wstephenson@kde.org>
    
    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

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
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <QVBoxLayout>

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassworddialog.h>
#include <kglobal.h>
#include "kopetepasswordedaccount.h"
#include "kopetepasswordwidget.h"

#include "gwaccount.h"
#include "gwerror.h"
#include "gwprotocol.h"

GroupWiseEditAccountWidget::GroupWiseEditAccountWidget( QWidget* parent, Kopete::Account* theAccount)
: QWidget( parent ), KopeteEditAccountWidget( theAccount )
{
	kDebug() ;
	m_layout = new QVBoxLayout( this );
	QWidget * wid = new QWidget;
	m_ui.setupUi( wid );
	m_layout->addWidget( wid );
	connect( m_ui.password, SIGNAL(changed()), this, SLOT(configChanged()) );
	connect( m_ui.server, SIGNAL(textChanged(QString)), this, SLOT(configChanged()) );
	connect( m_ui.port, SIGNAL(valueChanged(int)), this, SLOT(configChanged()) );
	if ( account() )
		reOpen();
	else
	{
		// look for a default server and port setting
		KConfigGroup config = KGlobal::config()->group("GroupWise Messenger");
		m_ui.server->setText( config.readEntry( "DefaultServer" ) );
		m_ui.port->setValue( config.readEntry( "DefaultPort", 8300 ) );
	}
	QWidget::setTabOrder( m_ui.userId, m_ui.password->mRemembered );
	QWidget::setTabOrder( m_ui.password->mRemembered, m_ui.password->mPassword );
	QWidget::setTabOrder( m_ui.password->mPassword, m_ui.autoConnect );

}

GroupWiseEditAccountWidget::~GroupWiseEditAccountWidget()
{
}

GroupWiseAccount *GroupWiseEditAccountWidget::account ()
{
	return dynamic_cast< GroupWiseAccount *>( KopeteEditAccountWidget::account() );
}

void GroupWiseEditAccountWidget::reOpen()
{
	kDebug() ;
	
	m_ui.password->load( &account()->password () );
	// Kopete at least <=0.90 doesn't support changing account IDs
	m_ui.userId->setReadOnly( true );
	m_ui.userId->setText( account()->accountId() );
	m_ui.password->load( &account()->password() );
	m_ui.server->setText( account()->configGroup()->readEntry( "Server") );
	
	m_ui.port->setValue( account()->configGroup()->readEntry( "Port", 0 ) );
	m_ui.autoConnect->setChecked( account()->excludeConnect() );
	m_ui.alwaysAccept->setChecked( account()->configGroup()->readEntry( "AlwaysAcceptInvitations", false ) );
}

Kopete::Account* GroupWiseEditAccountWidget::apply()
{
	kDebug() ;
		
	if ( !account() )
		setAccount( new GroupWiseAccount( GroupWiseProtocol::protocol(), m_ui.userId->text() ) );
	
	if(account()->isConnected())
	{
		KMessageBox::information(this,
					i18n("The changes you just made will take effect next time you log in with GroupWise."),
					i18n("GroupWise Settings Changed While Signed In"));
	}

	writeConfig();

	return account();
}

bool GroupWiseEditAccountWidget::validateData()
{
    return !( m_ui.userId->text().isEmpty() || m_ui.server->text().isEmpty() );
}

void GroupWiseEditAccountWidget::writeConfig()
{
	kDebug() ;
	account()->configGroup()->writeEntry( "Server", m_ui.server->text().trimmed() );
	account()->configGroup()->writeEntry( "Port", QString::number( m_ui.port->value() ) );
	account()->configGroup()->writeEntry( "AlwaysAcceptInvitations", 
			m_ui.alwaysAccept->isChecked() ? "true" : "false" );
	
	account()->setExcludeConnect( m_ui.autoConnect->isChecked() );
	m_ui.password->save( &account()->password() );
	settings_changed = false;
}

void GroupWiseEditAccountWidget::configChanged ()
{
	settings_changed = true;
}

#include "gweditaccountwidget.moc"
