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
#include "ui_historyviewer.h"
#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopeteappearancesettings.h"

#include <dom/dom_doc.h>
#include <dom/dom_element.h>
#include <dom/html_document.h>
#include <dom/html_element.h>
#include <khtml_part.h>
#include <khtmlview.h>

#include <QDir>
#include <QClipboard>
#include <QTextOStream>

#include <kdebug.h>
#include <krun.h>
#include <kmenu.h>
#include <kaction.h>
#include <kactioncollection.h>

class KListViewDateItem : public QTreeWidgetItem
{
public:
	KListViewDateItem(QTreeWidget* parent, QDate date, Kopete::MetaContact *mc);
	QDate date() const { return mDate; }
	Kopete::MetaContact *metaContact() const { return mMetaContact; }

	virtual bool operator<( const QTreeWidgetItem& other ) const;
private:
    QDate mDate;
	Kopete::MetaContact *mMetaContact;
};



KListViewDateItem::KListViewDateItem(QTreeWidget* parent, QDate date, Kopete::MetaContact *mc)
: QTreeWidgetItem(parent), mDate(date), mMetaContact(mc)
{
	setText( 0, mDate.toString(Qt::ISODate) );
	setText( 1, mMetaContact->displayName() );
}

bool KListViewDateItem::operator<( const QTreeWidgetItem& other ) const
{
	QTreeWidget *tw = treeWidget();
	int column =  tw ? tw->sortColumn() : 0;
	if ( column > 0 )
		return text(column) < other.text(column);

	//compare dates - do NOT use ascending var here
	const KListViewDateItem* item = static_cast<const KListViewDateItem*>(&other);
	return ( mDate < item->date() );
}


HistoryDialog::HistoryDialog(Kopete::MetaContact *mc, QWidget* parent)
 : KDialog(parent)
{
	setAttribute (Qt::WA_DeleteOnClose, true);
	setCaption( i18n("History for %1", mc->displayName()) );
	setButtons(KDialog::Close);
	QString fontSize;
	QString htmlCode;
	QString fontStyle;

	kDebug(14310) << "called.";

	// Class member initializations
	mSearch = 0L;

	// FIXME: Allow to show this dialog for only one contact
	mMetaContact = mc;



	// Widgets initializations
	QWidget* w = new QWidget( this );
	mMainWidget = new Ui::HistoryViewer();
	mMainWidget->setupUi( w );
	mMainWidget->searchLine->setFocus();
	mMainWidget->searchLine->setTrapReturnKey (true);
	mMainWidget->searchLine->setClearButtonShown(true);

	mMainWidget->contactComboBox->addItem(i18n("All"));
	mMetaContactList = Kopete::ContactList::self()->metaContacts();

	foreach(Kopete::MetaContact *metaContact, mMetaContactList)
	{
		mMainWidget->contactComboBox->addItem(metaContact->displayName());
	}


	if (mMetaContact)
		mMainWidget->contactComboBox->setCurrentIndex(mMetaContactList.indexOf(mMetaContact)+1);

	mMainWidget->dateSearchLine->setTreeWidget(mMainWidget->dateTreeWidget);
	mMainWidget->dateTreeWidget->sortItems(0, Qt::DescendingOrder); //newest-first

	setMainWidget( w );

	// Initializing HTML Part
	mMainWidget->htmlFrame->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	QVBoxLayout *l = new QVBoxLayout(mMainWidget->htmlFrame);
	mHtmlPart = new KHTMLPart(mMainWidget->htmlFrame);

	//Security settings, we don't need this stuff
	mHtmlPart->setJScriptEnabled(false);
	mHtmlPart->setJavaEnabled(false);
	mHtmlPart->setPluginsEnabled(false);
	mHtmlPart->setMetaRefreshEnabled(false);
	mHtmlPart->setOnlyLocalReferences(true);

	mHtmlView = mHtmlPart->view();
	mHtmlView->setMarginWidth(4);
	mHtmlView->setMarginHeight(4);
	mHtmlView->setFocusPolicy(Qt::NoFocus);
	mHtmlView->setSizePolicy(
	QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	l->setMargin(0);
	l->addWidget(mHtmlView);

	QTextStream( &fontSize ) << Kopete::AppearanceSettings::self()->chatFont().pointSize();
	fontStyle = "<style>.hf { font-size:" + fontSize + ".0pt; font-family:" + Kopete::AppearanceSettings::self()->chatFont().family() + "; color: " + Kopete::AppearanceSettings::self()->chatTextColor().name() + "; }</style>";

	mHtmlPart->begin();
	htmlCode = "<html><head>" + fontStyle + "</head><body class=\"hf\"></body></html>";
	mHtmlPart->write( QString::fromLatin1( htmlCode.toLatin1() ) );
	mHtmlPart->end();


	connect(mHtmlPart->browserExtension(), SIGNAL(openUrlRequestDelayed(const KUrl &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &)),
		this, SLOT(slotOpenURLRequest(const KUrl &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &)));
	connect(mMainWidget->dateTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(dateSelected(QTreeWidgetItem*)));
	connect(mMainWidget->searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
	connect(mMainWidget->searchLine, SIGNAL(returnPressed()), this, SLOT(slotSearch()));
	connect(mMainWidget->searchLine, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearchTextChanged(const QString&)));
	connect(mMainWidget->contactComboBox, SIGNAL(activated(int)), this, SLOT(slotContactChanged(int)));
	connect(mMainWidget->messageFilterBox, SIGNAL(activated(int)), this, SLOT(slotFilterChanged(int )));
	connect(mHtmlPart, SIGNAL(popupMenu(const QString &, const QPoint &)), this, SLOT(slotRightClick(const QString &, const QPoint &)));

	//initActions
	KActionCollection* ac = new KActionCollection(this);
	mCopyAct = KStandardAction::copy( this, SLOT(slotCopy()), ac );
	mCopyURLAct = new KAction( KIcon("edit-copy"), i18n( "Copy Link Address" ), this );
        ac->addAction( "mCopyURLAct", mCopyURLAct );
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
	delete mMainWidget;
	delete mSearch;
}

void HistoryDialog::init()
{
	if(mMetaContact)
	{
		init(mMetaContact);
	}
	else
	{
		foreach(Kopete::MetaContact *metaContact, mMetaContactList)
		{
			init(metaContact);
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

	HistoryLogger hlog(pair.metaContact());

	QList<int> dayList = hlog.getDaysForMonth(pair.date());
	for (int i=0; i<dayList.count(); i++)
	{
		QDate c2Date(pair.date().year(),pair.date().month(),dayList[i]);
		if (mInit.dateMCList.indexOf(pair) == -1)
			new KListViewDateItem(mMainWidget->dateTreeWidget, c2Date, pair.metaContact());
	}

	mMainWidget->searchProgress->setValue(mMainWidget->searchProgress->value()+1);
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
	QDir d1(KStandardDirs::locateLocal("data",QString("kopete/logs/")+
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

	QString logDir = KStandardDirs::locateLocal("data",QString("kopete/logs/")+
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

void HistoryDialog::dateSelected(QTreeWidgetItem* it)
{
	kDebug(14310) ;

	KListViewDateItem *item = static_cast<KListViewDateItem*>(it);

	if (!item) return;

	QDate chosenDate = item->date();

	setMessages(HistoryLogger(item->metaContact()).readMessages(chosenDate));
}

void HistoryDialog::setMessages(QList<Kopete::Message> msgs)
{
	kDebug(14310) ;

	// Clear View
	DOM::HTMLElement htmlBody = mHtmlPart->htmlDocument().body();
	while(htmlBody.hasChildNodes())
		htmlBody.removeChild(htmlBody.childNodes().item(htmlBody.childNodes().length() - 1));
	// ----

	QString dir = (QApplication::isRightToLeft() ? QString::fromLatin1("rtl") :
		QString::fromLatin1("ltr"));

	QString accountLabel;
	QString date = msgs.isEmpty() ? "" : msgs.front().timestamp().date().toString();
	QString resultHTML = "<b><font color=\"red\">" + date + "</font></b><br/>";

	DOM::HTMLElement newNode = mHtmlPart->document().createElement(QString::fromLatin1("span"));
	newNode.setAttribute(QString::fromLatin1("dir"), dir);
	newNode.setInnerHTML(resultHTML);
	mHtmlPart->htmlDocument().body().appendChild(newNode);

	// Populating HTML Part with messages
	foreach(const Kopete::Message& msg, msgs)
	{
		if ( mMainWidget->messageFilterBox->currentIndex() == 0
			|| ( mMainWidget->messageFilterBox->currentIndex() == 1 && msg.direction() == Kopete::Message::Inbound )
			|| ( mMainWidget->messageFilterBox->currentIndex() == 2 && msg.direction() == Kopete::Message::Outbound ) )
		{
			resultHTML.clear();

			if (accountLabel.isEmpty() || accountLabel != msg.from()->account()->accountLabel())
			// If the message's account is new, just specify it to the user
			{
				if (!accountLabel.isEmpty())
					resultHTML += "<br/><br/><br/>";
				resultHTML += "<b><font color=\"blue\">" + msg.from()->account()->accountLabel() + "</font></b><br/>";
			}
			accountLabel = msg.from()->account()->accountLabel();

			QString body = msg.parsedBody();

			if (!mMainWidget->searchLine->text().isEmpty())
			// If there is a search, then we hightlight the keywords
			{
				body = body.replace(mMainWidget->searchLine->text(), "<span style=\"background-color:yellow\">" + mMainWidget->searchLine->text() + "</span>", Qt::CaseInsensitive);
			}

			QString name;
			if ( msg.from()->metaContact() && msg.from()->metaContact() != Kopete::ContactList::self()->myself() )
			{
				name = msg.from()->metaContact()->displayName();
			}
			else
			{
				name = msg.from()->nickName();
			}

			QString fontColor;
			if (msg.direction() == Kopete::Message::Outbound)
			{
				fontColor = Kopete::AppearanceSettings::self()->chatTextColor().dark().name();
			}
			else
			{
				fontColor = Kopete::AppearanceSettings::self()->chatTextColor().light(200).name();
			}

			QString messageTemplate = "<b>%1&nbsp;<font color=\"%2\">%3</font></b>&nbsp;%4";
			resultHTML += messageTemplate.arg( msg.timestamp().time().toString(),
				fontColor, name, body );

			newNode = mHtmlPart->document().createElement(QString::fromLatin1("span"));
			newNode.setAttribute(QString::fromLatin1("dir"), dir);
			newNode.setInnerHTML(resultHTML);

			mHtmlPart->htmlDocument().body().appendChild(newNode);
		}
	}
}

void HistoryDialog::slotFilterChanged(int /*index*/)
{
	dateSelected(mMainWidget->dateTreeWidget->currentItem());
}

void HistoryDialog::slotOpenURLRequest(const KUrl &url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &)
{
	kDebug(14310) << "url=" << url.url();
	new KRun(url, 0, false); // false = non-local files
}

// Disable search button if there is no search text
void HistoryDialog::slotSearchTextChanged(const QString& searchText)
{
	if (searchText.isEmpty())
	{
		mMainWidget->searchButton->setEnabled(false);
		treeWidgetHideElements(false);
	}
	else
	{
		mMainWidget->searchButton->setEnabled(true);
	}
}

void HistoryDialog::treeWidgetHideElements(bool s)
{
	KListViewDateItem *item;
	for ( int i = 0; i < mMainWidget->dateTreeWidget->topLevelItemCount(); i++ )
	{
		item = static_cast<KListViewDateItem*>(mMainWidget->dateTreeWidget->topLevelItem(i));
		if ( item )
			item->setHidden(s);
	}
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

	if (mMainWidget->dateTreeWidget->topLevelItemCount() == 0) return;

	treeWidgetHideElements(true);

	mSearch = new Search();
	mSearch->itemIndex = 0;
	mSearch->foundPrevious = false;

	initProgressBar(i18n("Searching..."), mMainWidget->dateTreeWidget->topLevelItemCount() );
	mMainWidget->searchButton->setText(i18n("&Cancel"));

	searchFirstStep();
}

void HistoryDialog::searchFirstStep()
{
	QRegExp rx("^ <msg.*time=\"(\\d+) \\d+:\\d+:\\d+\" >");

	if (!mSearch)
	{
		return;
	}

	KListViewDateItem *item = static_cast<KListViewDateItem*>(mMainWidget->dateTreeWidget->topLevelItem(mSearch->itemIndex));
	if (item && !mSearch->dateSearchMap[item->date()].contains(item->metaContact()))
	{
		if (mMainWidget->contactComboBox->currentIndex() == 0
		    || mMetaContactList.at(mMainWidget->contactComboBox->currentIndex()-1) == item->metaContact())
		{
			HistoryLogger hlog(item->metaContact());

			QList<Kopete::Contact*> contacts=item->metaContact()->contacts();

			foreach(Kopete::Contact* contact, contacts)
			{
				mSearch->datePrevious = item->date();

				QString fullText;

				QFile file(hlog.getFileName(contact, item->date()));
				file.open(QIODevice::ReadOnly);
				if (!file.isOpen())
				{
					continue;
				}
				QTextStream stream(&file);
				QString textLine;
				while(!(textLine = stream.readLine()).isNull())
				{
					if (textLine.contains(mMainWidget->searchLine->text(), Qt::CaseInsensitive))
					{
						rx.indexIn(textLine);
						mSearch->dateSearchMap[QDate(item->date().year(),item->date().month(),rx.cap(1).toInt())].push_back(item->metaContact());
					}
				}

				file.close();
			}
		}
	}

	if ( item && mSearch->dateSearchMap[item->date()].contains(item->metaContact()) )
		item->setHidden(false);

	mSearch->itemIndex++;

	if( mSearch->itemIndex < mMainWidget->dateTreeWidget->topLevelItemCount() )
	{
		// Next iteration
		mMainWidget->searchProgress->setValue(mMainWidget->searchProgress->value()+1);

		QTimer::singleShot(0,this,SLOT(searchFirstStep()));
	}
	else
	{
		mMainWidget->searchButton->setText(i18n("&Search"));

		delete mSearch;
		mSearch = 0L;
		doneProgressBar();
	}
}



// When a contact is selected in the combobox. Item 0 is All contacts.
void HistoryDialog::slotContactChanged(int index)
{
	mMainWidget->dateTreeWidget->clear();
	if (index == 0)
	{
        setCaption(i18n("History for All Contacts"));
        mMetaContact = 0;
		init();
	}
	else
	{
		mMetaContact = mMetaContactList.at(index-1);
        setCaption(i18n("History for %1", mMetaContact->displayName()));
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

	disconnect( QApplication::clipboard(), SIGNAL( selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
	QApplication::clipboard()->setText(qsSelection, QClipboard::Clipboard);
	QApplication::clipboard()->setText(qsSelection, QClipboard::Selection);
	connect( QApplication::clipboard(), SIGNAL( selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
}

void HistoryDialog::slotCopyURL()
{
	disconnect( QApplication::clipboard(), SIGNAL( selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
	QApplication::clipboard()->setText( mURL, QClipboard::Clipboard);
	QApplication::clipboard()->setText( mURL, QClipboard::Selection);
	connect( QApplication::clipboard(), SIGNAL( selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
}

#include "historydialog.moc"
