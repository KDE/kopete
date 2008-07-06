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
#include "kopeteprefs.h"

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
#include <qclipboard.h>

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
#include <kpopupmenu.h>
#include <kstdaction.h>
#include <kaction.h>

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
		: KListViewItem(parent, date.toString(Qt::ISODate), mc->displayName())
{
	mDate = date;
	mMetaContact = mc;
}

int KListViewDateItem::compare(QListViewItem *i, int col, bool ascending) const
{
	if (col) 
		return QListViewItem::compare(i, col, ascending);

	//compare dates - do NOT use ascending var here
	KListViewDateItem* item = static_cast<KListViewDateItem*>(i);
	if ( mDate < item->date() )
		return -1;
	return ( mDate > item->date() );
}


HistoryDialog::HistoryDialog(Kopete::MetaContact *mc, QWidget* parent,
	const char* name) : KDialogBase(parent, name, false,
		i18n("History for %1").arg(mc->displayName()), 0), mSearching(false)
{
	QString fontSize;
	QString htmlCode;
	QString fontStyle;

	kdDebug(14310) << k_funcinfo << "called." << endl;
	setWFlags(Qt::WDestructiveClose);	// send SIGNAL(closing()) on quit

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
	QPtrListIterator<Kopete::MetaContact> it(mMetaContactList);
	for(; it.current(); ++it)
	{
		mMainWidget->contactComboBox->insertItem((*it)->displayName());
	}

	if (mMetaContact)
		mMainWidget->contactComboBox->setCurrentItem(mMetaContactList.find(mMetaContact)+1);

	mMainWidget->dateSearchLine->setListView(mMainWidget->dateListView);
	mMainWidget->dateListView->setSorting(0, 0); //newest-first

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
        mHtmlPart->setOnlyLocalReferences(true);

	mHtmlView = mHtmlPart->view();
	mHtmlView->setMarginWidth(4);
	mHtmlView->setMarginHeight(4);
	mHtmlView->setFocusPolicy(NoFocus);
	mHtmlView->setSizePolicy(
	QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	l->addWidget(mHtmlView);

	QTextOStream( &fontSize ) << KopetePrefs::prefs()->fontFace().pointSize();
	fontStyle = "<style>.hf { font-size:" + fontSize + ".0pt; font-family:" + KopetePrefs::prefs()->fontFace().family() + "; color: " + KopetePrefs::prefs()->textColor().name() + "; }</style>";

	mHtmlPart->begin();
	htmlCode = "<html><head>" + fontStyle + "</head><body class=\"hf\"></body></html>";
	mHtmlPart->write( QString::fromLatin1( htmlCode.latin1() ) );
	mHtmlPart->end();

	
	connect(mHtmlPart->browserExtension(), SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)),
		this, SLOT(slotOpenURLRequest(const KURL &, const KParts::URLArgs &)));
	connect(mMainWidget->dateListView, SIGNAL(clicked(QListViewItem*)), this, SLOT(dateSelected(QListViewItem*)));
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
	mCopyURLAct = new KAction( i18n( "Copy Link Address" ), QString::fromLatin1( "editcopy" ), 0, this, SLOT( slotCopyURL() ), ac );

	resize(650, 700);
	centerOnScreen(this);

	// show the dialog before people get impatient
	show();

	// Load history dates in the listview
	init();
}

HistoryDialog::~HistoryDialog()
{
	mSearching = false;
}

void HistoryDialog::init()
{
	if(mMetaContact)
	{
		HistoryLogger logger(mMetaContact, this);
		init(mMetaContact);
	}
	else
	{
		QPtrListIterator<Kopete::MetaContact> it(mMetaContactList);
		for(; it.current(); ++it)
		{
			HistoryLogger logger(*it, this);
			init(*it);
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
		HistoryLogger logger(pair.metaContact(), this);
		QValueList<int> dayList = logger.getDaysForMonth(pair.date());
		for (unsigned int i=0; i<dayList.count(); i++)
		{
				QDate c2Date(pair.date().year(),pair.date().month(),dayList[i]);
				if (mInit.dateMCList.find(pair) == mInit.dateMCList.end())
						new KListViewDateItem(mMainWidget->dateListView, c2Date, pair.metaContact());
		}
		mMainWidget->searchProgress->advance(1);
		QTimer::singleShot(0,this,SLOT(slotLoadDays()));


}

void HistoryDialog::init(Kopete::MetaContact *mc)
{
	QPtrList<Kopete::Contact> contacts=mc->contacts();
	QPtrListIterator<Kopete::Contact> it( contacts );

	for( ; it.current(); ++it )
	{
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
	if ( list1 != 0 )
	{
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
	if ( list != 0 )
	{
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
}

void HistoryDialog::dateSelected(QListViewItem* it)
{
	KListViewDateItem *item = static_cast<KListViewDateItem*>(it);

	if (!item) return;

	QDate chosenDate = item->date();

	HistoryLogger logger(item->metaContact(), this);
	QValueList<Kopete::Message> msgs=logger.readMessages(chosenDate);

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
		if ( mMainWidget->messageFilterBox->currentItem() == 0
			|| ( mMainWidget->messageFilterBox->currentItem() == 1 && (*it).direction() == Kopete::Message::Inbound )
			|| ( mMainWidget->messageFilterBox->currentItem() == 2 && (*it).direction() == Kopete::Message::Outbound ) )
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
									"<font color=\"" + KopetePrefs::prefs()->textColor().dark().name() + "\"><b>&gt;</b></font> "
									: "<font color=\"" + KopetePrefs::prefs()->textColor().light(200).name() + "\"><b>&lt;</b></font> ")
					+ body + "<br/>";
	
			newNode = mHtmlPart->document().createElement(QString::fromLatin1("span"));
			newNode.setAttribute(QString::fromLatin1("dir"), dir);
			newNode.setInnerHTML(resultHTML);
	
			mHtmlPart->htmlDocument().body().appendChild(newNode);
		}
	}
}

void HistoryDialog::slotFilterChanged(int /* index */)
{
	dateSelected(mMainWidget->dateListView->currentItem());
}

void HistoryDialog::slotOpenURLRequest(const KURL &url, const KParts::URLArgs &/*args*/)
{
	kdDebug(14310) << k_funcinfo << "url=" << url.url() << endl;
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

/*
* How does the search work
* ------------------------
* We do the search respecting the current metacontact filter item. To do this, we iterate over the
* elements in the KListView (KListViewDateItems) and, for each one, we iterate over its subcontacts,
* manually searching the log files of each one. To avoid searching files twice, the months that have
* been searched already are stored in searchedMonths. The matches are placed in the matches QMap.
* Finally, the current date item is checked in the matches QMap, and if it is present, it is shown.
*
* Keyword highlighting is done in setMessages() : if the search field isn't empty, we highlight the
* search keyword.
*
* The search is _not_ case sensitive
*/
void HistoryDialog::slotSearch()
{
	if (mMainWidget->dateListView->childCount() == 0) return;

	QRegExp rx("^ <msg.*time=\"(\\d+) \\d+:\\d+:\\d+\" >([^<]*)<");
	QMap<QDate, QValueList<Kopete::MetaContact*> > monthsSearched;
	QMap<QDate, QValueList<Kopete::MetaContact*> > matches;

	// cancel button pressed
	if (mSearching)
	{
		listViewShowElements(true);
		goto searchFinished;
	}

	listViewShowElements(false);

	initProgressBar(i18n("Searching..."), mMainWidget->dateListView->childCount());
	mMainWidget->searchButton->setText(i18n("&Cancel"));
	mSearching = true;

	// iterate over items in the date list widget
	for(KListViewDateItem *curItem = static_cast<KListViewDateItem*>(mMainWidget->dateListView->firstChild());
		curItem != 0;
		curItem = static_cast<KListViewDateItem *>(curItem->nextSibling())
	)
	{
		qApp->processEvents();
		if (!mSearching) return;

		QDate month(curItem->date().year(),curItem->date().month(),1);
		// if we haven't searched the relevant history logs, search them now
		if (!monthsSearched[month].contains(curItem->metaContact()))
		{
			monthsSearched[month].push_back(curItem->metaContact());
			QPtrList<Kopete::Contact> contacts = curItem->metaContact()->contacts();
			for(QPtrListIterator<Kopete::Contact> it( contacts ); it.current(); ++it)
			{
				// get filename and open file
				QString filename(HistoryLogger::getFileName(*it, curItem->date()));
				if (!QFile::exists(filename)) continue;
				QFile file(filename);
				file.open(IO_ReadOnly);
				if (!file.isOpen())
				{
					kdWarning(14310) << k_funcinfo << "Error opening " <<
							file.name() << ": " << file.errorString() << endl;
					continue;
				}

				QTextStream stream(&file);
				QString textLine;
				while(!stream.atEnd())
				{
					textLine = stream.readLine();
					if (textLine.contains(mMainWidget->searchLine->text(), false))
					{
						if(rx.search(textLine) != -1)
						{
							// only match message body
							if (rx.cap(2).contains(mMainWidget->searchLine->text()))
								matches[QDate(curItem->date().year(),curItem->date().month(),rx.cap(1).toInt())].push_back(curItem->metaContact());
						}
						// this will happen when multiline messages are searched, properly
						// parsing the files would fix this
						else { }
					}
					qApp->processEvents();
					if (!mSearching) return;
				}
				file.close();
			}
		}

		// relevant logfiles have been searched now, check if current date matches
		if (matches[curItem->date()].contains(curItem->metaContact()))
			curItem->setVisible(true);

		// Next date item
		mMainWidget->searchProgress->advance(1);
	}

searchFinished:
	mMainWidget->searchButton->setText(i18n("Se&arch"));
	mSearching = false;
	doneProgressBar();
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
		mMainWidget->searchProgress->setTotalSteps(nbSteps);
		mMainWidget->searchProgress->setProgress(0);
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
	KPopupMenu *chatWindowPopup = 0L;
	chatWindowPopup = new KPopupMenu();
	
	if ( !url.isEmpty() )
	{
		mURL = url;
		mCopyURLAct->plug( chatWindowPopup );
		chatWindowPopup->insertSeparator();
	}
	mCopyAct->setEnabled( mHtmlPart->hasSelection() );
	mCopyAct->plug( chatWindowPopup );
	
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
