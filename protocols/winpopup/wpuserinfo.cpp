/***************************************************************************
                          wpuserinfo.cpp  -  WinPopup User Info
                             -------------------
    begin                : Tue May 06 2003
    copyright            : (C) 2003 by Tais M. Hansen
    email                : tais.hansen@osd.dk

    Based on code from   : (C) 2002-2003 by the Kopete developers
    email                : kopete-devel@kde.org
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// KDE Includes
#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>

// Local Includes
#include "wpuserinfo.h"
#include "wpaccount.h"
#include "wpcontact.h"

WPUserInfo::WPUserInfo( WPContact *contact, WPAccount *account, QWidget *parent, const char* name )
	: KDialogBase( parent, name, false, QString::null, Close, Close, false )
{
	kdDebug( 14180 ) << k_funcinfo << endl;

	m_contact = contact;
	//m_account = account;

	QStringList theHostDetails = account->getHostDetails( m_contact->displayName() );
	kdDebug( 14180 ) << k_funcinfo << ">>> " << theHostDetails.join( ", " ) << endl;

	setCaption( i18n( "User Info for %1" ).arg( m_contact->displayName() ) );

	m_mainWidget = new WPUserInfoWidget( this, "WPUserInfo::m_mainWidget" );
	setMainWidget( m_mainWidget );

	m_mainWidget->sComputerName->setText( m_contact->displayName() );

	QStringList::Iterator it = theHostDetails.begin();
	m_mainWidget->sWorkgroup->setText( (*it).isEmpty() ? "N/A" : *it );
	m_mainWidget->sOS->setText( (*++it).isEmpty() ? "N/A" : *it );
	m_mainWidget->sServer->setText( (*++it).isEmpty() ? "N/A" : *it );

	connect( this, SIGNAL( closeClicked() ), this, SLOT( slotCloseClicked() ) );
}

void WPUserInfo::slotCloseClicked()
{
	kdDebug( 14180 ) << k_funcinfo << endl;

	emit closing();
}

#include "wpuserinfo.moc"
