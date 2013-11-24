/*
    yahooverifyaccount.cpp - UI Page for Verifying a locked account

    Copyright (c) 2005 by André Duffeck          <duffeck@kde.org>

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
#include "yahooverifyaccount.h"

// QT Includes
#include <qlayout.h>
#include <qfile.h>
#include <qlabel.h>

// KDE Includes
#include <kdebug.h>
#include <klineedit.h>
#include <ktemporaryfile.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kstandarddirs.h>

// Kopete Includes
#include <kopeteaccount.h>

// Local Includes
#include "ui_yahooverifyaccountbase.h"
#include "yahooaccount.h"

YahooVerifyAccount::YahooVerifyAccount(Kopete::Account *account, QWidget *parent)
: KDialog(parent)
{
	setCaption( i18n("Account Verification - Yahoo") );
	setButtons( KDialog::Cancel | KDialog::Apply );
	setDefaultButton(KDialog::Apply);
	showButtonSeparator(true);

	mTheAccount = account;	
	QWidget* w = new QWidget( this );
	mTheDialog = new Ui::YahooVerifyAccountBase;
	mTheDialog->setupUi( w );
	mTheDialog->mPicture->hide();
	setMainWidget( w );
	setEscapeButton( Cancel );
	connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

// Destructor
YahooVerifyAccount::~YahooVerifyAccount()
{
	kDebug(YAHOO_GEN_DEBUG) ;
	delete mTheDialog;
}

void YahooVerifyAccount::setUrl( const KUrl &url )
{
	mFile = new KTemporaryFile();
	mFile->setPrefix(url.fileName());
	mFile->open();
	KIO::TransferJob *transfer = KIO::get( url, KIO::NoReload, KIO::HideProgressInfo );
	connect( transfer, SIGNAL(result(KJob*)), this, SLOT(slotComplete(KJob*)) );
	connect( transfer, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotData(KIO::Job*,QByteArray)) );
}

void YahooVerifyAccount::slotData( KIO::Job */*job*/, const QByteArray& data )
{

	kDebug(YAHOO_GEN_DEBUG) ;

	mFile->write( data.data() , data.size() );
}

void YahooVerifyAccount::slotComplete( KJob */*job*/ )
{

	kDebug(YAHOO_GEN_DEBUG) ;
	mFile->close();
	mTheDialog->mPicture->setPixmap( mFile->fileName() );
	mTheDialog->mPicture->show();
}

bool YahooVerifyAccount::validateData()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	return ( !mTheDialog->mWord->text().isEmpty() );
}

void YahooVerifyAccount::slotClose()
{
	QDialog::done(0);
}

void YahooVerifyAccount::slotApply()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	YahooAccount* myAccount = static_cast<YahooAccount*>(mTheAccount);
	myAccount->verifyAccount( mTheDialog->mWord->text() );
	QDialog::done(0);
}

#include "yahooverifyaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

