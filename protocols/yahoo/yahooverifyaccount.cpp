/*
    yahooverifyaccount.cpp - UI Page for Verifying a locked account

    Copyright (c) 2005 by André Duffeck          <andre.duffeck@kdemail.net>

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
#include <qlayout.h>
#include <qfile.h>
#include <qlabel.h>

// KDE Includes
#include <kdebug.h>
#include <klineedit.h>
#include <ktempfile.h>
#include <klocale.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kstandarddirs.h>

// Kopete Includes
#include <yahooverifyaccountbase.h>
#include <kopeteaccount.h>

// Local Includes
#include "yahooverifyaccountbase.h"
#include "yahooverifyaccount.h"
#include "yahooaccount.h"

YahooVerifyAccount::YahooVerifyAccount(Kopete::Account *account, QWidget *parent, const char *name)
: KDialogBase(parent, name, true, i18n("Account Verification - Yahoo"), Cancel|Apply,
              Apply, true )
{
	mTheAccount = account;	
	mTheDialog = new YahooVerifyAccountBase( this );
	mTheDialog->mPicture->hide();
	setMainWidget( mTheDialog );
	setEscapeButton( Cancel );
}

// Destructor
YahooVerifyAccount::~YahooVerifyAccount()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
}

void YahooVerifyAccount::setUrl( KURL url )
{
	mFile = new KTempFile( locateLocal( "tmp", url.fileName() ) );
	mFile->setAutoDelete( true );
	KIO::TransferJob *transfer = KIO::get( url, false, false );
	connect( transfer, SIGNAL( result( KIO::Job* ) ), this, SLOT( slotComplete( KIO::Job* ) ) );
	connect( transfer, SIGNAL( data( KIO::Job*, const QByteArray& ) ), this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
}

void YahooVerifyAccount::slotData( KIO::Job */*job*/, const QByteArray& data )
{

	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	mFile->file()->writeBlock( data.data() , data.size() );
}

void YahooVerifyAccount::slotComplete( KIO::Job */*job*/ )
{

	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;
	mFile->file()->close();
	mTheDialog->mPicture->setPixmap( mFile->file()->name() );
	mTheDialog->mPicture->show();
}

bool YahooVerifyAccount::validateData()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	return ( !mTheDialog->mWord->text().isEmpty() );
}

void YahooVerifyAccount::slotClose()
{
	QDialog::done(0);
}

void YahooVerifyAccount::slotApply()
{
	kdDebug(YAHOO_GEN_DEBUG) << k_funcinfo << endl;

	YahooAccount* myAccount = static_cast<YahooAccount*>(mTheAccount);
	myAccount->verifyAccount( mTheDialog->mWord->text() );
	QDialog::done(0);
}

#include "yahooverifyaccount.moc"

// vim: set noet ts=4 sts=4 sw=4:

