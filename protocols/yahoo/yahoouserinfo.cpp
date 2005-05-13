/*
    yahoouserinfo.cpp - hold and display buddy information

    Copyright (c) 2005 by Andre Duffeck <andre@duffeck.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

// KDE Includes
#include <klocale.h>
#include <kdebug.h>

// Local Includes
#include "yahoouserinfo.h"
#include "kyahoo.h"

// QT Includes
#include <qlineedit.h>

YahooUserInfoDialog::YahooUserInfoDialog(QWidget* parent, const char* name)
: KDialogBase(parent, name, true, i18n("User Information"), Cancel|Apply|User1,
              Cancel, true, i18n("&View Yahoo Profile") )
{
	mMainWidget = new YahooUserInfoWidget(this);
	setMainWidget( mMainWidget );
	setEscapeButton( Cancel );
}

YahooUserInfoDialog::~YahooUserInfoDialog()
{
}

void YahooUserInfoDialog::slotClose()
{
	kdDebug(14180) << k_funcinfo << endl;
	QDialog::done(0);
}

void YahooUserInfoDialog::slotApply()
{
	kdDebug(14180) << k_funcinfo << endl;

	m_userInfo.firstName = mMainWidget->m_fname->text();
	m_userInfo.lastName = mMainWidget->m_lname->text();
	m_userInfo.nickName = mMainWidget->m_nname->text(),
	m_userInfo.email = mMainWidget->m_email->text();
	m_userInfo.phoneHome = mMainWidget->m_hphone->text();
	m_userInfo.phoneWork = mMainWidget->m_wphone->text();
	//m_userInfo.phoneMobile 	mMainWidget->= m_mphone->text();

	if( m_theSession )
		m_theSession->saveAdressBookEntry( m_userInfo );
	QDialog::done(0);
}

void YahooUserInfoDialog::slotUser1()
{
	kdDebug(14180) << k_funcinfo << endl;
	if( m_theSession )
		m_theSession->viewUserProfile( m_userInfo.userID );
}

void YahooUserInfoDialog::setSession( YahooSession *session)
{
	kdDebug(14180) << k_funcinfo << endl;
	m_theSession = session;
}

void YahooUserInfoDialog::setUserInfo( const YahooUserInfo &info)
{
	kdDebug(14180) << k_funcinfo << endl;
	m_userInfo.userID = info.userID;
	m_userInfo.abID = info.abID;
	m_userInfo.firstName = info.firstName;
	m_userInfo.lastName = info.lastName;
	m_userInfo.nickName	= info.nickName,
	m_userInfo.email = info.email;
	m_userInfo.phoneHome = info.phoneHome;
	m_userInfo.phoneWork = info.phoneWork;
	//m_userInfo.phoneMobile 	= info.phoneMobile;

	mMainWidget->m_userID->setText( m_userInfo.userID );
	mMainWidget->m_fname->setText( m_userInfo.firstName );
	mMainWidget->m_lname->setText( m_userInfo.lastName );
	mMainWidget->m_nname->setText( m_userInfo.nickName );
	mMainWidget->m_email->setText( m_userInfo.email );
	mMainWidget->m_hphone->setText( m_userInfo.phoneHome );
	mMainWidget->m_wphone->setText( m_userInfo.phoneWork );
	//m_mphone->setText( m_userInfo.phoneMobile );

}



#include "yahoouserinfo.moc"

