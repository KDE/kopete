
/***************************************************************************
                          dlgjabberservices.cpp  -  Service browsing
                             -------------------
    begin                : Mon Dec 9 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
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

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qtable.h>

#include "jabberaccount.h"
#include "jabberclient.h"
#include "dlgjabberregister.h"
#include "dlgjabberbrowse.h"
#include "dlgjabberservices.h"

#include "dlgjabberservices.moc"

dlgJabberServices::dlgJabberServices (JabberAccount *account, QWidget *parent, const char *name):dlgServices (parent, name)
{
	m_account = account;

	if(m_account->isConnected())
	{
		// pre-populate the server field
		leServer->setText(m_account->server());
	}

	// disable the left margin
	//tblServices->setLeftMargin (0);

	// no content for now
	//tblServices->setNumRows (0);

	// disable the buttons as long as nothing has been selected
	btnRegister->setDisabled (true);
	btnBrowse->setDisabled (true);

	// allow autostretching
	//tblServices->setColumnStretchable (0, true);
	//tblServices->setColumnStretchable (1, true);

	// disable user selections
	//tblServices->setSelectionMode (QTable::NoSelection);

	// name table headers
	//tblServices->horizontalHeader ()->setLabel (0, i18n ("Name"));
	//tblServices->horizontalHeader ()->setLabel (1, i18n ("Address"));

	connect (btnQuery, SIGNAL (clicked ()), this, SLOT (slotDisco ()));
	//connect (tblServices, SIGNAL (clicked (int, int, int, const QPoint &)), this, SLOT (slotSetSelection (int, int, int, const QPoint &)));
	connect (lvServices, SIGNAL (selectionChanged (QListViewItem *)), this, SLOT (slotSetSelection (QListViewItem *)));

	connect (btnRegister, SIGNAL (clicked ()), this, SLOT (slotRegister ()));
	connect (btnBrowse, SIGNAL (clicked ()), this, SLOT (slotBrowse ()));

}

void dlgJabberServices::slotSetSelection (QListViewItem *it)
{
	dlgJabberServies_item *item=dynamic_cast<dlgJabberServies_item*>(it);
	if(!item)
	{
		btnRegister->setDisabled (true);
		btnBrowse->setDisabled (true);
	}
	else
	{
		btnRegister->setDisabled (! item->can_register);
		btnBrowse->setDisabled (! item->can_browse);
		current_jid=item->jid;
	}

}

void dlgJabberServices::slotService ()
{

	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}
	
	XMPP::JT_GetServices *serviceTask = new XMPP::JT_GetServices (m_account->client()->rootTask ());
	connect (serviceTask, SIGNAL (finished ()), this, SLOT (slotServiceFinished ()));

	/* populate server field if it is empty */
	if(leServer->text().isEmpty())
		leServer->setText(m_account->server());

	kdDebug (14130) << "[dlgJabberServices] Trying to fetch a list of services at " << leServer->text () << endl;

	serviceTask->get (leServer->text ());
	serviceTask->go (true);
}



void dlgJabberServices::slotServiceFinished ()
{
	kdDebug (14130) << "[dlgJabberServices] Query task finished" << endl;

	XMPP::JT_GetServices * task = (XMPP::JT_GetServices *) sender ();

	if (!task->success ())
	{
		QString error = task->statusString();
		KMessageBox::queuedMessageBox (this, KMessageBox::Error, i18n ("Unable to retrieve the list of services.\nReason: %1").arg(error), i18n ("Jabber Error"));
		return;
	}

	lvServices->clear();

	for (XMPP::AgentList::const_iterator it = task->agents ().begin (); it != task->agents ().end (); ++it)
	{
		dlgJabberServies_item *item=new dlgJabberServies_item( lvServices , (*it).jid ().userHost () , (*it).name ());
		item->jid=(*it).jid();
		item->can_browse=(*it).features().canSearch();
		item->can_register=(*it).features().canRegister();
	}
}

void dlgJabberServices::slotDisco()
{
	lvServices->clear();

	if(!m_account->isConnected())
	{
		m_account->errorConnectFirst();
		return;
	}
	
	JT_DiscoItems *jt = new JT_DiscoItems(m_account->client()->rootTask());
	connect(jt, SIGNAL(finished()), this, SLOT(slotDiscoFinished()));
	
	/* populate server field if it is empty */
	if(leServer->text().isEmpty())
		leServer->setText(m_account->server());
	
	jt->get(leServer->text() , QString());
	jt->go(true);
}





void dlgJabberServices::slotDiscoFinished( )
{
	XMPP::JT_DiscoItems *jt = (JT_DiscoItems *)sender();

	if ( jt->success() ) 
	{
		QValueList<XMPP::DiscoItem> list = jt->items();
		
		lvServices->clear();

		for(QValueList<XMPP::DiscoItem>::ConstIterator it = list.begin(); it != list.end(); ++it) 
		{
			const XMPP::DiscoItem a = *it;
			dlgJabberServies_item *item=new dlgJabberServies_item( lvServices , (*it).jid ().userHost () , (*it).name ());
			item->jid=a.jid();
			item->updateInfo(a.jid() , a.node(), m_account);
		}
	}
	else
	{
		slotService();
	}
}


void dlgJabberServices::slotRegister ()
{

	dlgJabberRegister *registerDialog = new dlgJabberRegister (m_account, current_jid);

	registerDialog->show ();
	registerDialog->raise ();

}

void dlgJabberServices::slotBrowse ()
{

	dlgJabberBrowse *browseDialog = new dlgJabberBrowse (m_account, current_jid);

	browseDialog->show ();
	browseDialog->raise ();

}

dlgJabberServices::~dlgJabberServices ()
{
}

void dlgJabberServies_item::updateInfo( const XMPP::Jid & jid , const QString & node , JabberAccount *account )
{
	XMPP::JT_DiscoInfo *jt = new XMPP::JT_DiscoInfo(account->client()->rootTask());
	connect(jt, SIGNAL(finished()),this, SLOT(slotDiscoFinished()));
	jt->get(jid, node);
	jt->go(true);

}

void dlgJabberServies_item::slotDiscoFinished( )
{
	JT_DiscoInfo *jt = (JT_DiscoInfo *)sender();

	if ( jt->success() ) 
	{
		can_browse = jt->item().features().canSearch();
		can_register = jt->item().features().canRegister();
	}
	else
	{
		//TODO: error message  (it's a simple message box to show)
	}
}

