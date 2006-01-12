
/***************************************************************************
                          jabberaddcontactpage.cpp  -  Add contact widget
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
                           (C) 2003 by Daniel Stone <dstone@kde.org>
                           (C) 2006 by Olivier Goffart <ogoffart at kde.org>
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabberaddcontactpage.h"

#include <qlayout.h>
#include <klineedit.h>
#include <klocale.h>
#include <kopeteaccount.h>
#include <qlabel.h>
#include <kopetegroup.h>
#include <kopetemetacontact.h>

#include "dlgaddcontact.h"
#include "jabberaccount.h"
#include "jabbertransport.h"
#include "kopetecontact.h"
#include "jabberclient.h"
#include "xmpp_tasks.h"

JabberAddContactPage::JabberAddContactPage (Kopete::Account * owner, QWidget * parent, const char *name):AddContactPage (parent, name)
{
	(new QVBoxLayout (this))->setAutoAdd (true);
	
	JabberTransport *transport=dynamic_cast<JabberTransport*>(owner);
	JabberAccount *jaccount= transport ? transport->account() : dynamic_cast<JabberAccount*>(owner);
	
	if (jaccount->isConnected ())
	{
		jabData = new dlgAddContact (this);
		jabData->show ();
		
		if(transport)
		{
			jabData->textLabel1->setText( i18n("Loading instruction from gateway...") );
			XMPP::JT_Gateway * gatewayTask = new XMPP::JT_Gateway ( jaccount->client()->rootTask () );
			QObject::connect (gatewayTask, SIGNAL (finished ()), this, SLOT (slotPromtReceived()));
			gatewayTask->get ( transport->myself()->contactId() );
			gatewayTask->go ( true );
		}
		canadd = true;
	}
	else
	{
		noaddMsg1 = new QLabel (i18n ("You need to be connected to be able to add contacts."), this);
		noaddMsg2 = new QLabel (i18n ("Connect to the Jabber network and try again."), this);
		canadd = false;
	}

}

JabberAddContactPage::~JabberAddContactPage ()
{
}

bool JabberAddContactPage::validateData ()
{
	return true;
}


bool JabberAddContactPage::apply ( Kopete::Account *account, Kopete::MetaContact *parentContact )
{

	if( canadd && validateData () )
	{
		JabberTransport *transport=dynamic_cast<JabberTransport*>(account);
		JabberAccount *jaccount=transport?transport->account():dynamic_cast<JabberAccount*>(account);
				
		QString contactId = jabData->addID->text ();
		
		if(transport)
		{
			XMPP::JT_Gateway * gatewayTask = new XMPP::JT_Gateway ( jaccount->client()->rootTask () );
			JabberAddContactPage_there_is_no_possibility_to_add_assync_WORKAROUND *workaround = 
					new JabberAddContactPage_there_is_no_possibility_to_add_assync_WORKAROUND( transport , parentContact , gatewayTask );
			QObject::connect (gatewayTask, SIGNAL (finished ()), workaround, SLOT (slotJidReceived()));
			gatewayTask->set ( transport->myself()->contactId() , contactId );
			gatewayTask->go ( true );
			return true;
		}
		
		QString displayName = parentContact->displayName ();
		/*		
		if ( displayName.isEmpty () )
			displayName = contactId;
		*/
		// collect all group names
		QStringList groupNames;
		Kopete::GroupList groupList = parentContact->groups();
		for(Kopete::Group *group = groupList.first(); group; group = groupList.next())
			groupNames += group->displayName();

		if ( jaccount->addContact ( contactId, parentContact, Kopete::Account::ChangeKABC ) )
		{
			XMPP::RosterItem item;
			XMPP::Jid jid ( contactId );

			item.setJid ( jid );
			item.setName ( displayName );
			item.setGroups ( groupNames );

			// add the new contact to our roster.
			XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( jaccount->client()->rootTask () );

			rosterTask->set ( item.jid(), item.name(), item.groups() );
			rosterTask->go ( true );

			// send a subscription request.
			XMPP::JT_Presence *presenceTask = new XMPP::JT_Presence ( jaccount->client()->rootTask () );

			presenceTask->sub ( jid, "subscribe" );
			presenceTask->go ( true );

			return true;
		}
	}

	return false;
}

void JabberAddContactPage::slotPromtReceived( )
{
	XMPP::JT_Gateway * task = (XMPP::JT_Gateway *) sender ();

	if (task->success ())
	{
		jabData->lblID->setText( task->prompt() );
		jabData->textLabel1->setText( task->desc() );
	}
	else
	{
		jabData->textLabel1->setText( i18n("An error occured while loading instructions from gateway.") );
	}
}

JabberAddContactPage_there_is_no_possibility_to_add_assync_WORKAROUND::JabberAddContactPage_there_is_no_possibility_to_add_assync_WORKAROUND( JabberTransport *t, Kopete::MetaContact * mc, QObject* task )
	: QObject(task) , metacontact(mc) ,  transport(t)
{}

void JabberAddContactPage_there_is_no_possibility_to_add_assync_WORKAROUND::slotJidReceived( )
{
	XMPP::JT_Gateway * task = (XMPP::JT_Gateway *) sender ();
	
	if (!task->success ())
	{
		return;
		// maybe we should show an error message, but i don't like showing error message  - Olivier
	}

	QString contactId=task->prompt();
	
	Kopete::MetaContact* parentContact=metacontact;
	JabberAccount *jaccount=transport->account();;
	
	/*\
	 *   this is a copy of the end of JabberAddContactPage::apply
	\*/
	
	QString displayName = parentContact->displayName ();
		/*		
	if ( displayName.isEmpty () )
	displayName = contactId;
		*/
		// collect all group names
	QStringList groupNames;
	Kopete::GroupList groupList = parentContact->groups();
	for(Kopete::Group *group = groupList.first(); group; group = groupList.next())
		groupNames += group->displayName();

	if ( jaccount->addContact ( contactId, parentContact, Kopete::Account::ChangeKABC ) )
	{
		XMPP::RosterItem item;
		XMPP::Jid jid ( contactId );

		item.setJid ( jid );
		item.setName ( displayName );
		item.setGroups ( groupNames );

			// add the new contact to our roster.
		XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( jaccount->client()->rootTask () );

		rosterTask->set ( item.jid(), item.name(), item.groups() );
		rosterTask->go ( true );

			// send a subscription request.
		XMPP::JT_Presence *presenceTask = new XMPP::JT_Presence ( jaccount->client()->rootTask () );

		presenceTask->sub ( jid, "subscribe" );
		presenceTask->go ( true );

		return;
	}
}



#include "jabberaddcontactpage.moc"

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
