/*
    jabbereditaccountwidget.cpp - Jabber Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart@tiscalinet.be>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "jabbereditaccountwidget.h"
#include "jabberprotocol.h"
#include "jabberaccount.h"
#include "dlgpreferences.h"

JabberEditAccountWidget::JabberEditAccountWidget(JabberProtocol *proto, JabberAccount *ident, QWidget *parent, const char * name )
				  : DlgPreferences(parent, name), EditAccountWidget(ident)
{
  m_protocol = proto;

  if (m_account)
  {
	QString serverInfo = m_account->accountId().section('@',1);
	QString tport = serverInfo.section(':',1,1);

	mID->setText(m_account->accountId().section('@',0,0));
	mPass->setText(m_account->getPassword());
	mServer->setText(serverInfo.section(':',0,0));
	mPort->setValue(tport.section('/',0,0).toUInt());
	mResource->setText(serverInfo.section('/',1,1));
  }

  account = m_account;

  connect (mID, SIGNAL(textChanged(const QString &)), this, SLOT(configChanged()));
  connect (mPass, SIGNAL(textChanged(const QString &)), this, SLOT(configChanged()));
  connect (mResource, SIGNAL(textChanged(const QString &)), this, SLOT(configChanged()));
  connect (mServer, SIGNAL(textChanged(const QString &)), this, SLOT(configChanged()));
  connect (mPort, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
  connect (mAutoConnect, SIGNAL(toggled(bool)), this, SLOT(configChanged()));
  connect (chkUseSSL, SIGNAL(toggled(bool)), this, SLOT(configChanged()));
  connect (chkRemPass, SIGNAL(toggled(bool)), this, SLOT(configChanged()));

 // Chat TAB
  connect (mLogAll, SIGNAL(toggled(bool)), this, SLOT(configChanged()));

  connect (cmbAuth, SIGNAL(activated(int)), this, SLOT(configChanged()));

  connect (cbProxyType, SIGNAL(activated(int)), this, SLOT(configChanged()));
  connect (leProxyName, SIGNAL(textChanged(const QString &)), this, SLOT(configChanged()));
  connect (spbProxyPort, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
  connect (cbProxyAuth, SIGNAL(toggled(bool)), this, SLOT(configChanged()));
  connect (leProxyUser, SIGNAL(textChanged(const QString &)), this, SLOT(configChanged()));
  connect (leProxyPass, SIGNAL(textChanged(const QString &)), this, SLOT(configChanged()));

  connect (btnRegister, SIGNAL(clicked()), this, SLOT(registerClicked()));
  connect (chkUseSSL, SIGNAL(toggled(bool)), this, SLOT(sslToggled(bool)));

}

JabberEditAccountWidget::~JabberEditAccountWidget()
{
}


KopeteAccount *JabberEditAccountWidget::apply()
{
	kdDebug(14180) << "JabberEditAccount::apply()";
	if (!m_account) {
		m_account = new JabberAccount(m_protocol, 
					      mID->text() + "@" + mServer->text() + ":" + mPort->text() + "/" +
					      mResource->text(), 
					      mID->text() + "@" + mServer->text() + ":" + mPort->text() + "/" +
					      mResource->text()); 
	}
	else {
		m_account->setAccountId(mID->text() + "@" + mServer->text() + ":" + mPort->text() + "/" +
		                      mResource->text());
	}
		
	return account;
}


bool JabberEditAccountWidget::validateData()
{
	return true;
}

void JabberEditAccountWidget::configChanged()
{
        settings_changed = true;
}

void JabberEditAccountWidget::registerClicked()
{
	if (!m_account) {
		m_account = new JabberAccount(m_protocol, 
					      mID->text() + "@" + mServer->text() + ":" + mPort->text() + "/" +
					      mResource->text(), 
					      mID->text() + "@" + mServer->text() + ":" + mPort->text() + "/" +
					      mResource->text()); 
		account = m_account;
	}
	else {
		account->setAccountId(mID->text() + "@" + mServer->text() + ":" + mPort->text() + "/" +
		                      mResource->text());
	}
	
	account->registerUser();
}

void JabberEditAccountWidget::sslToggled(bool value)
{
      if (value)
      	mPort->stepUp(); 
      else
      	mPort->stepDown(); 
}

void JabberEditAccountWidget::remPassToggled(bool value)
{
      if (value)
      	m_account->setPassword(mPass->text());
      else
      	m_account->setPassword(NULL); 
}

#include "jabbereditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

