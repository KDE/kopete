/*
    Kopete Groupwise Protocol
    gwreceiveinvitationdialog.cpp - dialog shown when the user receives an invitation to chat

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
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

#include "gwreceiveinvitationdialog.h"
#include <qcheckbox.h>
#include <qlabel.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kopetecontact.h>
#include <kopeteglobal.h>
#include <kopetemetacontact.h>
#include "client.h"
#include "gwaccount.h"
#include "gwcontact.h"
#include "gwerror.h"
#include "gwprotocol.h"

ReceiveInvitationDialog::ReceiveInvitationDialog( GroupWiseAccount * account, const ConferenceEvent & event, QWidget *parent, const char * /*name*/)
 : KDialog(  parent )
{
	setCaption(i18n("Invitation to Conversation"));
	setButtons(KDialog::Yes|KDialog::No);
	setDefaultButton(KDialog::No);
	setModal(false);
	m_account = account;
	m_guid = event.guid;	
	connect( this, SIGNAL(yesClicked()), SLOT(slotYesClicked()) );
	connect( this, SIGNAL(noClicked()), SLOT(slotNoClicked()) );
	
	GroupWiseContact * c = account->contactForDN( event.user );

	QWidget * wid = new QWidget( this );
	m_ui.setupUi( wid );
	if ( c )
		m_ui.contactName->setText( c->metaContact()->displayName() );
	else //something is very wrong
		m_ui.contactName->setText( event.user );
		
	m_ui.dateTime->setText( KGlobal::locale()->formatDateTime( event.timeStamp ) );
	m_ui.message->setText( QString("<b>%1</b>").arg( event.message ) );
	
	setMainWidget( wid );
}

ReceiveInvitationDialog::~ReceiveInvitationDialog()
{
}

void ReceiveInvitationDialog::slotYesClicked()
{
	m_account->client()->joinConference( m_guid );
	// save the state of always accept invitations
	QString alwaysAccept = m_ui.cbDontShowAgain->isChecked() ? "true" : "false";
	m_account->configGroup()->writeEntry( "AlwaysAcceptInvitations", alwaysAccept );
	deleteLater();
}

void ReceiveInvitationDialog::slotNoClicked()
{
	m_account->client()->rejectInvitation( m_guid );
	deleteLater();
}

#include "gwreceiveinvitationdialog.moc"
