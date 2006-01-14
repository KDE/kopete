/*
    Kopete Groupwise Protocol
    gwreceiveinvitationdialog.cpp - dialog shown when the user receives an invitation to chat

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
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
#include <qlabel.h>

#include <kconfig.h>
#include <klocale.h>
#include <kopetecontact.h>
#include <kopeteglobal.h>
#include <kopetemetacontact.h>
#include "client.h"
#include "gwaccount.h"
#include "gwcontact.h"
#include "gwerror.h"
#include "gwprotocol.h"
#include "gwshowinvitation.h"

#include "gwreceiveinvitationdialog.h"

ReceiveInvitationDialog::ReceiveInvitationDialog( GroupWiseAccount * account, const ConferenceEvent & event, QWidget *parent, const char *name)
 : KDialogBase( i18n("Invitation to Conversation"), KDialogBase::Yes|KDialogBase::No, KDialogBase::Yes, KDialogBase::No, parent, name, false )
{
	m_account = account;
	m_guid = event.guid;	
	connect( this, SIGNAL( yesClicked() ), SLOT( slotYesClicked() ) );
	connect( this, SIGNAL( noClicked() ), SLOT( slotNoClicked() ) );
	
	GroupWiseContact * c = account->contactForDN( event.user );
	
	m_wid = new ShowInvitationWidget ( this );
	if ( c )
		m_wid->m_contactName->setText( c->metaContact()->displayName() );
	else //something is very wrong
		m_wid->m_contactName->setText( event.user );
		
	m_wid->m_dateTime->setText( KGlobal::locale()->formatDateTime( event.timeStamp ) );
	m_wid->m_message->setText( QString("<b>%1</b>").arg( event.message ) );
	
	setMainWidget( m_wid );
}

ReceiveInvitationDialog::~ReceiveInvitationDialog()
{
}

void ReceiveInvitationDialog::slotYesClicked()
{
	m_account->client()->joinConference( m_guid );
	// save the state of always accept invitations
	QString alwaysAccept = m_wid->cb_dontShowAgain->isChecked() ? "true" : "false";
	m_account->configGroup()->writeEntry( "AlwaysAcceptInvitations", alwaysAccept );
	deleteLater();
}

void ReceiveInvitationDialog::slotNoClicked()
{
	m_account->client()->rejectInvitation( m_guid );
	deleteLater();
}

#include "gwreceiveinvitationdialog.moc"
