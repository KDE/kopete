//
// C++ Implementation: gwreceiveinvitationdialog
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qlabel.h>

#include <klocale.h>
#include <qlabel.h>
#include <kopetecontact.h>
#include <kopeteglobal.h>

#include "client.h"
#include "gwaccount.h"
#include "gwcontact.h"
#include "gwerror.h"
#include "gwshowinvitation.h"

#include "gwreceiveinvitationdialog.h"

ReceiveInvitationDialog::ReceiveInvitationDialog( GroupWiseAccount * account, const ConferenceEvent & event, QWidget *parent, const char *name)
 : KDialogBase( i18n("Invitation to Conversation"), KDialogBase::Yes|KDialogBase::No, KDialogBase::Yes, KDialogBase::No, parent, name, false )
{
	m_account = account;
	m_guid = event.guid;	
	connect( this, SIGNAL( yesClicked() ), SLOT( slotYesClicked() ) );
	connect( this, SIGNAL( noClicked() ), SLOT( slotNoClicked() ) );
	
	GroupWiseContact * c = static_cast<GroupWiseContact *>( account->contacts()[ event.user ] );
	
	ShowInvitationWidget * wid = new ShowInvitationWidget ( this );
	if ( c )
		wid->m_contactName->setText( c->property( Kopete::Global::Properties::self()->nickName() ).value().toString() );
	else //something is very wrong
		wid->m_contactName->setText( event.user );
		
	wid->m_dateTime->setText( KGlobal::locale()->formatDateTime( event.timeStamp ) );
	wid->m_message->setText( event.message );
	setMainWidget( wid );
}

ReceiveInvitationDialog::~ReceiveInvitationDialog()
{
}

void ReceiveInvitationDialog::slotYesClicked()
{
	m_account->client()->joinConference( m_guid );
	deleteLater();
}

void ReceiveInvitationDialog::slotNoClicked()
{
	m_account->client()->rejectInvitation( m_guid );
	deleteLater();
}

#include "gwreceiveinvitationdialog.moc"
