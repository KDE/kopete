/*
    kopetehistorydialog.cpp - Kopete History Dialog

    Copyright (c) 2002 by  Richard Stellingwerff <remenic@linuxfromscratch.org>
    Copyright (c) 2004 by  Stefan Gehn <metz AT gehn.net>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "historydialog.h"
#include "historylogger.h"
#include "historyviewer.h"
#include "kopetemetacontact.h"
#include "kopetexsl.h"

#include <dom/dom_doc.h>
#include <dom/dom_element.h>
#include <dom/html_document.h>
#include <dom/html_element.h>
#include <khtml_part.h>
#include <khtmlview.h>

#include <qfile.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <krun.h>
#include <kstandarddirs.h>


HistoryDialog::HistoryDialog(KopeteMetaContact *mc, int count, QWidget* parent,
	const char* name) : KDialogBase(parent, name, false,
		i18n("History for %1").arg(mc->displayName()), Close, Close)
{
	kdDebug(14310) << k_funcinfo << "called." << endl;
	setWFlags(Qt::WDestructiveClose);	// send SIGNAL(closing()) on quit

	mMetaContact = mc;
	msgCount = count;
	mLogger= new HistoryLogger(mMetaContact, this);
	// BEGIN TODO: add to history-prefs
 	QString styleContents;
	QFile file(locate("appdata", QString::fromLatin1("styles/Kopete.xsl")));
	if (file.open(IO_ReadOnly))
	{
		QTextStream stream(&file);
		styleContents = stream.read();
		file.close();
	}
	// END TODO

	mXsltParser = new KopeteXSLT(styleContents);

	mMainWidget = new HistoryViewer(this, "HistoryDialog::mMainWidget");
	setMainWidget(mMainWidget);

	mMainWidget->mBack->setPixmap(SmallIcon("2leftarrow"));
	mMainWidget->mPrevious->setPixmap(SmallIcon(QString::fromLatin1("1leftarrow")));
	mMainWidget->mNext->setPixmap(SmallIcon(QString::fromLatin1("1rightarrow")));
	mMainWidget->mForward->setPixmap(SmallIcon(QString::fromLatin1("2rightarrow")));


	mMainWidget->htmlFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	QVBoxLayout *l = new QVBoxLayout(mMainWidget->htmlFrame);
	mHtmlPart = new KHTMLPart(mMainWidget->htmlFrame, "htmlHistoryView");
	//Security settings, we don't need this stuff
	mHtmlPart->setJScriptEnabled(false);
	mHtmlPart->setJavaEnabled(false);
	mHtmlPart->setPluginsEnabled(false);
	mHtmlPart->setMetaRefreshEnabled(false);

	mHtmlView = mHtmlPart->view();
	mHtmlView->setMarginWidth(4);
	mHtmlView->setMarginHeight(4);
	mHtmlView->setFocusPolicy(NoFocus);
	mHtmlView->setSizePolicy(
		QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	l->addWidget(mHtmlView);

	mHtmlPart->begin();
	mHtmlPart->write( QString::fromLatin1( "<html><head></head><body></body></html>") );
	mHtmlPart->end();

	connect(mHtmlPart->browserExtension(), SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)),
		this, SLOT(slotOpenURLRequest(const KURL &, const KParts::URLArgs &)));
	/*
	connect(mHtmlPart, SIGNAL(popupMenu(const QString &, const QPoint &)),
		this, SLOT(slotRightClick(const QString &, const QPoint &)) );
	connect(htmlView, SIGNAL(contentsMoving(int,int)),
		this, SLOT(slotScrollingTo(int,int)) );
	*/


	connect(mMainWidget->mNext, SIGNAL(clicked()),
		this, SLOT(slotNextClicked()));
	connect(mMainWidget->mPrevious, SIGNAL(clicked()),
		this, SLOT(slotPrevClicked()));
	connect(mMainWidget->mForward, SIGNAL(clicked()),
		this, SLOT(slotForwardClicked()));
	connect(mMainWidget->mBack, SIGNAL(clicked()),
		this, SLOT(slotBackClicked()));

	connect(mMainWidget->chkOldestFirst, SIGNAL(toggled(bool)),
		this, SLOT(slotReversedToggled(bool)));
	connect(mMainWidget->chkIncomingOnly, SIGNAL(toggled(bool)),
		this, SLOT(slotIncomingToggled(bool)));
	connect(mMainWidget->btnSearch, SIGNAL(clicked()),
		this, SLOT(slotSearchClicked()));

	refreshEnabled(Prev|Next);

	// show the dialog before people get impatient
	show();
	// Load history data
	init();
}

void HistoryDialog::init()
{
	slotBackClicked();
}


void HistoryDialog::setMessages(QValueList<KopeteMessage> msgs)
{
	// Clear View
	DOM::HTMLElement htmlBody = mHtmlPart->htmlDocument().body();
	while(htmlBody.hasChildNodes())
		htmlBody.removeChild(htmlBody.childNodes().item(htmlBody.childNodes().length() - 1));
	// ----

	QString dir = (QApplication::reverseLayout() ? QString::fromLatin1("rtl") :
		QString::fromLatin1("ltr"));

	QValueList<KopeteMessage>::iterator it;

	for ( it = msgs.begin(); it != msgs.end(); ++it )
	{
		QDomDocument message = (*it).asXML();
		QString resultHTML = mXsltParser->transform(message.toString());

		DOM::HTMLElement newNode = mHtmlPart->document().createElement(QString::fromLatin1("span"));
			newNode.setAttribute(QString::fromLatin1("dir"), dir);
			newNode.setInnerHTML(resultHTML);

		mHtmlPart->htmlDocument().body().appendChild(newNode);
	}
}


void HistoryDialog::slotPrevClicked()
{
	QValueList<KopeteMessage> msgs = mLogger->readMessages(msgCount, 0,
		!mMainWidget->chkOldestFirst->isChecked() ? HistoryLogger::Chronological :
		HistoryLogger::AntiChronological, true, false);

	if(msgs.count() < msgCount)
		refreshEnabled(Prev);
	else
		refreshEnabled(0);

	setMessages(msgs);
}

void HistoryDialog::slotNextClicked()
{
	QValueList<KopeteMessage> msgs = mLogger->readMessages(msgCount,
		0, mMainWidget->chkOldestFirst->isChecked() ? HistoryLogger::Chronological :
		HistoryLogger::AntiChronological , false, false);

	if(msgs.count() < msgCount)
		refreshEnabled(Next);
	else
		refreshEnabled(0);

	setMessages(msgs);
}

void HistoryDialog::slotBackClicked()
{
	if(mMainWidget->chkOldestFirst->isChecked())
		mLogger->setPositionToFirst();
	else
		mLogger->setPositionToLast();

	QValueList<KopeteMessage> msgs=mLogger->readMessages(msgCount, 0,
		mMainWidget->chkOldestFirst->isChecked() ? HistoryLogger::Chronological :
		HistoryLogger::AntiChronological, false, false);

	if(msgs.count() < msgCount)
		refreshEnabled(Next | Prev);
	else
		refreshEnabled(Prev);

	setMessages(msgs);
}

void HistoryDialog::slotForwardClicked()
{
	if(!mMainWidget->chkOldestFirst->isChecked())
		mLogger->setPositionToFirst();
	else
		mLogger->setPositionToLast();

	QValueList<KopeteMessage> msgs=mLogger->readMessages(msgCount, 0,
		!mMainWidget->chkOldestFirst->isChecked() ? HistoryLogger::Chronological :
		HistoryLogger::AntiChronological, true, false);

	if(msgs.count() < msgCount)
		refreshEnabled(Next | Prev);
	else
		refreshEnabled(Next);

	setMessages(msgs);
}

void HistoryDialog::slotSearchClicked()
{
	if (mMainWidget->txtSearch->text().stripWhiteSpace().isEmpty())
		mLogger->setFilter(QString::null); //cancel the search
	else
		mLogger->setFilter(mMainWidget->txtSearch->text().stripWhiteSpace());

	slotBackClicked();
}

void HistoryDialog::slotReversedToggled(bool /*b*/)
{
	slotBackClicked();
}

void HistoryDialog::slotIncomingToggled(bool b)
{
	mLogger->setHideOutgoing( b );
	slotBackClicked();
}


void HistoryDialog::refreshEnabled(uint disabled)
{
	mMainWidget->mPrevious->setEnabled(!(disabled & Prev));
	mMainWidget->mBack->setEnabled(!(disabled & Prev));

	mMainWidget->mNext->setEnabled(!(disabled & Next));
	mMainWidget->mForward->setEnabled(!(disabled & Next));
}

void HistoryDialog::slotOpenURLRequest(const KURL &url, const KParts::URLArgs &/*args*/)
{
	kdDebug(14310) << k_funcinfo << "url=" << url.url() << endl;
	new KRun(url, 0, false); // false = non-local files
}

#include "historydialog.moc"
