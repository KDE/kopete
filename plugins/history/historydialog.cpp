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
#include "kopetecontactlist.h"

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
#include <kcombobox.h>

class KListViewDateItem : public KListViewItem
{
public:
    KListViewDateItem(KListView* parent, QDate date, Kopete::MetaContact *mc);
	QDate date() { return mDate; }
	Kopete::MetaContact *metaContact() { return mMetaContact; }

public:
	int compare(QListViewItem *i, int col, bool ascending) const;
private:
    QDate mDate;
	Kopete::MetaContact *mMetaContact;
};



KListViewDateItem::KListViewDateItem(KListView* parent, QDate date, Kopete::MetaContact *mc)
		: KListViewItem(parent, date.toString(), mc->displayName())
{
	mDate = date;
	mMetaContact = mc;
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
	mLogger = 0L;

	// FIXME: Allow to show this dialog for only one contact
	mMetaContact = mc;

	

	// Widgets initializations
	mMainWidget = new HistoryViewer(this, "HistoryDialog::mMainWidget");
	mMainWidget->dateListView->header()->hide();
	mMainWidget->searchLine->setFocus();
	mMainWidget->searchErase->setPixmap(BarIcon("locationbar_erase"));

	mMainWidget->contactComboBox->insertItem("All");
	mMetaContactList = Kopete::ContactList::self()->metaContacts();
	QPtrListIterator<Kopete::MetaContact> it(mMetaContactList);
	for(; it.current(); ++it)
	{
		mMainWidget->contactComboBox->insertItem((*it)->displayName());
	}

	if (mMetaContact)
		mMainWidget->contactComboBox->setCurrentItem(mMetaContactList.find(mMetaContact)+1);

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
	connect(mMainWidget->contactComboBox, SIGNAL(activated(int)), this, SLOT(slotContactChanged(int)));

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
	if(mMetaContact)
	{
		delete mLogger;
		mLogger= new HistoryLogger(mMetaContact, this);
		init(mMetaContact);
	}
	else
	{
		QPtrListIterator<Kopete::MetaContact> it(mMetaContactList);
		for(; it.current(); ++it)
		{
			mLogger= new HistoryLogger(*it, this);
			init(*it);
			delete mLogger;
			mLogger = 0;
		}

	}
	initProgressBar(i18n("Loading..."),mInit.dateMCList.count());
	QTimer::singleShot(0,this,SLOT(slotLoadDays()));
}

void HistoryDialog::slotLoadDays()
{
		if(mInit.dateMCList.isEmpty())
		{
				doneProgressBar();
				return;
		}
		
		DMPair pair(mInit.dateMCList.first());
		mInit.dateMCList.pop_front();
		mLogger= new HistoryLogger(pair.metaContact(), this);
		QValueList<int> dayList = mLogger->getDaysForMonth(pair.date());
		for (unsigned int i=0; i<dayList.count(); i++)
		{
				QDate c2Date(pair.date().year(),pair.date().month(),dayList[i]);
				if (mInit.dateMCList.find(pair) == mInit.dateMCList.end())
						new KListViewDateItem(mMainWidget->dateListView, c2Date, pair.metaContact());
		}
		delete mLogger;
		mLogger = 0;
		mMainWidget->searchProgress->advance(1);
		QTimer::singleShot(0,this,SLOT(slotLoadDays()));
}

void HistoryDialog::init(Kopete::MetaContact *mc)
{
	QPtrList<Kopete::Contact> contacts=mc->contacts();
	QPtrListIterator<Kopete::Contact> it( contacts );

	for( ; it.current(); ++it )
	{
		kdDebug() << "    " << (*it)->nickName() << endl;
		init(*it);
	}
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
			
			QDate cDate = QDate(rx.cap(1).toInt(), rx.cap(2).toInt(), 1);

			DMPair pair(cDate, c->metaContact());
			mInit.dateMCList.append(pair);

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

			DMPair pair(cDate, c->metaContact());
			mInit.dateMCList.append(pair);
		}
		++it;
	}
}

void HistoryDialog::dateSelected(QListViewItem* it)
{
	KListViewDateItem *item = static_cast<KListViewDateItem*>(it);

	if (!item) return;

	QDate chosenDate = QDate::fromString(item->text(0));

	mLogger= new HistoryLogger(item->metaContact(), this);
	QValueList<Kopete::Message> msgs=mLogger->readMessages(chosenDate);
	delete mLogger;
	mLogger = 0;

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

void HistoryDialog::searchZeroStep()
{
	searchFirstStep();

	mSearch->item = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());
	initProgressBar(i18n("Searching...") , mMainWidget->dateListView->childCount());
	QTimer::singleShot(0, this, SLOT(searchSecondStep()));
}

void HistoryDialog::searchFirstStep()
{
	if (!mSearch)
	{
		delete mSearch;
		mSearch = 0L;
		return;
	}

	if (!mSearch->currentMetaContact)
		return;

	if (mMetaContactList.at(mMainWidget->contactComboBox->currentItem()) == mSearch->item->metaContact())
	{
		mLogger = new HistoryLogger(mSearch->item->metaContact(), this);

		QPtrList<Kopete::Contact> contacts=mSearch->item->metaContact()->contacts();
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
				|| fullText.contains(mMainWidget->searchLine->text(), false))
			{
				mSearch->dateSearchMap[mSearch->item] = true;
				mSearch->foundPrevious = true;
				break;
			}
			else if (!mSearch->dateSearchMap[mSearch->item])
			{
				mSearch->dateSearchMap[mSearch->item] = false;
			}

			mSearch->datePrevious = mSearch->item->date();
		
		}
		delete mLogger;
	}

	mSearch->item = static_cast<KListViewDateItem *>(mSearch->item->nextSibling());

	if(mSearch->item != 0)
	{
		// Next iteration
		searchFirstStep();
	}
}

void HistoryDialog::searchSecondStep()
{
	if (!mSearch)
		return;

	mMainWidget->searchProgress->advance(1);
	mLogger= new HistoryLogger(mSearch->item->metaContact(), this);
	QValueList<Kopete::Message> msgs = mLogger->readMessages(mSearch->item->date());
	for (int i = 0; i<msgs.count(); i++)
	{
		if (msgs[i].plainBody().contains(mMainWidget->searchLine->text(), false))
		{
			mSearch->dateSearchMap[mSearch->item] = true;
			break;
		}
		else
			mSearch->dateSearchMap[mSearch->item] = false;
	}

	if (mSearch->dateSearchMap[mSearch->item] && mMetaContactList.at(mMainWidget->contactComboBox->currentItem()) == mSearch->item->metaContact())
	{
		mSearch->item->setVisible(true);
;
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
		delete mLogger;
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

void HistoryDialog::listViewShowElements(bool s)
{
	KListViewDateItem* item = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());
	do
	{
			item->setVisible(s);
			item = static_cast<KListViewDateItem*>(item->nextSibling());
	}
	while(item != 0);
}

void HistoryDialog::slotSearchErase()
{
	mMainWidget->searchLine->setText(QString::null);
	listViewShowElements(true);
}

void HistoryDialog::slotSearch()
{
	if (mSearch)
	{
		mMainWidget->searchButton->setText("&Search");

		delete mSearch;
		mSearch = 0L;
		return;
	}

	listViewShowElements(false);

	mSearch = new Search();
	mSearch->item = 0;
	mSearch->foundPrevious = false;

	mMainWidget->searchProgress->setProgress(0);

	mMainWidget->searchButton->setText("&Cancel");

	mSearch->item = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());

	searchZeroStep();
}

void HistoryDialog::slotContactChanged(int index)
{
	mMainWidget->dateListView->clear();
	if (index == 0)
	{
		mMetaContact = 0;
		init();
	}
	else
	{
		mMetaContact = mMetaContactList.at(index-1);
		init();
	}
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
