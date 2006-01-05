 /*

    Copyright (c) 2006      by Olivier Goffart  <ogoffart at kde.org>

    Kopete    (c) 2006 by the Kopete developers <kopete-devel@kde.org>


    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#include "im.h"
#include "filetransfer.h"
#include "xmpp.h"
#include "xmpp_tasks.h"
#include "qca.h"
#include "bsocket.h"

#include "jabbertransport.h"

#include <time.h>

#include <qstring.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <ksocketbase.h>

#include "kopetepassword.h"
#include "kopeteawayaction.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include "kopetegroup.h"
#include "kopetecontactlist.h"

#include "jabberconnector.h"
#include "jabberclient.h"
#include "jabberprotocol.h"
#include "jabberresourcepool.h"
#include "jabbercontactpool.h"
#include "jabberfiletransfer.h"
#include "jabbercontact.h"
#include "jabbergroupcontact.h"
#include "jabbercapabilitiesmanager.h"
#include "dlgjabbersendraw.h"
#include "dlgjabberservices.h"
#include "dlgjabberchatjoin.h"

#include <sys/utsname.h>

#ifdef SUPPORT_JINGLE
#include "jingle/voicecaller.h"
#include "jingle/jinglevoicecaller.h"
#include "jingle/voicecalldlg.h"
#endif


JabberTransport::JabberTransport (JabberAccount * parentAccount, const QString & myselfContactId, const char *name)
	:Kopete::Account ( parentAccount->protocol(), myselfContactId+"/"+parentAccount->accountId(), name )
{
	m_account = parentAccount;
	m_account->addTransport( this, myselfContactId );
	
	JabberContact *myContact = m_account->contactPool()->addContact ( XMPP::RosterItem ( myselfContactId ), Kopete::ContactList::self()->myself(), false );
	setMyself( myContact );
}

JabberTransport::~JabberTransport ()
{
	m_account->removeTransport( myself()->contactId() );
}

KActionMenu *JabberTransport::actionMenu ()
{
	KActionMenu *menu = new KActionMenu( accountId(), myself()->onlineStatus().iconFor( this ),  this );
	QString nick = myself()->property( Kopete::Global::Properties::self()->nickName()).value().toString();

	menu->popupMenu()->insertTitle( myself()->onlineStatus().iconFor( myself() ),
	nick.isNull() ? accountLabel() : i18n( "%2 <%1>" ).arg( accountLabel(), nick )
								  );
	
	QPtrList<KAction> *customActions = myself()->customContextMenuActions(  );
	if( customActions && !customActions->isEmpty() )
	{
		menu->popupMenu()->insertSeparator();

		for( KAction *a = customActions->first(); a; a = customActions->next() )
			a->plug( menu->popupMenu() );
	}
	delete customActions;

	return menu;

/*	KActionMenu *m_actionMenu = Kopete::Account::actionMenu();

	m_actionMenu->popupMenu()->insertSeparator();

	m_actionMenu->insert(new KAction (i18n ("Join Groupchat..."), "jabber_group", 0,
		this, SLOT (slotJoinNewChat ()), this, "actionJoinChat"));

	m_actionMenu->popupMenu()->insertSeparator();

	m_actionMenu->insert ( new KAction ( i18n ("Services..."), "jabber_serv_on", 0,
										 this, SLOT ( slotGetServices () ), this, "actionJabberServices") );

	m_actionMenu->insert ( new KAction ( i18n ("Send Raw Packet to Server..."), "mail_new", 0,
										 this, SLOT ( slotSendRaw () ), this, "actionJabberSendRaw") );

	m_actionMenu->insert ( new KAction ( i18n ("Edit User Info..."), "identity", 0,
										 this, SLOT ( slotEditVCard () ), this, "actionEditVCard") );

	return m_actionMenu;*/
}


bool JabberTransport::createContact (const QString & contactId,  Kopete::MetaContact * metaContact)
{
#if 0 //TODO
	// collect all group names
	QStringList groupNames;
	Kopete::GroupList groupList = metaContact->groups();
	for(Kopete::Group *group = groupList.first(); group; group = groupList.next())
		groupNames += group->displayName();

	XMPP::Jid jid ( contactId );
	XMPP::RosterItem item ( jid );
	item.setName ( metaContact->displayName () );
	item.setGroups ( groupNames );

	// this contact will be created with the "dirty" flag set
	// (it will get reset if the contact appears in the roster during connect)
	JabberContact *contact = contactPool()->addContact ( item, metaContact, true );

	return ( contact != 0 );
#endif
	return false;
}


void JabberTransport::setOnlineStatus( const Kopete::OnlineStatus& status  , const QString &reason)
{
#if 0
	if( status.status() == Kopete::OnlineStatus::Offline )
	{
		disconnect( Kopete::Account::Manual );
		return;
	}

	if( isConnecting () )
	{
		errorConnectionInProgress ();
		return;
	}

	XMPP::Status xmppStatus ( "", reason );

	switch ( status.internalStatus () )
	{
		case JabberProtocol::JabberFreeForChat:
			xmppStatus.setShow ( "chat" );
			break;

		case JabberProtocol::JabberOnline:
			xmppStatus.setShow ( "" );
			break;

		case JabberProtocol::JabberAway:
			xmppStatus.setShow ( "away" );
			break;

		case JabberProtocol::JabberXA:
			xmppStatus.setShow ( "xa" );
			break;

		case JabberProtocol::JabberDND:
			xmppStatus.setShow ( "dnd" );
			break;

		case JabberProtocol::JabberInvisible:
			xmppStatus.setIsInvisible ( true );
			break;
	}

	if ( !isConnected () )
	{
		// we are not connected yet, so connect now
		m_initialPresence = xmppStatus;
		connect ();
	}
	else
	{
		setPresence ( xmppStatus );
	}
#endif
}

JabberProtocol * JabberTransport::protocol( ) const
{
	return m_account->protocol(); 
}



#include "jabbertransport.moc"

// vim: set noet ts=4 sts=4 sw=4:
