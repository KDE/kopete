
/***************************************************************************
                          jabberaddcontactpage.cpp  -  Add contact widget
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2003 by Till Gerken <till@tantalo.net>
                           (C) 2003 by Daniel Stone <dstone@kde.org>
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
#include "xmpp_tasks.h"

JabberAddContactPage::JabberAddContactPage (Kopete::Account * owner, QWidget * parent, const char *name):AddContactPage (parent, name)
{
	(new QVBoxLayout (this))->setAutoAdd (true);
	if (owner->isConnected ())
	{
		jabData = new dlgAddContact (this);
		jabData->show ();

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
		QString contactId = jabData->addID->text ();
		QString displayName = parentContact->displayName ();
		
		if ( displayName.isEmpty () )
			displayName = contactId;

		// collect all group names
		QStringList groupNames;
		Kopete::GroupList groupList = parentContact->groups();
		for(Kopete::Group *group = groupList.first(); group; group = groupList.next())
			groupNames += group->displayName();

		if ( account->addContact ( contactId, displayName, parentContact, Kopete::Account::ChangeKABC ) )
		{
			XMPP::RosterItem item;
			XMPP::Jid jid ( contactId );

			item.setJid ( jid );
			item.setName ( displayName );
			item.setGroups ( groupNames );

			// add the new contact to our roster.
			XMPP::JT_Roster * rosterTask = new XMPP::JT_Roster ( static_cast<JabberAccount *>(account)->client()->rootTask () );

			rosterTask->set ( item.jid(), item.name(), item.groups() );
			rosterTask->go ( true );

			// send a subscription request.
			XMPP::JT_Presence *presenceTask = new XMPP::JT_Presence ( static_cast<JabberAccount *>(account)->client()->rootTask () );

			presenceTask->sub ( jid, "subscribe" );
			presenceTask->go ( true );

			return true;
		}
	}

	return false;
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
