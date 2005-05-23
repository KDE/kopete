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
#include "kopeteprotocol.h"
#include "kopeteaccount.h"

#include <dom/dom_doc.h>
#include <dom/dom_element.h>
#include <dom/html_document.h>
#include <dom/html_element.h>
#include <khtml_part.h>
#include <khtmlview.h>

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qheader.h>
#include <qlabel.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <klistview.h>
#include <klistviewsearchline.h>
#include <kprogress.h>
#include <kiconloader.h>

class KListViewDateItem : public KListViewItem
{
public:
    KListViewDateItem(KListView* parent, QDate date);
	QDate date() { return mDate; }
public:
	int compare(QListViewItem *i, int col, bool ascending) const;
private:
    QDate mDate;
};



KListViewDateItem::KListViewDateItem(KListView* parent, QDate date)
		: KListViewItem(parent, date.toString())
{
	mDate = date;
}

int KListViewDateItem::compare(QListViewItem *i, int col, bool ascending) const
{
	if (col) return QListViewItem::compare(i, col, ascending);

	KListViewDateItem* item = static_cast<KListViewDateItem*>(i);
	if (item->date() > mDate)
		return ascending ? -1 : 1;
	else if (item->date() < mDate)
		return ascending ? 1 : -1;
	
	return 0;
}


HistoryDialog::HistoryDialog(Kopete::MetaContact *mc, QWidget* parent,
	const char* name) : KDialogBase(parent, name, false,
		i18n("History for %1").arg(mc->displayName()), 0)
{
	kdDebug(14310) << k_funcinfo << "called." << endl;
	setWFlags(Qt::WDestructiveClose);	// send SIGNAL(closing()) on quit

	// Class member initializations
	mSearch = 0L;
	mMetaContact = mc;
	mLogger= new HistoryLogger(mMetaContact, this);

	// Widgets initializations
	mMainWidget = new HistoryViewer(this, "HistoryDialog::mMainWidget");
	mMainWidget->dateListView->header()->hide();
	mMainWidget->searchLine->setFocus();
	mMainWidget->searchErase->setPixmap(BarIcon("locationbar_erase"));
	setMainWidget(mMainWidget);

	// Initializing HTML Part
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
	connect(mMainWidget->dateListView, SIGNAL(clicked(QListViewItem*)), this, SLOT(dateSelected(QListViewItem*)));
	connect(mMainWidget->searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
	connect(mMainWidget->searchLine, SIGNAL(returnPressed()), this, SLOT(slotSearch()));
	connect(mMainWidget->searchLine, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearchTextChanged(const QString&)));
	connect(mMainWidget->searchErase, SIGNAL(clicked()), this, SLOT(slotSearchErase()));

	resize(650, 700);
	centerOnScreen(this);

	// show the dialog before people get impatient
	show();

	// Load history dates in the listview
	init();
}

HistoryDialog::~HistoryDialog()
{
	delete mSearch;
}

void HistoryDialog::init()
{
	if(!mMetaContact)
		return;

	QPtrList<Kopete::Contact> contacts=mMetaContact->contacts();
	QPtrListIterator<Kopete::Contact> it( contacts );
	for( ; it.current(); ++it )
	{
		init(*it);
	}

	qHeapSort(m_monthsList);

	initProgressBar(i18n("Loading..."),m_monthsList.count());
	QTimer::singleShot(0,this,SLOT(slotLoadDays()));
}

void HistoryDialog::slotLoadDays()
{
	if(m_monthsList.isEmpty())
	{
		doneProgressBar();
		return;
	}

	QDate cDate=m_monthsList.first();
	m_monthsList.pop_front();
	QValueList<int> dayList = mLogger->getDaysForMonth(cDate);
	for (unsigned int i=0; i<dayList.count(); i++)
	{
		QDate c2Date(cDate.year(),cDate.month(),dayList[i]);
		if (!mMainWidget->dateListView->findItem(c2Date.toString(), 0))
			new KListViewDateItem(mMainWidget->dateListView, c2Date);
	}
	mMainWidget->searchProgress->advance(1);
	QTimer::singleShot(0,this,SLOT(slotLoadDays()));
}

void HistoryDialog::init(Kopete::Contact *c)
{
	// Get year and month list
	QRegExp rx( "\\.(\\d\\d\\d\\d)(\\d\\d)" );
	const QString contact_in_filename=c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) );

	QFileInfo *fi;
	// BEGIN check if there are Kopete 0.7.x
	QDir d1(locateLocal("data",QString("kopete/logs/")+
			c->protocol()->pluginId().replace( QRegExp(QString::fromLatin1("[./~?*]")),QString::fromLatin1("-"))
					   ));
	d1.setFilter( QDir::Files | QDir::NoSymLinks );
	d1.setSorting( QDir::Name );

	const QFileInfoList *list1 = d1.entryInfoList();
	QFileInfoListIterator it1( *list1 );
	
	while ( (fi = it1.current()) != 0 )
	{
		if(fi->fileName().contains(contact_in_filename))
		{
			rx.search(fi->fileName());
			
			// We search for an item in the list view with the same year. If then we add the month
			QDate cDate = QDate(rx.cap(1).toInt(), rx.cap(2).toInt(), 1);

			if(!m_monthsList.contains(cDate))
				m_monthsList.append(cDate);
		}
		++it1;
	}
	// END of kopete 0.7.x check


	QString logDir = locateLocal("data",QString("kopete/logs/")+
			c->protocol()->pluginId().replace( QRegExp(QString::fromLatin1("[./~?*]")),QString::fromLatin1("-")) +
					QString::fromLatin1( "/" ) +
					c->account()->accountId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) )
								);
	QDir d(logDir);
	d.setFilter( QDir::Files | QDir::NoSymLinks );
	d.setSorting( QDir::Name );
	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );
	while ( (fi = it.current()) != 0 )
	{
		if(fi->fileName().contains(contact_in_filename)) 
		{
			rx.search(fi->fileName());
			
			// We search for an item in the list view with the same year. If then we add the month
			QDate cDate = QDate(rx.cap(1).toInt(), rx.cap(2).toInt(), 1);

			if(!m_monthsList.contains(cDate))
				m_monthsList.append(cDate);
		}
		++it;
	}
}

void HistoryDialog::dateSelected(QListViewItem* it)
{
	KListViewDateItem *item = static_cast<KListViewDateItem*>(it);

	if (!item) return;

	QDate chosenDate = QDate::fromString(item->text(0));

	QValueList<Kopete::Message> msgs=mLogger->readMessages(chosenDate);
	setMessages(msgs);

}

void HistoryDialog::setMessages(QValueList<Kopete::Message> msgs)
{
	// Clear View
	DOM::HTMLElement htmlBody = mHtmlPart->htmlDocument().body();
	while(htmlBody.hasChildNodes())
		htmlBody.removeChild(htmlBody.childNodes().item(htmlBody.childNodes().length() - 1));
	// ----

	QString dir = (QApplication::reverseLayout() ? QString::fromLatin1("rtl") :
		QString::fromLatin1("ltr"));

	QValueList<Kopete::Message>::iterator it = msgs.begin();


	QString accountLabel;
	QString resultHTML = "<b><font color=\"red\">" + (*it).timestamp().date().toString() + "</font></b><br/>";
	DOM::HTMLElement newNode = mHtmlPart->document().createElement(QString::fromLatin1("span"));
	newNode.setAttribute(QString::fromLatin1("dir"), dir);
	newNode.setInnerHTML(resultHTML);
	mHtmlPart->htmlDocument().body().appendChild(newNode);

	// Populating HTML Part with messages
	for ( it = msgs.begin(); it != msgs.end(); ++it )
	{
		resultHTML = "";

		if (accountLabel.isEmpty() || accountLabel != (*it).from()->account()->accountLabel())
		// If the message's account is new, just specify it to the user
		{
			if (!accountLabel.isEmpty())
				resultHTML += "<br/><br/><br/>";
			resultHTML += "<b><font color=\"blue\">" + (*it).from()->account()->accountLabel() + "</font></b><br/>";
		}
		accountLabel = (*it).from()->account()->accountLabel();

		QString body = (*it).parsedBody();

		if (!mMainWidget->searchLine->text().isEmpty())
		// If there is a search, then we hightlight the keywords
		{
			body = body.replace(mMainWidget->searchLine->text(), "<span style=\"background-color:yellow\">" + mMainWidget->searchLine->text() + "</span>", false);
		}
	
		resultHTML += "(<b>" + (*it).timestamp().time().toString() + "</b>) "
				   + ((*it).direction() == Kopete::Message::Outbound ?
								"<font color=\"navy\"><b>&gt;</b></font> "
								: "<font color=\"orange\"><b>&lt;</b></font> ")
				   + body + "<br/>";

		newNode = mHtmlPart->document().createElement(QString::fromLatin1("span"));
		newNode.setAttribute(QString::fromLatin1("dir"), dir);
		newNode.setInnerHTML(resultHTML);

		mHtmlPart->htmlDocument().body().appendChild(newNode);
	}
}

void HistoryDialog::slotOpenURLRequest(const KURL &url, const KParts::URLArgs &/*args*/)
{
	kdDebug(14310) << k_funcinfo << "url=" << url.url() << endl;
	new KRun(url, 0, false); // false = non-local files
}

void HistoryDialog::searchFirstStep()
{
	if (!mSearch || !mSearch->item)
	{
		delete mSearch;
		mSearch = 0L;
		return;
	}

	QPtrList<Kopete::Contact> contacts=mMetaContact->contacts();
	QPtrListIterator<Kopete::Contact> it( contacts );

	for( ; it.current(); ++it )
	{
		QString fullText;
		if (mSearch->item->date().month() != mSearch->datePrevious.month())
		{
			QFile file(mLogger->getFileName(*it, mSearch->item->date()));
			file.open(IO_ReadOnly);
			if (!&file)
			{
				mSearch->datePrevious = mSearch->item->date();
				continue;
			}
			QTextStream stream(&file);
			fullText = stream.read();
			file.close();
		}
		if ((mSearch->foundPrevious
			&& mSearch->item->date().month() == mSearch->datePrevious.month())
			|| fullText.contains(mMainWidget->searchLine->text()))
		{
			mSearch->dateSearchMap[mSearch->item->date()] = true;
			mSearch->foundPrevious = true;
			break;
		}
		else if (!mSearch->dateSearchMap[mSearch->item->date()])
		{
			mSearch->dateSearchMap[mSearch->item->date()] = false;
		}

		mSearch->datePrevious = mSearch->item->date();
		
	}

	mSearch->item = static_cast<KListViewDateItem *>(mSearch->item->nextSibling());

	if(mSearch->item != 0)
	{
		// Next iteration
		QTimer::singleShot(0,this,SLOT(searchFirstStep()));
	}
	else
	{
		mSearch->item = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());
		initProgressBar(i18n("Searching...") , mMainWidget->dateListView->childCount() );
		QTimer::singleShot(0, this, SLOT(searchSecondStep()));
	}



}

void HistoryDialog::searchSecondStep()
{
	if (!mSearch)
		return;

	mMainWidget->searchProgress->advance(1);

	QValueList<Kopete::Message> msgs = mLogger->readMessages(mSearch->item->date());
	for (int i = 0; i<msgs.count(); i++)
	{
		if (msgs[i].plainBody().contains(mMainWidget->searchLine->text()))
		{
			mSearch->dateSearchMap[mSearch->item->date()] = true;
			break;
		}
		else
			mSearch->dateSearchMap[mSearch->item->date()] = false;
	}

	if (mSearch->dateSearchMap[mSearch->item->date()])
	{
		mSearch->item->setVisible(true);
	}
	else
	{
		mSearch->item->setVisible(false);
	}

	mSearch->item = static_cast<KListViewDateItem*>(mSearch->item->nextSibling());

	if(mSearch->item != 0)
	{
		// Next iteration
		QTimer::singleShot(0,this,SLOT(searchSecondStep()));
	}
	else
	{
		mMainWidget->searchButton->setText("&Search");
		delete mSearch;
		mSearch = 0L;
	}

}

void HistoryDialog::slotSearchTextChanged(const QString& searchText)
{
	if (searchText.isEmpty())
	{
		mMainWidget->searchButton->setEnabled(false);
	}
	else
	{
		mMainWidget->searchButton->setEnabled(true);
	}
}

void HistoryDialog::slotSearchErase()
{
	mMainWidget->searchLine->setText(QString::null);

	KListViewDateItem* item = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());
	do
	{
			item->setVisible(true);
			item = static_cast<KListViewDateItem*>(item->nextSibling());
	}
	while(item != 0);
}

void HistoryDialog::slotSearch()
{
	if(!mMetaContact)
		return;

	if (mSearch)
	{
		mMainWidget->searchButton->setText("&Search");

		delete mSearch;
		mSearch = 0L;
		return;
	}

	mSearch = new Search();
	mSearch->item = 0;
	mSearch->foundPrevious = false;

	mMainWidget->searchProgress->setProgress(0);



	mMainWidget->searchButton->setText("&Cancel");

	mSearch->item = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());
	QTimer::singleShot(0, this, SLOT(searchFirstStep()));
}


void HistoryDialog::initProgressBar(const QString& text, int nbSteps)
{
	mMainWidget->searchProgress->setTotalSteps(nbSteps);
	mMainWidget->searchProgress->setProgress(0);
	mMainWidget->searchProgress->show();
	mMainWidget->statusLabel->setText(text);
}

void HistoryDialog::doneProgressBar()
{
	mMainWidget->searchProgress->hide();
	mMainWidget->statusLabel->setText("ready");
}

#include "historydialog.moc"
