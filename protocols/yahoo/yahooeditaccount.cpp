/*
    yahooeditaccount.cpp - UI Page to edit a Yahoo account

    Copyright (c) 2003 by Matt Rogers <mattrogers@sbcglobal.net>
    Copyright (c) 2002 by Gav Wood <gav@kde.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

// Own header
#include "yahooeditaccount.h"

// QT Includes
#include <QCheckBox>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QPixmap>
#include <QLatin1String>

// KDE Includes
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <kpassworddialog.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>

// Kopete Includes
#include <addcontactpage.h>
#include <kopeteuiglobal.h>
#include <avatardialog.h>

// Local Includes
#include "yahooaccount.h"
#include "yahoocontact.h"

// Yahoo Add Contact page
YahooEditAccount::YahooEditAccount(YahooProtocol *protocol, Kopete::Account *theAccount, QWidget *parent): QWidget(parent), KopeteEditAccountWidget(theAccount)
{
	setupUi(this);
	kDebug(YAHOO_GEN_DEBUG) ;

	theProtocol = protocol;

	if(YahooAccount *acct = dynamic_cast<YahooAccount*>(account()))
	{	mScreenName->setText(acct->accountId());
		mScreenName->setReadOnly(true); //the accountId is Constant FIXME: remove soon!
		mAutoConnect->setChecked(acct->excludeConnect());
		mPasswordWidget->load( &acct->password() );

		QString pagerServer = account()->configGroup()->readEntry("Server", "scsa.msg.yahoo.com");
		int pagerPort = account()->configGroup()->readEntry("Port", 5050);
		if( pagerServer != "scsa.msg.yahoo.com" || pagerPort != 5050 )
			optionOverrideServer->setChecked( true );
		else
			optionOverrideServer->setChecked( false );
		editServerAddress->setText( pagerServer );
		sbxServerPort->setValue( pagerPort );

		QString iconUrl = account()->configGroup()->readEntry("pictureUrl", "");
		bool sendPicture = account()->configGroup()->readEntry("sendPicture", false);
		optionSendBuddyIcon->setChecked( sendPicture );
		buttonSelectPicture->setEnabled( sendPicture );  
		connect( optionSendBuddyIcon, SIGNAL(toggled(bool)), buttonSelectPicture, SLOT(setEnabled(bool)) ); 
		m_photoPath = iconUrl;
		if( !iconUrl.isEmpty() )
			m_Picture->setPixmap( iconUrl );
		groupBox->hide();
	}

	QObject::connect(buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));
	QObject::connect(buttonSelectPicture, SIGNAL(clicked()), this, SLOT(slotSelectPicture()));

	optionSendBuddyIcon->setEnabled( account() );

	/* Set tab order to password custom widget correctly */
	QWidget::setTabOrder( mScreenName, mPasswordWidget->mRemembered );
	QWidget::setTabOrder( mPasswordWidget->mRemembered, mPasswordWidget->mPassword );
	QWidget::setTabOrder( mPasswordWidget->mPassword, mAutoConnect );

	show();
}

bool YahooEditAccount::validateData()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if(mScreenName->text().isEmpty())
	{	KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
			i18n("<qt>You must enter a valid screen name.</qt>"), i18n("Yahoo"));
		return false;
	}
	if(!mPasswordWidget->validate())
	{	KMessageBox::queuedMessageBox(this, KMessageBox::Sorry,
			i18n("<qt>You must enter a valid password.</qt>"), i18n("Yahoo"));
		return false;
	}
	return true;
}

Kopete::Account *YahooEditAccount::apply()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if ( !account() )
		setAccount( new YahooAccount( theProtocol, mScreenName->text().toLower() ) );

	YahooAccount *yahooAccount = static_cast<YahooAccount *>( account() );

	yahooAccount->setExcludeConnect( mAutoConnect->isChecked() );

	mPasswordWidget->save( &yahooAccount->password() );

	if ( optionOverrideServer->isChecked() )
	{
		yahooAccount->setServer( editServerAddress->text().trimmed() );
		yahooAccount->setPort( sbxServerPort->value() );
	}
	else
	{
		yahooAccount->setServer( "scsa.msg.yahoo.com" );
		yahooAccount->setPort( 5050 );
	}

	account()->configGroup()->writeEntry("pictureUrl", m_photoPath );
	account()->configGroup()->writeEntry("sendPicture", optionSendBuddyIcon->isChecked() );
	if ( optionSendBuddyIcon->isChecked() )
	{
		yahooAccount->setBuddyIcon( m_photoPath );
	}
	else
	{
		yahooAccount->setBuddyIcon( KUrl() );
	}
	
	return yahooAccount;
}

void YahooEditAccount::slotOpenRegister()
{
	KToolInvocation::invokeBrowser( QLatin1String("http://edit.yahoo.com/config/eval_register?new=1") );
}

void YahooEditAccount::slotSelectPicture()
{
	QString file = Kopete::UI::AvatarDialog::getAvatar( this, m_photoPath );

	QPixmap pix(file);
	if( !pix.isNull() )
	{
		m_photoPath =  file;
		m_Picture->setPixmap( pix );
	}
	else
	{
		KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "<qt>The selected buddy icon could not be opened. <br />Please set a new buddy icon.</qt>" ), i18n( "Yahoo Plugin" ) );
		return;
	}
}

#include "yahooeditaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

