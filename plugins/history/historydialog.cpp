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
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontactlist.h"
#include "kopeteappearancesettings.h"

#include <dom/dom_doc.h>
#include <dom/dom_element.h>
#include <dom/html_document.h>
#include <dom/html_element.h>
#include <khtml_part.h>
#include <khtmlview.h>

#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLayout>
#include <QDir>
#include <QDateTime>
#include <Q3Header>
#include <QLabel>
#include <QClipboard>
#include <QTextStream>
#include <Q3Frame>
#include <QVBoxLayout>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <k3listview.h>
#include <k3listviewsearchline.h>
#include <kprogressbar.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kmenu.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kaction.h>


class KListViewDateItem : public K3ListViewItem
{
public:
    KListViewDateItem(K3ListView* parent, QDate date, Kopete::MetaContact *mc);
	QDate date() { return mDate; }
	Kopete::MetaContact *metaContact() { return mMetaContact; }

public:
	int compare(Q3ListViewItem *i, int col, bool ascending) const;
private:
    QDate mDate;
	Kopete::MetaContact *mMetaContact;
};



KListViewDateItem::KListViewDateItem(K3ListView* parent, QDate date, Kopete::MetaContact *mc)
		: K3ListViewItem(parent, date.toString(Qt::ISODate), mc->displayName())
{
	mDate = date;
	mMetaContact = mc;
}

int KListViewDateItem::compare(Q3ListViewItem *i, int col, bool ascending) const
{
	if (col) 
		return Q3ListViewItem::compare(i, col, ascending);

	//compare dates - do NOT use ascending var here
	KListViewDateItem* item = static_cast<KListViewDateItem*>(i);
	if ( mDate < item->date() )
		return -1;
	return ( mDate > item->date() );
}


HistoryDialog::HistoryDialog(Kopete::MetaContact *mc, QWidget* parent) : KDialog(parent,
		i18n("History for %1", mc->displayName()))
{
	QString fontSize;
	QString htmlCode;
	QString fontStyle;

	kDebug(14310) << k_funcinfo << "called." << endl;
	//setWFlags(Qt::WDestructiveClose);	// send SIGNAL(closing()) on quit

	// Class member initializations
	mSearch = 0L;
	mLogger = 0L;

	// FIXME: Allow to show this dialog for only one contact
	mMetaContact = mc;

	

	// Widgets initializations
	mMainWidget = new HistoryViewer(this, "HistoryDialog::mMainWidget");
	mMainWidget->searchLine->setFocus(); 
	mMainWidget->searchLine->setTrapReturnKey (true);
	mMainWidget->searchLine->setTrapReturnKey(true);
	mMainWidget->searchErase->setPixmap(BarIcon("locationbar_erase"));

	mMainWidget->contactComboBox->insertItem(i18n("All"));
	mMetaContactList = Kopete::ContactList::self()->metaContacts();
	
	foreach(Kopete::MetaContact *metaContact, mMetaContactList)
	{
		mMainWidget->contactComboBox->insertItem(metaContact->displayName());
	}


	if (mMetaContact)
		mMainWidget->contactComboBox->setCurrentItem(mMetaContactList.indexOf(mMetaContact)+1);

	mMainWidget->dateSearchLine->setListView(mMainWidget->dateListView);
	mMainWidget->dateListView->setSorting(0, 0); //newest-first

	setMainWidget(mMainWidget);

	// Initializing HTML Part
	mMainWidget->htmlFrame->setFrameStyle(Q3Frame::WinPanel | Q3Frame::Sunken);
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
	mHtmlView->setFocusPolicy(Qt::NoFocus);
	mHtmlView->setSizePolicy(
	QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	l->addWidget(mHtmlView);

	QTextOStream( &fontSize ) << Kopete::AppearanceSettings::self()->chatFont().pointSize();
	fontStyle = "<style>.hf { font-size:" + fontSize + ".0pt; font-family:" + Kopete::AppearanceSettings::self()->chatFont().family() + "; color: " + Kopete::AppearanceSettings::self()->chatTextColor().name() + "; }</style>";

	mHtmlPart->begin();
	htmlCode = "<html><head>" + fontStyle + "</head><body class=\"hf\"></body></html>";
	mHtmlPart->write( QString::fromLatin1( htmlCode.toLatin1() ) );
	mHtmlPart->end();

	
	connect(mHtmlPart->browserExtension(), SIGNAL(openURLRequestDelayed(const KUrl &, const KParts::URLArgs &)),
		this, SLOT(slotOpenURLRequest(const KUrl &, const KParts::URLArgs &)));
	connect(mMainWidget->dateListView, SIGNAL(clicked(Q3ListViewItem*)), this, SLOT(dateSelected(Q3ListViewItem*)));
	connect(mMainWidget->searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
	connect(mMainWidget->searchLine, SIGNAL(returnPressed()), this, SLOT(slotSearch()));
	connect(mMainWidget->searchLine, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearchTextChanged(const QString&)));
	connect(mMainWidget->searchErase, SIGNAL(clicked()), this, SLOT(slotSearchErase()));
	connect(mMainWidget->contactComboBox, SIGNAL(activated(int)), this, SLOT(slotContactChanged(int)));
	connect(mMainWidget->messageFilterBox, SIGNAL(activated(int)), this, SLOT(slotFilterChanged(int )));
	connect(mHtmlPart, SIGNAL(popupMenu(const QString &, const QPoint &)), this, SLOT(slotRightClick(const QString &, const QPoint &)));

	//initActions
	KActionCollection* ac = new KActionCollection(this);
	mCopyAct = KStdAction::copy( this, SLOT(slotCopy()), ac );
	mCopyURLAct = new KAction( KIcon("editcopy"), i18n( "Copy Link Address" ), ac, "mCopyURLAct" );
	connect(mCopyURLAct, SIGNAL(triggered(bool)), this, SLOT( slotCopyURL() ) );

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
		foreach(Kopete::MetaContact *metaContact, mMetaContactList)
		{
			mLogger= new HistoryLogger(metaContact, this);
			init(metaContact);
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
				if (!mMainWidget->searchLine->text().isEmpty())
					QTimer::singleShot(0, this, SLOT(slotSearch()));
				doneProgressBar();
				return;
		}
		
		DMPair pair(mInit.dateMCList.first());
		mInit.dateMCList.pop_front();
		mLogger= new HistoryLogger(pair.metaContact(), this);
		QList<int> dayList = mLogger->getDaysForMonth(pair.date());
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
	QList<Kopete::Contact*> contacts=mc->contacts();

	foreach(Kopete::Contact *contact, contacts)
	{
		init(contact);
	}
}

void HistoryDialog::init(Kopete::Contact *c)
{
	// Get year and month list
	QRegExp rx( "\\.(\\d\\d\\d\\d)(\\d\\d)" );
	const QString contact_in_filename=c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) );
	QFileInfo fi;
	

	// BEGIN check if there are Kopete 0.7.x
	QDir d1(locateLocal("data",QString("kopete/logs/")+
			c->protocol()->pluginId().replace( QRegExp(QString::fromLatin1("[./~?*]")),QString::fromLatin1("-"))
					   ));
	d1.setFilter( QDir::Files | QDir::NoSymLinks );
	d1.setSorting( QDir::Name );

	const QFileInfoList list1 = d1.entryInfoList();
	if ( !list1.isEmpty() )
	{
		foreach( fi, list1 )
		{
			if(fi.fileName().contains(contact_in_filename))
			{
				rx.indexIn(fi.fileName());

				QDate cDate = QDate(rx.cap(1).toInt(), rx.cap(2).toInt(), 1);

				DMPair pair(cDate, c->metaContact());
				mInit.dateMCList.append(pair);
			}
		}
		
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
	const QFileInfoList list = d.entryInfoList();
	if ( !list.isEmpty() )
	{
		foreach( fi, list )
		{
			if(fi.fileName().contains(contact_in_filename))
			{
				
				rx.indexIn(fi.fileName());
				
				// We search for an item in the list view with the same year. If then we add the month
				QDate cDate = QDate(rx.cap(1).toInt(), rx.cap(2).toInt(), 1);

				DMPair pair(cDate, c->metaContact());
				mInit.dateMCList.append(pair);
			}
		}
	}
}

void HistoryDialog::dateSelected(Q3ListViewItem* it)
{
	KListViewDateItem *item = static_cast<KListViewDateItem*>(it);

	if (!item) return;

	QDate chosenDate = item->date();

	mLogger= new HistoryLogger(item->metaContact(), this);
	QList<Kopete::Message> msgs=mLogger->readMessages(chosenDate);
	delete mLogger;
	mLogger = 0;

	setMessages(msgs);
}

void HistoryDialog::setMessages(QList<Kopete::Message> msgs)
{
	// Clear View
	DOM::HTMLElement htmlBody = mHtmlPart->htmlDocument().body();
	while(htmlBody.hasChildNodes())
		htmlBody.removeChild(htmlBody.childNodes().item(htmlBody.childNodes().length() - 1));
	// ----

	QString dir = (QApplication::isRightToLeft() ? QString::fromLatin1("rtl") :
		QString::fromLatin1("ltr"));

	QList<Kopete::Message>::iterator it = msgs.begin();


	QString accountLabel;
	QString resultHTML = "<b><font color=\"red\">" + (*it).timestamp().date().toString() + "</font></b><br/>";
	DOM::HTMLElement newNode = mHtmlPart->document().createElement(QString::fromLatin1("span"));
	newNode.setAttribute(QString::fromLatin1("dir"), dir);
	newNode.setInnerHTML(resultHTML);
	mHtmlPart->htmlDocument().body().appendChild(newNode);

	// Populating HTML Part with messages
	for ( it = msgs.begin(); it != msgs.end(); ++it )
	{
		if ( mMainWidget->messageFilterBox->currentIndex() == 0
			|| ( mMainWidget->messageFilterBox->currentIndex() == 1 && (*it).direction() == Kopete::Message::Inbound )
			|| ( mMainWidget->messageFilterBox->currentIndex() == 2 && (*it).direction() == Kopete::Message::Outbound ) )
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
									"<font color=\"" + Kopete::AppearanceSettings::self()->chatTextColor().dark().name() + "\"><b>&gt;</b></font> "
									: "<font color=\"" + Kopete::AppearanceSettings::self()->chatTextColor().light(200).name() + "\"><b>&lt;</b></font> ")
					+ body + "<br/>";
	
			newNode = mHtmlPart->document().createElement(QString::fromLatin1("span"));
			newNode.setAttribute(QString::fromLatin1("dir"), dir);
			newNode.setInnerHTML(resultHTML);
	
			mHtmlPart->htmlDocument().body().appendChild(newNode);
		}
	}
}

void HistoryDialog::slotFilterChanged(int index)
{
	dateSelected(mMainWidget->dateListView->currentItem());
}

void HistoryDialog::slotOpenURLRequest(const KUrl &url, const KParts::URLArgs &/*args*/)
{
	kDebug(14310) << k_funcinfo << "url=" << url.url() << endl;
	new KRun(url, 0, false); // false = non-local files
}

// Disable search button if there is no search text
void HistoryDialog::slotSearchTextChanged(const QString& searchText)
{
	if (searchText.isEmpty())
	{
		mMainWidget->searchButton->setEnabled(false);
		slotSearchErase();
	}
	else
	{
		mMainWidget->searchButton->setEnabled(true);
	}
}

void HistoryDialog::listViewShowElements(bool s)
{
	KListViewDateItem* item = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());
	while (item != 0)
	{
			item->setVisible(s);
			item = static_cast<KListViewDateItem*>(item->nextSibling());
	}
}

// Erase the search line, show all date/metacontacts items in the list (accordint to the
// metacontact selected in the combobox)
void HistoryDialog::slotSearchErase()
{
	mMainWidget->searchLine->clear();
	listViewShowElements(true);
}

// Search initialization
void HistoryDialog::slotSearch()
{
	/*
	* How does the search work
	* ------------------------
	* We do the search respecting the current metacontact filter item. So to do this, we iterate (searchFirstStep()) over
	* the elements in the K3ListView (KListViewDateItems) and, for each one, we iterate over its subcontacts, retrieving the log
	* files of each one, log files in which we do a fulltext search. If we match the keyword, we use dateSearchMap to mark which days
	* in this file contain the keyword.
	*
	* Then, we only show the items for which there is a correct dateSearchMap
	*
	* Keyword highlighting is done in setMessages() : if the search field isn't empty, we highlight the search keyword.
	*
	* The search is _not_ case sensitive
	* TODO: Speed up search ! Solutions : optimisations ?
	*/

	if (mSearch)
	{
		mMainWidget->searchButton->setText(i18n("&Search"));
		delete mSearch;
		mSearch = 0L;
		doneProgressBar();
		return;
	}

	if (mMainWidget->dateListView->childCount() == 0) return;

	listViewShowElements(false);

	mSearch = new Search();
	mSearch->item = 0;
	mSearch->foundPrevious = false;

	initProgressBar(i18n("Searching..."), mMainWidget->dateListView->childCount() );
	mMainWidget->searchButton->setText(i18n("&Cancel"));

	mSearch->item = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());
	searchFirstStep();


}

void HistoryDialog::searchFirstStep()
{
	QRegExp rx("^ <msg.*time=\"(\\d+) \\d+:\\d+:\\d+\" >");

	if (!mSearch)
	{
		return;
	}
	
	if (!mSearch->dateSearchMap[mSearch->item->date()].contains(mSearch->item->metaContact()))
	{
		if (mMainWidget->contactComboBox->currentIndex() == 0
				|| mMetaContactList.at(mMainWidget->contactComboBox->currentIndex()-1) == mSearch->item->metaContact())
		{
			mLogger = new HistoryLogger(mSearch->item->metaContact(), this);
	
			QList<Kopete::Contact*> contacts=mSearch->item->metaContact()->contacts();
	
			foreach(Kopete::Contact* contact, contacts)
			{
				mSearch->datePrevious = mSearch->item->date();
	
				QString fullText;

				QFile file(mLogger->getFileName(contact, mSearch->item->date()));
				file.open(QIODevice::ReadOnly);
				if (!file.isOpen())
				{
					continue;
				}
				QTextStream stream(&file);
				QString textLine;
				while((textLine = stream.readLine()) != QString::null)
				{
					if (textLine.contains(mMainWidget->searchLine->text(), false))
					{
						rx.indexIn(textLine);
						mSearch->dateSearchMap[QDate(mSearch->item->date().year(),mSearch->item->date().month(),rx.cap(1).toInt())].push_back(mSearch->item->metaContact());
					}
				}
				
				file.close();
			}
			delete mLogger;
			mLogger = 0L;
		}
	}

	mSearch->item = static_cast<KListViewDateItem *>(mSearch->item->nextSibling());

	if(mSearch->item != 0)
	{
		// Next iteration
		mMainWidget->searchProgress->advance(1);

		QTimer::singleShot(0,this,SLOT(searchFirstStep()));
	}
	else
	{
		mSearch->item = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());
		do
		{
			if (mSearch->dateSearchMap[mSearch->item->date()].contains(mSearch->item->metaContact()))
				mSearch->item->setVisible(true);
		}
		while((mSearch->item = static_cast<KListViewDateItem *>(mSearch->item->nextSibling())));
		mMainWidget->searchButton->setText(i18n("&Search"));

		delete mSearch;
		mSearch = 0L;
		doneProgressBar();
	}
}



// When a contact is selected in the combobox. Item 0 is All contacts.
void HistoryDialog::slotContactChanged(int index)
{
	mMainWidget->dateListView->clear();
	if (index == 0)
	{
        setCaption(i18n("History for All Contacts"));
        mMetaContact = 0;
		init();
	}
	else
	{
		mMetaContact = mMetaContactList.at(index-1);
        setCaption(i18n("History for %1").arg(mMetaContact->displayName()));
		init();
	}
}

void HistoryDialog::initProgressBar(const QString& text, int nbSteps)
{
		mMainWidget->searchProgress->setMaximum(nbSteps);
		mMainWidget->searchProgress->setValue(0);
		mMainWidget->searchProgress->show();
		mMainWidget->statusLabel->setText(text);
}

void HistoryDialog::doneProgressBar()
{
		mMainWidget->searchProgress->hide();
		mMainWidget->statusLabel->setText(i18n("Ready"));
}

void HistoryDialog::slotRightClick(const QString &url, const QPoint &point)
{
	KMenu *chatWindowPopup = 0L;
	chatWindowPopup = new KMenu();
	
	if ( !url.isEmpty() )
	{
		mURL = url;
		chatWindowPopup->addAction( mCopyURLAct );
		chatWindowPopup->addSeparator();
	}
	mCopyAct->setEnabled( mHtmlPart->hasSelection() );
	chatWindowPopup->addAction( mCopyAct );
	
	connect( chatWindowPopup, SIGNAL( aboutToHide() ), chatWindowPopup, SLOT( deleteLater() ) );
	chatWindowPopup->popup(point);
}

void HistoryDialog::slotCopy()
{
	QString qsSelection;
	qsSelection = mHtmlPart->selectedText();
	if ( qsSelection.isEmpty() ) return;
	
	disconnect( kapp->clipboard(), SIGNAL( selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
	QApplication::clipboard()->setText(qsSelection, QClipboard::Clipboard);
	QApplication::clipboard()->setText(qsSelection, QClipboard::Selection);
	connect( kapp->clipboard(), SIGNAL( selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
}

void HistoryDialog::slotCopyURL()
{
	disconnect( kapp->clipboard(), SIGNAL( selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
	QApplication::clipboard()->setText( mURL, QClipboard::Clipboard);
	QApplication::clipboard()->setText( mURL, QClipboard::Selection);
	connect( kapp->clipboard(), SIGNAL( selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
}

#include "historydialog.moc"
