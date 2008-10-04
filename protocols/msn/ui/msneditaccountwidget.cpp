/*
    msneditaccountwidget.cpp - MSN Account Widget

    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msneditaccountwidget.h"

#include <qcheckbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include "kopetepasswordwidget.h"
#include "avatardialog.h"

#include "msnaccount.h"
#include "msncontact.h"
#include "ui_msneditaccountui.h"
#include "msnnotifysocket.h"
#include "msnprotocol.h"

// TODO: This was using KAutoConfig before, use KConfigXT instead.
class MSNEditAccountWidgetPrivate
{
public:
	MSNProtocol *protocol;
	Ui::MSNEditAccountUI *ui;

	QString pictureUrl;
	QImage pictureData;
};

MSNEditAccountWidget::MSNEditAccountWidget( MSNProtocol *proto, Kopete::Account *account, QWidget *parent )
: QWidget( parent ), KopeteEditAccountWidget( account )
{
	d = new MSNEditAccountWidgetPrivate;

	d->protocol=proto;

	d->ui = new Ui::MSNEditAccountUI();
	d->ui->setupUi( this );

	d->ui->mainTabWidget->setCurrentIndex(0);

	// FIXME: actually, I don't know how to set fonts for qlistboxitem - Olivier
	d->ui->label_font->hide();

	// default fields
	if ( account )
	{
		KConfigGroup * config=account->configGroup();
	
		d->ui->m_login->setText( account->accountId() );
		d->ui->m_password->load( &static_cast<MSNAccount *>(account)->password() );

		//remove me after we can change account ids (Matt)
		d->ui->m_login->setReadOnly( true );
		d->ui->m_autologin->setChecked( account->excludeConnect()  );
		if ( ( static_cast<MSNAccount*>(account)->serverName() != "messenger.hotmail.com" ) || ( static_cast<MSNAccount*>(account)->serverPort() != 1863) || ( static_cast<MSNAccount*>(account)->useHttpMethod() ) ) {
			d->ui->optionOverrideServer->setChecked( true );
		}

		d->ui->optionUseHttpMethod->setChecked( static_cast<MSNAccount*>(account)->useHttpMethod() );

		MSNContact *myself = static_cast<MSNContact *>( account->myself() );

		d->ui->m_displayName->setText( myself->property( Kopete::Global::Properties::self()->nickName()).value().toString() );
		d->ui->m_phw->setText( config->readEntry("PHW") );
		d->ui->m_phm->setText( config->readEntry("PHM") );
		d->ui->m_phh->setText( config->readEntry("PHH") );

		bool connected = account->isConnected();
		if ( connected )
		{
			d->ui->m_warning_1->hide();
			d->ui->m_warning_2->hide();
		}
		d->ui->m_phones->setEnabled( connected );
		d->ui->m_displayName->setEnabled( connected );
		d->ui->m_allowButton->setEnabled( connected );
		d->ui->m_blockButton->setEnabled( connected );

		d->ui->m_allowButton->setIcon( KIcon( "arrow-left" ) );
		d->ui->m_blockButton->setIcon( KIcon( "arrow-right" ) );

		MSNAccount *m_account = static_cast<MSNAccount*>( account );
		d->ui->m_serverName->setText( m_account->serverName() );
		d->ui->m_serverPort->setValue( m_account->serverPort() );

		QStringList blockList = config->readEntry( "blockList", QStringList() );
		QStringList allowList = config->readEntry( "allowList", QStringList() );
		//QStringList reverseList =  config->readListEntry("reverseList" );

		for ( QStringList::Iterator it = blockList.begin(); it != blockList.end(); ++it )
			d->ui->m_BL->addItem( *it );

		for ( QStringList::Iterator it = allowList.begin(); it != allowList.end(); ++it )
			d->ui->m_AL->addItem( *it );

		d->ui->m_blp->setChecked( config->readEntry( "BLP" ) == "BL" );

		d->pictureUrl = config->readEntry("avatar", QString());
		d->ui->m_displayPicture->setPixmap( d->pictureUrl );

		d->ui->m_useDisplayPicture->setChecked( config->readEntry( "exportCustomPicture", false ));

		d->ui->NotifyNewChat->setChecked( config->readEntry( "NotifyNewChat", false ));
		d->ui->DownloadPicture->setCurrentIndex( config->readEntry( "DownloadPicture", 2 ));
		d->ui->useCustomEmoticons->setChecked( config->readEntry( "useCustomEmoticons", true ));
		d->ui->exportEmoticons->setChecked( config->readEntry( "exportEmoticons", false ));
		d->ui->SendClientInfo->setChecked( config->readEntry( "SendClientInfo", true ));
		d->ui->SendTypingNotification->setChecked( config->readEntry( "SendTypingNotification", true ));
	}
	else
	{
		d->ui->tab_contacts->setDisabled( true );
		d->ui->m_displayName->setDisabled( true );
		d->ui->m_phones->setDisabled( true );
	}

	connect( d->ui->m_allowButton, SIGNAL( clicked() ), this, SLOT( slotAllow() ) );
	connect( d->ui->m_blockButton, SIGNAL( clicked() ), this, SLOT( slotBlock() ) );
	connect( d->ui->m_selectImage, SIGNAL( clicked() ), this, SLOT( slotSelectImage() ) );
	connect( d->ui->m_RLButton, SIGNAL( clicked() ), this, SLOT( slotShowReverseList() ) );
	connect( d->ui->buttonRegister, SIGNAL(clicked()), this, SLOT(slotOpenRegister()));
	QWidget::setTabOrder( d->ui->m_login, d->ui->m_password->mRemembered );
	QWidget::setTabOrder( d->ui->m_password->mRemembered, d->ui->m_password->mPassword );
	QWidget::setTabOrder( d->ui->m_password->mPassword, d->ui->m_autologin );
}

MSNEditAccountWidget::~MSNEditAccountWidget()
{
	delete d->ui;
	delete d;
}

Kopete::Account * MSNEditAccountWidget::apply()
{
	if ( !account() )
		setAccount( new MSNAccount( d->protocol, d->ui->m_login->text() ) );
	
	KConfigGroup *config=account()->configGroup();

	account()->setExcludeConnect( d->ui->m_autologin->isChecked() );
	d->ui->m_password->save( &static_cast<MSNAccount *>(account())->password() );

	config->writeEntry( "NotifyNewChat", d->ui->NotifyNewChat->isChecked() );
	config->writeEntry( "DownloadPicture", d->ui->DownloadPicture->currentIndex() );
	config->writeEntry( "useCustomEmoticons", d->ui->useCustomEmoticons->isChecked() );
	config->writeEntry( "exportEmoticons", d->ui->exportEmoticons->isChecked() );
	config->writeEntry( "SendClientInfo", d->ui->SendClientInfo->isChecked() );
	config->writeEntry( "SendTypingNotification", d->ui->SendTypingNotification->isChecked() );
	
	config->writeEntry( "exportCustomPicture", d->ui->m_useDisplayPicture->isChecked() );
	if (d->ui->optionOverrideServer->isChecked() ) {
		config->writeEntry( "serverName", d->ui->m_serverName->text().trimmed() );
		config->writeEntry( "serverPort", d->ui->m_serverPort->value()  );
	}
	else {
		config->writeEntry( "serverName", "messenger.hotmail.com" );
		config->writeEntry( "serverPort", "1863" );
	}

	config->writeEntry( "useHttpMethod", d->ui->optionUseHttpMethod->isChecked() );

	// Save the avatar image
	config->writeEntry("avatar", d->pictureUrl);
	
	static_cast<MSNAccount *>( account() )->setPictureUrl( d->pictureUrl );
	static_cast<MSNAccount *>( account() )->resetPictureObject();

	if ( account()->isConnected() )
	{
		MSNContact *myself = static_cast<MSNContact *>( account()->myself() );
		MSNNotifySocket *notify = static_cast<MSNAccount *>( account() )->notifySocket();
		if ( d->ui->m_displayName->text() != myself->property( Kopete::Global::Properties::self()->nickName()).value().toString() )
			static_cast<MSNAccount *>( account() )->setPublicName( d->ui->m_displayName->text() );

		if ( notify )
		{
			if ( d->ui->m_phw->text() != myself->phoneWork() && ( !d->ui->m_phw->text().isEmpty() || !myself->phoneWork().isEmpty() ) )
				notify->changePhoneNumber( "PHW", d->ui->m_phw->text() );
			if( d->ui->m_phh->text() != myself->phoneHome() && ( !d->ui->m_phh->text().isEmpty() || !myself->phoneHome().isEmpty() ) )
				notify->changePhoneNumber( "PHH", d->ui->m_phh->text() );
			if( d->ui->m_phm->text() != myself->phoneMobile() && ( !d->ui->m_phm->text().isEmpty() || !myself->phoneMobile().isEmpty() ) )
				notify->changePhoneNumber( "PHM", d->ui->m_phm->text() );
			// (the && .isEmpty is because one can be null and the other empty)

			if ( ( config->readEntry("BLP") == "BL" ) != d->ui->m_blp->isChecked() )
			{
				// Yes, I know, calling sendCommand here is not very clean - Olivier
				notify->sendCommand( "BLP", d->ui->m_blp->isChecked() ? "BL" : "AL" );
			}
		}
	}
	return account();
}

bool MSNEditAccountWidget::validateData()
{
	QString userid = d->ui->m_login->text();
	if ( MSNProtocol::validContactId( userid ) )
		return true;

	KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
		i18n( "<qt>You must enter a valid email address.</qt>" ), i18n( "MSN Plugin" ) );
	return false;
}

void MSNEditAccountWidget::slotAllow()
{
	//TODO: play with multiple selection
	if ( d->ui->m_BL->selectedItems().isEmpty() )
		return;
	QListWidgetItem *item = d->ui->m_BL->selectedItems().at(0);

	QString handle = item->text();

	MSNNotifySocket *notify = static_cast<MSNAccount *>( account() )->notifySocket();
	if ( !notify )
		return;
	notify->removeContact( handle, MSNProtocol::BL, QString(), QString() );

	d->ui->m_BL->takeItem( d->ui->m_BL->row( item ) );
	d->ui->m_AL->addItem( item );
}

void MSNEditAccountWidget::slotBlock()
{
	//TODO: play with multiple selection
	if ( d->ui->m_AL->selectedItems().isEmpty() )
		return;
	QListWidgetItem *item = d->ui->m_AL->selectedItems().at(0);

	QString handle = item->text();

	MSNNotifySocket *notify = static_cast<MSNAccount *>( account() )->notifySocket();
	if ( !notify )
		return;

	notify->removeContact( handle, MSNProtocol::AL, QString(), QString() );

	d->ui->m_AL->takeItem( d->ui->m_AL->row( item ) );
	d->ui->m_BL->addItem( item );
}

void MSNEditAccountWidget::slotShowReverseList()
{
	QStringList reverseList = account()->configGroup()->readEntry( "reverseList", QStringList() );
	KMessageBox::informationList( this, i18n( "Here you can see a list of the contacts who have added you to their contact list" ), reverseList,
		i18n( "Reverse List - MSN Plugin" ) );
}

void MSNEditAccountWidget::slotSelectImage()
{
	bool ok;
	QString path = Kopete::UI::AvatarDialog::getAvatar(this, d->pictureUrl,&ok);
	if( !ok )
		return;
	QImage img( path );
	if(!img.isNull()) 
	{
		d->ui->m_displayPicture->setPixmap( QPixmap::fromImage(img) );
		d->pictureData = img;
		d->pictureUrl = path;
	}
	else
	{
		KMessageBox::sorry( this, i18n( "<qt>An error occurred when trying to change the display picture.<br />"
			"Make sure that you have selected a valid image file</qt>" ), i18n( "MSN Plugin" ) );
	}
}

void MSNEditAccountWidget::slotOpenRegister()
{
	KToolInvocation::invokeBrowser( "http://register.passport.net/"  );
}

#include "msneditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

