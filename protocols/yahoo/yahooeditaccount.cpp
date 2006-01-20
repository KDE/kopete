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

// QT Includes
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>

// KDE Includes
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <kpassdlg.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kpixmapregionselectordialog.h>

// Kopete Includes
#include <addcontactpage.h>

// Local Includes
#include "yahooaccount.h"
#include "yahoocontact.h"
#include "yahooeditaccount.h"

// Yahoo Add Contact page
YahooEditAccount::YahooEditAccount(YahooProtocol *protocol, Kopete::Account *theAccount, QWidget *parent, const char* /*name*/): YahooEditAccountBase(parent), KopeteEditAccountWidget(theAccount)
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	theProtocol = protocol;

	mPasswordWidget = new Kopete::UI::PasswordWidget( mAccountInfo );
	mAccountInfoLayout->add( mPasswordWidget );

	if(YahooAccount *acct = dynamic_cast<YahooAccount*>(account()))
	{	mScreenName->setText(acct->accountId());
		mScreenName->setReadOnly(true); //the accountId is Constant FIXME: remove soon!
		mScreenName->setDisabled(true);
		mAutoConnect->setChecked(acct->excludeConnect());
		mPasswordWidget->load( &acct->password() );

		QString pagerServer = account()->configGroup()->readEntry("Server", "scs.msg.yahoo.com");
		int pagerPort = account()->configGroup()->readNumEntry("Port", 5050);
		if( pagerServer != "scs.msg.yahoo.com" || pagerPort != 5050 )
			optionOverrideServer->setChecked( true );
		else
			optionOverrideServer->setChecked( false );
		editServerAddress->setText( pagerServer );
		sbxServerPort->setValue( pagerPort );

		QString iconUrl = account()->configGroup()->readEntry("pictureUrl", "");
		bool sendPicture = account()->configGroup()->readBoolEntry("sendPicture", false);
		optionSendBuddyIcon->setChecked( sendPicture );
    buttonSelectPicture->setEnabled( sendPicture );  
    connect( optionSendBuddyIcon, SIGNAL( toggled( bool ) ), buttonSelectPicture, SLOT( setEnabled( bool ) ) ); 
		editPictureUrl->setText( iconUrl );
		if( !iconUrl.isEmpty() )
			m_Picture->setPixmap( KURL( iconUrl ).path() );
		editPictureUrl->setEnabled( sendPicture );

		// Global Identity
		mGlobalIdentity->setChecked( account()->configGroup()->readBoolEntry("ExcludeGlobalIdentity", false) );
	}

	QObject::connect(buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));
	QObject::connect(buttonSelectPicture, SIGNAL(clicked()), this, SLOT(slotSelectPicture()));

	optionSendBuddyIcon->setEnabled( account() );

	/* Set tab order to password custom widget correctly */
	QWidget::setTabOrder( mAutoConnect, mPasswordWidget->mRemembered );
	QWidget::setTabOrder( mPasswordWidget->mRemembered, mPasswordWidget->mPassword );
	QWidget::setTabOrder( mPasswordWidget->mPassword, buttonRegister );

	show();
}

bool YahooEditAccount::validateData()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

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
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	if ( !account() )
		setAccount( new YahooAccount( theProtocol, mScreenName->text().lower() ) );

	YahooAccount *yahooAccount = static_cast<YahooAccount *>( account() );

	yahooAccount->setExcludeConnect( mAutoConnect->isChecked() );

	mPasswordWidget->save( &yahooAccount->password() );

	if ( optionOverrideServer->isChecked() )
	{
		yahooAccount->setServer( editServerAddress->text() );
		yahooAccount->setPort( sbxServerPort->value() );
	}
	else
	{
		yahooAccount->setServer( "scs.msg.yahoo.com" );
		yahooAccount->setPort( 5050 );
	}

	account()->configGroup()->writeEntry("pictureUrl", editPictureUrl->text() );
	account()->configGroup()->writeEntry("sendPicture", optionSendBuddyIcon->isChecked() );
	if ( optionSendBuddyIcon->isChecked() )
	{
		yahooAccount->setBuddyIcon( editPictureUrl->text() );
	}
	else
	{
		yahooAccount->setBuddyIcon( KURL( QString::null ) );
	}
	
	// Global Identity
	account()->configGroup()->writeEntry("ExcludeGlobalIdentity", mGlobalIdentity->isChecked() );

	return yahooAccount;
}

void YahooEditAccount::slotOpenRegister()
{
    KRun::runURL( "http://edit.yahoo.com/config/eval_register?new=1", "text/html" );
}

void YahooEditAccount::slotSelectPicture()
{
	KURL file = KFileDialog::getImageOpenURL( QString::null, this, i18n( "Yahoo Buddy Icon" ) );

	if ( file.isEmpty() )
		return;

	QImage picture(file.path());
	if( !picture.isNull() )
	{
		picture = KPixmapRegionSelectorDialog::getSelectedImage( QPixmap(picture), 96, 96, this );
		QString newlocation( locateLocal( "appdata", "yahoopictures/"+ file.fileName().lower() ) ) ;
		file = KURL(newlocation);
		if( !picture.save( newlocation, "PNG" ))
		{
			KMessageBox::sorry( this, i18n( "An error occurred when trying to change the display picture." ), i18n( "Yahoo Plugin" ) );
			return;
		}
	}
	else
	{
		KMessageBox::sorry( this, i18n( "<qt>The selected buddy icon could not be opened. <br>Please set a new buddy icon.</qt>" ), i18n( "Yahoo Plugin" ) );
		return;
	}
	editPictureUrl->setText( file.path() );
	
	m_Picture->setPixmap( file.path() );
}

#include "yahooeditaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

