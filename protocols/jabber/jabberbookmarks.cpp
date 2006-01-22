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

#include "jabberbookmarks.h"
#include "jabberaccount.h"

#include <kopetecontact.h>


#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>

#include "xmpp_tasks.h"


JabberBookmarks::JabberBookmarks(JabberAccount *parent) : QObject(parent) , m_account(parent) 
{
	connect( m_account , SIGNAL( isConnectedChanged() ) , this , SLOT( accountConnected() ) );
}

void JabberBookmarks::accountConnected()
{
	if(!m_account->isConnected())
		return;
	
	XMPP::JT_PrivateStorage * task = new XMPP::JT_PrivateStorage ( m_account->client()->rootTask ());
	task->get( "storage" , "storage:bookmarks" );
	QObject::connect ( task, SIGNAL ( finished () ), this, SLOT ( slotReceivedBookmarks() ) );
	task->go ( true );
}

void JabberBookmarks::slotReceivedBookmarks( )
{
	XMPP::JT_PrivateStorage * task = (XMPP::JT_PrivateStorage*)(sender());
	m_storage=QDomDocument("storage");
	m_conferencesJID.clear();
	if(task->success())
	{
		QDomElement storage_e=task->element();
		if(!storage_e.isNull() && storage_e.tagName() == "storage")
		{
			storage_e=m_storage.importNode(storage_e,true).toElement();
			m_storage.appendChild(storage_e);

			for(QDomNode n = storage_e.firstChild(); !n.isNull(); n = n.nextSibling()) 
			{
				QDomElement i = n.toElement();
				if(i.isNull())
					continue;
				if(i.tagName() == "conference")
				{
					QString jid=i.attribute("jid");
					QString password;
					for(QDomNode n = i.firstChild(); !n.isNull(); n = n.nextSibling()) {
						QDomElement e = n.toElement();
						if(e.isNull())
							continue;
						else if(e.tagName() == "nick")
							jid+="/"+e.text();
						else if(e.tagName() == "password")
							password=e.text();
						
					}
					m_conferencesJID += jid;
					if(i.attribute("autojoin") == "true")
					{
						XMPP::Jid x_jid(jid);
						QString nick=x_jid.resource();
						if(nick.isEmpty())
							nick=m_account->myself()->nickName();

						if(password.isEmpty())
							m_account->client()->joinGroupChat(x_jid.host() , x_jid.user() , nick );
						else
							m_account->client()->joinGroupChat(x_jid.host() , x_jid.user() , nick , password);
					}
				}
			}
		}
	}
}


void JabberBookmarks::insertGroupChat(const XMPP::Jid &jid)
{
	if(m_conferencesJID.contains(jid.full()) || !m_account->isConnected())
	{
		return;
	}

	QDomElement storage_e=m_storage.documentElement();
	if(storage_e.isNull())
	{
		storage_e=m_storage.createElement("storage");
		m_storage.appendChild(storage_e);
		storage_e.setAttribute("xmlns","storage:bookmarks");
	}
	
	QDomElement conference=m_storage.createElement("conference");
	storage_e.appendChild(conference);
	conference.setAttribute("jid",jid.userHost());
	QDomElement nick=m_storage.createElement("nick");
	conference.appendChild(nick);
	nick.appendChild(m_storage.createTextNode(jid.resource()));
	QDomElement name=m_storage.createElement("name");
	conference.appendChild(name);
	name.appendChild(m_storage.createTextNode(jid.full()));

		
	XMPP::JT_PrivateStorage * task = new XMPP::JT_PrivateStorage ( m_account->client()->rootTask ());
	task->set( storage_e );
	task->go ( true );
	
	m_conferencesJID += jid.full();
}

KAction * JabberBookmarks::bookmarksAction(QObject *parent)
{
	KSelectAction *groupchatBM = new KSelectAction( i18n("Groupchat bookmark") , "jabber_group" , 0 , parent , "actionBookMark" );
	groupchatBM->setItems(m_conferencesJID);
	QObject::connect(groupchatBM, SIGNAL(activated (const QString&)) , this, SLOT(slotJoinChatBookmark(const QString&)));
	return groupchatBM;
}

void JabberBookmarks::slotJoinChatBookmark( const QString & _jid )
{
	if(!m_account->isConnected())
		return;
	XMPP::Jid jid(_jid);
	m_account->client()->joinGroupChat( jid.host() , jid.user() , jid.resource() );
}



#include "jabberbookmarks.moc"

