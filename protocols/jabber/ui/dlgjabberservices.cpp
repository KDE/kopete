
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

#include "dlgjabberservices.h"

#include <QPushButton>
#include <QLineEdit>
#include <QTreeWidget>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <KMenu>
#include <KMessageBox>
#include <KLocale>
#include <KDebug>

#include "jabberaccount.h"
#include "jabberclient.h"
#include "tasks/jt_ahcommand.h"
#include "dlgregister.h"
#include "dlgsearch.h"
#include "dlgahclist.h"

dlgJabberServices::dlgJabberServices(JabberAccount *account, QWidget *parent):
KDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	QWidget *widget = new QWidget(this);
	ui.setupUi(widget);
	setMainWidget(widget);
	setButtons(Close);
	setCaption(i18n("Services"));

	mAccount = account;
	if(mAccount->isConnected())
	{
		// pre-populate the server field
		ui.leServer->setText(mAccount->server());
	}

	ui.trServices->header()->setResizeMode(QHeaderView::Stretch);
	ui.trServices->installEventFilter(this);
	connect(ui.btnQuery, SIGNAL(clicked()), this, SLOT(slotDisco()));

	connect(ui.trServices, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(slotItemExpanded(QTreeWidgetItem*)));
	connect(ui.trServices, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotMenuRequested(QPoint)));

	mActRegister = new QAction(i18n("Register..."), this);
	connect(mActRegister, SIGNAL(triggered()), this, SLOT(slotRegister()));
	mActSearch = new QAction(i18n("Search..."), this);
	connect(mActSearch, SIGNAL(triggered()), this, SLOT(slotSearch()));
	mActCommand = new QAction(i18n("Execute..."), this);
	connect(mActCommand, SIGNAL(triggered()), this, SLOT(slotCommand()));
}

dlgJabberServices::~dlgJabberServices()
{
}

void dlgJabberServices::initTree()
{
	ui.trServices->clear();
	mRootItem = new ServiceItem(mAccount, ui.leServer->text() , ui.leNode->text(), "");
	ui.trServices->addTopLevelItem(mRootItem);
	mRootItem->setExpanded(true);
}

bool dlgJabberServices::eventFilter(QObject *object, QEvent *event)
{
	if(object == ui.trServices)
	{
		if(event->type() == QEvent::ContextMenu && ui.trServices->currentItem())
		{
			QContextMenuEvent *e = (QContextMenuEvent *)event;
			ServiceItem *si = (ServiceItem *)ui.trServices->currentItem();
			KMenu *menu = new KMenu(this);
			if(si->features().canRegister())
				menu->addAction(mActRegister);
			if(si->features().canSearch())
				menu->addAction(mActSearch);
			if(si->features().canCommand())
				menu->addAction(mActCommand);
			menu->popup(e->globalPos());
			return true;
		}
	}
	return false;
}
/*void dlgJabberServices::slotSetSelection (Q3ListViewItem *it)
{
	dlgJabberServies_item *item=dynamic_cast<dlgJabberServies_item*>(it);
	if(!item)
	{
		btnRegister->setDisabled (true);
		btnBrowse->setDisabled (true);
		current_node.clear();
	}
	else
	{
		btnRegister->setDisabled (! item->can_register);
		btnBrowse->setDisabled (! item->can_browse);
		current_jid=item->jid;
		current_node=item->node;
	}

}*/

void dlgJabberServices::slotItemExpanded(QTreeWidgetItem *item)
{
	ServiceItem *si = (ServiceItem *)item;
	si->startDisco();
}

void dlgJabberServices::slotService()
{
/*	if(!mAccount->isConnected())
	{
		mAccount->errorConnectFirst();
		return;
	}

	XMPP::JT_GetServices *serviceTask = new XMPP::JT_GetServices (mAccount->client()->rootTask ());
	connect (serviceTask, SIGNAL (finished()), this, SLOT (slotServiceFinished()));

	// populate server field if it is empty
	if(leServer->text().isEmpty())
		leServer->setText(mAccount->server());

	kDebug (14130) << "[dlgJabberServices] Trying to fetch a list of services at " << leServer->text ();

	serviceTask->get (leServer->text ());
	serviceTask->go (true);*/
}



void dlgJabberServices::slotServiceFinished ()
{
/*	kDebug (14130) << "[dlgJabberServices] Query task finished";

	XMPP::JT_GetServices * task = (XMPP::JT_GetServices *) sender ();

	if (!task->success ())
	{
		QString error = task->statusString();
		KMessageBox::queuedMessageBox (this, KMessageBox::Error, i18n ("Unable to retrieve the list of services.\nReason: %1", error), i18n ("Jabber Error"));
		return;
	}

	lvServices->clear();

	for (XMPP::AgentList::const_iterator it = task->agents ().begin (); it != task->agents ().end (); ++it)
	{
		dlgJabberServies_item *item=new dlgJabberServies_item( lvServices , (*it).jid ().userHost () , QString(),  (*it).name ());
		item->jid=(*it).jid();
		item->can_browse=(*it).features().canSearch();
		item->can_register=(*it).features().canRegister();
		kDebug (14130) << "*******  " << (*it).features().canCommand();
	}*/
}

void dlgJabberServices::slotDisco()
{
	initTree();
	mRootItem->startDisco();
/*	trServices->clear();

	if(!mAccount->isConnected())
	{
		mAccount->errorConnectFirst();
		return;
	}

	JT_DiscoItems *jt = new JT_DiscoItems(mAccount->client()->rootTask());
	connect(jt, SIGNAL(finished()), this, SLOT(slotDiscoFinished()));

	// populate server field if it is empty
	if(leServer->text().isEmpty())
		leServer->setText(mAccount->server());

	jt->get(leServer->text() , leNode->text());
	jt->go(true);*/
}





void dlgJabberServices::slotDiscoFinished( )
{
/*	XMPP::JT_DiscoItems *jt = (JT_DiscoItems *)sender();

	if( jt->success() ) 
	{
		const DiscoList &list = jt->items();
		//lvServices->clear();

		for(DiscoList::ConstIterator it = list.begin(); it != list.end(); it++)
		{
			const DiscoItem a = *it;
			//dlgJabberServies_item *item=new dlgJabberServies_item( lvServices ,a.jid().full() , a.node() , a.name());
			kDebug() << a.jid().full() << " " << a.node() << " " << a.name();
			ServiceItem *item = new ServiceItem(a.jid().full(), a.node(), a.name());
			trServices->addTopLevelItem(item);
		}
	}
	else
	{
		//slotService();
	}*/
}


void dlgJabberServices::slotRegister()
{
	ServiceItem *si = (ServiceItem *)ui.trServices->currentItem();
	dlgRegister *w = new dlgRegister(mAccount, si->jid());
	w->show();
	w->raise();
}

void dlgJabberServices::slotSearch()
{
	ServiceItem *si = (ServiceItem *)ui.trServices->currentItem();
	dlgSearch *w = new dlgSearch(mAccount, si->jid());
	w->show();
	w->raise();
}

void dlgJabberServices::slotCommand()
{
	ServiceItem *si = (ServiceItem *)ui.trServices->currentItem();
	if(si->node().isEmpty())
	{
		dlgAHCList *w = new dlgAHCList(si->jid(), mAccount->client()->client());
		w->show();
	}
	else
	{
		JT_AHCommand *jt = new JT_AHCommand(si->jid(), AHCommand(si->node()), mAccount->client()->rootTask());
		jt->go(true);
	}
}

ServiceItem::ServiceItem(JabberAccount *account, const QString &jid , const QString &node, const QString &name):
QTreeWidgetItem(0)
{
	mAccount = account;
	mDiscoReady = false;
	mJid = jid;
	mNode = node;
	setChildIndicatorPolicy(ShowIndicator);
	if(name.isEmpty())
		setText(0, jid);
	else
		setText(0, name);
	setText(1, jid);
	setText(2, node);
	XMPP::JT_DiscoInfo *jt = new XMPP::JT_DiscoInfo(mAccount->client()->rootTask());
	connect(jt, SIGNAL(finished()), this, SLOT(slotDiscoInfoFinished()));
	jt->get(mJid, mNode);
	jt->go(true);
}

void ServiceItem::startDisco()
{
	if(mDiscoReady)
		return;
	mDiscoReady = true;
	JT_DiscoItems *jt = new JT_DiscoItems(mAccount->client()->rootTask());
	connect(jt, SIGNAL(finished()), this, SLOT(slotDiscoFinished()));
	jt->get(mJid , mNode);
	jt->go(true);
}

void ServiceItem::slotDiscoFinished()
{
	XMPP::JT_DiscoItems *jt = (JT_DiscoItems *)sender();

	if(jt->success())
	{
		const DiscoList &list = jt->items();
		for(DiscoList::ConstIterator it = list.begin(); it != list.end(); ++it)
		{
			const DiscoItem a = *it;
			//kDebug() << a.jid().full() << " " << a.node() << " " << a.name();
			ServiceItem *item = new ServiceItem(mAccount, a.jid().full(), a.node(), a.name());
			addChild(item);
		}
	}
	else
	{
		//slotService();
	}
}

void ServiceItem::slotDiscoInfoFinished()
{
	JT_DiscoInfo *jt = (JT_DiscoInfo *)sender();
	if(jt->success())
	{
		mFeatures = jt->item().features();
	}
	else
	{
		//TODO: error message  (it's a simple message box to show)
	}
}

#include "dlgjabberservices.moc"
