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

#include <QtCore/QPointer>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtGui/QClipboard>
#include <QtGui/QTextDocument>

#include <kdebug.h>
#include <krun.h>
#include <kmenu.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <dom/dom_doc.h>
#include <dom/dom_element.h>
#include <dom/html_document.h>
#include <dom/html_element.h>
#include <khtml_part.h>
#include <khtmlview.h>

#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopeteappearancesettings.h"

#include "historylogger.h"
#include "historyimport.h"
#include "ui_historyviewer.h"

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
 : KDialog(parent),
   mSearching(false)
{
	setAttribute (Qt::WA_DeleteOnClose, true);
	setCaption( i18n("History for %1", mc->displayName()) );
	setButtons(KDialog::Close);
	QString fontSize;
	QString htmlCode;
	QString fontStyle;

	kDebug(14310) << "called.";


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
	mHtmlView->setFocusPolicy(Qt::ClickFocus);
	mHtmlView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	l->setMargin(0);
	l->addWidget(mHtmlView);

	QTextStream( &fontSize ) << Kopete::AppearanceSettings::self()->chatFont().pointSize();
	fontStyle = "<style>.hf { font-size:" + fontSize + ".0pt; font-family:" + Kopete::AppearanceSettings::self()->chatFont().family() + "; color: " + Kopete::AppearanceSettings::self()->chatTextColor().name() + "; }</style>";

	mHtmlPart->begin();
	htmlCode = "<html><head>" + fontStyle + "</head><body class=\"hf\"></body></html>";
	mHtmlPart->write( QString::fromLatin1( htmlCode.toLatin1() ) );
	mHtmlPart->end();


	connect(mHtmlPart->browserExtension(), SIGNAL(openUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
		this, SLOT(slotOpenURLRequest(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)));
	connect(mMainWidget->dateTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(dateSelected(QTreeWidgetItem*)));
	connect(mMainWidget->searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
	connect(mMainWidget->searchLine, SIGNAL(returnPressed()), this, SLOT(slotSearch()));
	connect(mMainWidget->searchLine, SIGNAL(textChanged(QString)), this, SLOT(slotSearchTextChanged(QString)));
	connect(mMainWidget->contactComboBox, SIGNAL(activated(int)), this, SLOT(slotContactChanged(int)));
	connect(mMainWidget->messageFilterBox, SIGNAL(activated(int)), this, SLOT(slotFilterChanged(int)));
	connect(mMainWidget->importHistory, SIGNAL(clicked()), this, SLOT(slotImportHistory()));
	connect(mHtmlPart, SIGNAL(popupMenu(QString,QPoint)), this, SLOT(slotRightClick(QString,QPoint)));

	//initActions
	mCopyAct = KStandardAction::copy( this, SLOT(slotCopy()), mHtmlView );
	mHtmlView->addAction( mCopyAct );

	mCopyURLAct = new KAction( KIcon("edit-copy"), i18n("Copy Link Address"), mHtmlView );
	mHtmlView->addAction( mCopyURLAct );
	connect( mCopyURLAct, SIGNAL(triggered(bool)), this, SLOT(slotCopyURL()) );

	resize(650, 700);
	centerOnScreen(this);

	// show the dialog before people get impatient
	show();

	// Load history dates in the listview
	init();
}

HistoryDialog::~HistoryDialog()
{
	// end the search function, if it's still running
	mSearching = false;
	delete mMainWidget;
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

	// BEGIN check if there are Kopete 0.7.x
	QDir d1(KStandardDirs::locateLocal("data",QString("kopete/logs/")+
			c->protocol()->pluginId().replace( QRegExp(QString::fromLatin1("[./~?*]")),QString::fromLatin1("-"))
					   ));
	d1.setFilter( QDir::Files | QDir::NoSymLinks );
	d1.setSorting( QDir::Name );

	const QFileInfoList list1 = d1.entryInfoList();
	if ( !list1.isEmpty() )
	{
		foreach( const QFileInfo &fi, list1 )
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
		foreach( const QFileInfo &fi, list )
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

	const QString searchForEscaped = Qt::escape(mMainWidget->searchLine->text());
	
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

			// If there is a search, then we highlight the keywords
			if (!searchForEscaped.isEmpty() && body.contains(searchForEscaped, Qt::CaseInsensitive))
				body = highlight( body, searchForEscaped );

			QString name;
			if ( msg.from()->metaContact() && msg.from()->metaContact() != Kopete::ContactList::self()->myself() )
			{
				name = msg.from()->metaContact()->displayName();
			}
			else
			{
				name = msg.from()->displayName();
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

/*
* How does the search work
* ------------------------
* We do the search respecting the current metacontact filter item. To do this, we iterate over the
* elements in the KListView (KListViewDateItems) and, for each one, we iterate over its subcontacts,
* manually searching the log files of each one. To avoid searching files twice, the months that have
* been searched already are stored in monthsSearched. The matches are placed in the matches QMap.
* Finally, the current date item is checked in the matches QMap, and if it is present, it is shown.
*
* Keyword highlighting is done in setMessages() : if the search field isn't empty, we highlight the
* search keyword.
*
* The search is _not_ case sensitive
*/
void HistoryDialog::slotSearch()
{
	QRegExp rx("^[\\s]*(\\d+)[\\s]+\\d+:\\d+:\\d+[\\s]*$");
	QMap<QDate, QList<Kopete::MetaContact*> > monthsSearched;
	QMap<QDate, QList<Kopete::MetaContact*> > matches;

	// cancel button pressed
	if (mSearching)
	{
		treeWidgetHideElements(false);
		searchFinished();
		return;
	}

	if (mMainWidget->dateTreeWidget->topLevelItemCount() == 0) return;

	treeWidgetHideElements(true);

	initProgressBar(i18n("Searching..."), mMainWidget->dateTreeWidget->topLevelItemCount());
	mMainWidget->searchButton->setText(i18n("&Cancel"));
	mSearching = true;

	QDomDocument doc;

	QString searchFor = mMainWidget->searchLine->text();
	// We are sarching in xml file so we need escaped search text too.
	QString searchForEscaped = escapeXMLText(searchFor);

	const QString StartMsgTag("<msg");
	const QString EndMsgTag("</msg");

	// iterate over items in the date list widget
	for(int i = 0; i < mMainWidget->dateTreeWidget->topLevelItemCount(); ++i)
	{
		qApp->processEvents();
		if (!mSearching) return;

		KListViewDateItem *curItem = static_cast<KListViewDateItem*>(mMainWidget->dateTreeWidget->topLevelItem(i));
		QDate month(curItem->date().year(),curItem->date().month(),1);
		// if we haven't searched the relevant history logs, search them now
		if (!monthsSearched[month].contains(curItem->metaContact()))
		{
			monthsSearched[month].push_back(curItem->metaContact());
			QList<Kopete::Contact*> contacts = curItem->metaContact()->contacts();
			foreach(Kopete::Contact* contact, contacts)
			{
				// get filename and open file
				QString filename(HistoryLogger::getFileName(contact, curItem->date()));
				if (!QFile::exists(filename)) continue;
				QFile file(filename);
				file.open(QIODevice::ReadOnly);
				if (!file.isOpen())
				{
					kWarning(14310) << k_funcinfo << "Error opening " <<
							file.fileName() << ": " << file.errorString() << endl;
					continue;
				}

				QTextStream stream(&file);
				QString textLine;
    			QString msgItem;
				while(!stream.atEnd())
				{
					textLine = stream.readLine();
					if (!textLine.contains(StartMsgTag))
						continue;

					msgItem = textLine;
					// Get whole message
					while(!stream.atEnd() && !textLine.contains(EndMsgTag))
						msgItem += textLine = stream.readLine();

					if (msgItem.contains(searchForEscaped, Qt::CaseInsensitive))
					{
						// Load message
						if (doc.setContent(msgItem))
						{
							// Check if only message body matches
							if (doc.documentElement().text().contains(searchFor, Qt::CaseInsensitive))
							{
								if(rx.indexIn(doc.documentElement().attribute("time")) != -1)
								{
									QDate date(curItem->date().year(),curItem->date().month(),rx.cap(1).toInt());
									matches[date].push_back(curItem->metaContact());
								}
							}
						}
						else
						{
							kDebug(14310) << "Error: Cannot parse:" << msgItem;
						}
					}
					qApp->processEvents();
					if (!mSearching) return;
				}
				file.close();
			}
		}

		// relevant logfiles have been searched now, check if current date matches
		if (matches[curItem->date()].contains(curItem->metaContact()))
			curItem->setHidden(false);

		// Next date item
		mMainWidget->searchProgress->setValue(mMainWidget->searchProgress->value()+1);
	}
	searchFinished();
}

void HistoryDialog::searchFinished()
{
	mMainWidget->searchButton->setText(i18n("&Search"));
	mSearching = false;
	doneProgressBar();
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
	KMenu *chatWindowPopup = new KMenu();

	if ( !url.isEmpty() )
	{
		mURL = url;
		chatWindowPopup->addAction( mCopyURLAct );
		chatWindowPopup->addSeparator();
	}
	mCopyAct->setEnabled( mHtmlPart->hasSelection() );
	chatWindowPopup->addAction( mCopyAct );

	connect( chatWindowPopup, SIGNAL(aboutToHide()), chatWindowPopup, SLOT(deleteLater()) );
	chatWindowPopup->popup(point);
}

void HistoryDialog::slotCopy()
{
	QString qsSelection;
	qsSelection = mHtmlPart->selectedText();
	if ( qsSelection.isEmpty() ) return;

	disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
	QApplication::clipboard()->setText(qsSelection, QClipboard::Clipboard);
	QApplication::clipboard()->setText(qsSelection, QClipboard::Selection);
	connect( QApplication::clipboard(), SIGNAL(selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
}

void HistoryDialog::slotCopyURL()
{
	disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
	QApplication::clipboard()->setText( mURL, QClipboard::Clipboard);
	QApplication::clipboard()->setText( mURL, QClipboard::Selection);
	connect( QApplication::clipboard(), SIGNAL(selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
}

void HistoryDialog::slotImportHistory(void)
{
	QPointer <HistoryImport> importer = new HistoryImport(this);
	importer->exec();
	delete importer;
}

QString HistoryDialog::highlight(const QString &htmlText, const QString &highlight) const
{
	const int highlightLength = highlight.length();
	QString highlightedText;
	int eIndex = -1;
	int sIndex = 0;
	int midLen;

	for (;;)
	{
		sIndex = htmlText.indexOf("<", eIndex + 1);
		midLen = (sIndex == -1) ? -1 : sIndex - eIndex - 1;

		//Text to highlight
		QString body = htmlText.mid(eIndex + 1, midLen);
		int highlightIndex = 0;
		while ((highlightIndex = body.indexOf(highlight, highlightIndex, Qt::CaseInsensitive)) > -1)
		{
			QString after = QString("<span style=\"background-color:yellow\">%1</span>").arg(body.mid(highlightIndex, highlightLength));
			body.replace(highlightIndex, highlightLength, after);
			highlightIndex += after.length();
		}
		highlightedText += body;
		
		if (sIndex == -1)
			break;

		eIndex = htmlText.indexOf(">", sIndex);
		midLen = (eIndex == -1) ? -1 : eIndex - sIndex + 1;
		highlightedText += htmlText.mid(sIndex, midLen); // Tag element

		if (eIndex == -1)
			break;
	}

	return highlightedText;
}

QString HistoryDialog::escapeXMLText(const QString& text) const
{
	if (text.isEmpty())
		return QString();

	QDomDocument doc;
	QDomElement tmpElement = doc.createElement("tmpElement");
	QDomText tmpTextNode = doc.createTextNode(text);
	tmpElement.appendChild(tmpTextNode); //We have to attach the text to element otherwise save won't work correctly

	QString excapedText;
	QTextStream stream(&excapedText, QIODevice::WriteOnly);
	tmpTextNode.save(stream, 0);
	return excapedText;
}

#include "historydialog.moc"
