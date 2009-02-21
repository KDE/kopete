
/***************************************************************************
                          dlgjabberbrowse.cpp  -  description
                             -------------------
    begin                : Wed Dec 11 2002
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

#include "dlgsearch.h"

#include <QHeaderView>
#include <KMessageBox>
#include <KLocale>

#include "xmpp_xmlcommon.h"
#include "xmpp_xdata.h"
#include "jabberaccount.h"
#include "jabberprotocol.h"
#include "jabberclient.h"
#include "jabberformtranslator.h"
#include "jabberxdatawidget.h"
#include "jt_xsearch.h"
#include <kdebug.h>

dlgSearch::dlgSearch(JabberAccount *account, const XMPP::Jid &jid, QWidget *parent):
KDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	QWidget *widget = new QWidget(this);
	ui.setupUi(widget);
	setMainWidget(widget);
	setButtons(Close | User1);
	setButtonText(User1, i18n("Search"));
	setCaption(i18n("Search"));

	mAccount = account;
	mXDataWidget = 0L;

	ui.tblResults->header()->setResizeMode(QHeaderView::ResizeToContents);
	ui.lblWait->setText(i18n("Please wait while retrieving search form..."));
	enableButton(User1, false);
	connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSendForm()));
	// get form
	JT_XSearch * task = new JT_XSearch(mAccount->client()->rootTask());
	connect(task, SIGNAL(finished()), this, SLOT(slotGotForm()));
	task->get(jid);
	task->go(true);
}

dlgSearch::~dlgSearch()
{
}

void dlgSearch::slotGotForm()
{
	JT_XSearch *task = (JT_XSearch *)sender();

	// delete the wait message
	delete ui.lblWait;

	if(!task->success())
	{
		KMessageBox::queuedMessageBox(this, KMessageBox::Information, i18n("Unable to retrieve search form."), i18n ("Jabber Error"));
		return;
	}

	mForm = task->form();
	bool xdataform = false;
	QDomNode n = queryTag(task->iq()).firstChild();
	for(; !n.isNull(); n = n.nextSibling())
	{
		QDomElement e = n.toElement();
		if(e.isNull())
			continue;
		if(e.attribute("xmlns") == "jabber:x:data")
		{
			XMPP::XData form;
			form.fromXml(e);
			mXDataWidget = new JabberXDataWidget(form, ui.dynamicForm);
			ui.dynamicForm->layout()->addWidget(mXDataWidget);
			mXDataWidget->show();
			xdataform = true;
		}
	}

	// translate the form and create it inside the display widget
	if(!xdataform)
	{
		translator = new JabberFormTranslator(task->form(), ui.dynamicForm);
		ui.dynamicForm->layout()->addWidget( translator );
		translator->show();
	}

	// enable the send button
	enableButton(User1, true);
	resize(sizeHint());
}

void dlgSearch::slotSendForm()
{
	JT_XSearch *task = new JT_XSearch(mAccount->client()->rootTask());
	connect(task, SIGNAL(finished()), this, SLOT(slotSentForm()));

	if(mXDataWidget)
	{
		XMPP::XData form;
		form.setFields(mXDataWidget->fields());
		task->setForm(mForm, form);
	}
	else
	{
		task->set(translator->resultData());
	}
	task->go(true);
	ui.tblResults->clear();
	enableButton(User1, false);
	enableButton(Close, false);
}

void dlgSearch::slotSentForm()
{
	JT_XSearch * task = (JT_XSearch *) sender ();
	enableButton(User1, true);
	enableButton(Close, true);

	if (!task->success ())
	{
		KMessageBox::queuedMessageBox (this, KMessageBox::Error, i18n ("The Jabber server rejected the search."), i18n ("Jabber Search"));
		return;
	}

	if(mXDataWidget)
	{
		XMPP::XData form;
		QDomNode n = queryTag(task->iq()).firstChild();
		for(; !n.isNull(); n = n.nextSibling())
		{
			QDomElement e = n.toElement();
			if(e.isNull())
				continue;
			if(e.attribute("xmlns") == "jabber:x:data")
			{
				form.fromXml(e);
				break;
			}
		}
		ui.tblResults->setColumnCount(form.report().count());
		//while(tblResults->columnCount() > 0)
		//	tblResults->removeColumn(0);
		QStringList hdrs;
		QList<XMPP::XData::ReportField>::ConstIterator it = form.report().begin();
		for(; it != form.report().end(); it++)
			hdrs << (*it).label;
		ui.tblResults->setHeaderLabels(hdrs);
		QList<XMPP::XData::ReportItem>::ConstIterator iit = form.reportItems().begin();
		for(; iit != form.reportItems().end(); iit++)
		{
			QTreeWidgetItem *item = new QTreeWidgetItem(0);
			it = form.report().begin();
			int n = 0;
			for(; it != form.report().end(); it++)
			{
				item->setText(n, (*iit)[(*it).name]);
				n++;
			}
			ui.tblResults->addTopLevelItem(item);
		}
	}
	else
	{
		ui.tblResults->setColumnCount(5);
		QStringList hdrs;
		hdrs << i18n("JID");
		hdrs << i18n("Nickname");
		hdrs << i18nc("First name", "First");
		hdrs << i18nc("Last name", "Last");
		hdrs << i18n("e-mail");
		ui.tblResults->setHeaderLabels(hdrs);
		for(QList<XMPP::SearchResult>::const_iterator it = task->results().begin(); it != task->results().end(); it++)
		{
			QTreeWidgetItem *item = new QTreeWidgetItem(0);
			item->setText(0, (*it).jid().bare());
			item->setText(1, (*it).nick());
			item->setText(2, (*it).first());
			item->setText(3, (*it).last());
			item->setText(4, (*it).email());
			ui.tblResults->addTopLevelItem(item);
		}
	}
}

#include "dlgsearch.moc"
