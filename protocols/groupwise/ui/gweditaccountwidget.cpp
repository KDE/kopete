/*
    Kopete GroupWise Protocol
    gweditaccountwidget.cpp - widget for adding or editing GroupWise accounts

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

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kopetepasswordedaccount.h"
#include "kopetepasswordwidget.h"

#include "gwaccountpreferences.h"
#include "gwaccount.h"
#include "gwerror.h"
#include "gwprotocol.h"

#include "gweditaccountwidget.h"

GroupWiseEditAccountWidget::GroupWiseEditAccountWidget( QWidget* parent, KopeteAccount* theAccount)
: QWidget( parent ), KopeteEditAccountWidget( theAccount )
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << endl;
	m_layout = new QVBoxLayout( this );
	m_preferencesDialog = new GroupWiseAccountPreferences( this );
	m_layout->addWidget( m_preferencesDialog );
	connect( m_preferencesDialog->m_password, SIGNAL( changed() ), this, SLOT( configChanged() ) );
	connect( m_preferencesDialog->m_server, SIGNAL( textChanged( const QString & ) ), this, SLOT( configChanged() ) );
	connect( m_preferencesDialog->m_port, SIGNAL( valueChanged( int ) ), this, SLOT( configChanged() ) );
	if ( account() )
		reOpen();
	else
	{
		// look for a default server and port setting
		KConfig *config = kapp->config();
		config->setGroup("GroupWise Messenger");
		m_preferencesDialog->m_server->setText( config->readEntry( "DefaultServer" ) );
		m_preferencesDialog->m_port->setValue( config->readNumEntry( "DefaultPort", 8300 ) );
	}
}

GroupWiseEditAccountWidget::~GroupWiseEditAccountWidget()
{
}

GroupWiseAccount *GroupWiseEditAccountWidget::account ()
{
	return dynamic_cast< GroupWiseAccount *>(KopeteEditAccountWidget::account () );
}

void GroupWiseEditAccountWidget::reOpen()
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << endl;
	
	m_preferencesDialog->m_password->load( &account()->password () );
	// Kopete at least <=0.90 doesn't support changing account IDs
	m_preferencesDialog->m_userId->setDisabled( true );
	m_preferencesDialog->m_userId->setText( account()->accountId() );
	m_preferencesDialog->m_password->load( &account()->password() );
	m_preferencesDialog->m_server->setText( account()->pluginData( GroupWiseProtocol::protocol(), "Server") );
	m_preferencesDialog->m_port->setValue( account()->pluginData( GroupWiseProtocol::protocol(), "Port" ).toInt() );
	m_preferencesDialog->m_autoConnect->setChecked( account()->autoLogin() );
	
}

KopeteAccount* GroupWiseEditAccountWidget::apply()
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << endl;
		
	if ( !account() )
		setAccount( new GroupWiseAccount( GroupWiseProtocol::protocol(), m_preferencesDialog->m_userId->text() ) );
	
	if(account()->isConnected())
	{
		KMessageBox::information(this,
					i18n("The changes you just made will take effect next time you log in with GroupWise."),
					i18n("GroupWise settings changed while signed in"));
	}

	writeConfig();

	return account();
}

bool GroupWiseEditAccountWidget::validateData()
{
    return !( m_preferencesDialog->m_userId->text().isEmpty() || m_preferencesDialog->m_server->text().isEmpty() );
}

void GroupWiseEditAccountWidget::writeConfig()
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << k_funcinfo << endl;
	account()->setPluginData( GroupWiseProtocol::protocol(), "Server", m_preferencesDialog->m_server->text() );
	account()->setPluginData( GroupWiseProtocol::protocol(), "Port", QString::number( m_preferencesDialog->m_port->value() ) );
	account()->setAutoLogin( m_preferencesDialog->m_autoConnect->isChecked() );
	m_preferencesDialog->m_password->save( &account()->password() );
	settings_changed = false;
}

void GroupWiseEditAccountWidget::configChanged ()
{
	settings_changed = true;
}

#include "gweditaccountwidget.moc"
