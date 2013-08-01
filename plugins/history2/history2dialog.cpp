/*
    kopetehistory2dialog.cpp - Kopete History2 Dialog

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

#include "history2dialog.h"

#include <QtCore/QPointer>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtGui/QClipboard>
#include <QtGui/QTextDocument>
#include <QtAlgorithms>

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

#include "history2logger.h"
#include "history2import.h"
#include "ui_history2viewer.h"

class KListViewDateItem : public QTreeWidgetItem {
public:
	KListViewDateItem(QTreeWidget* parent, QDate date, Kopete::MetaContact *mc);
	QDate date() const {
		return mDate;
	}
	Kopete::MetaContact *metaContact() const {
		return mMetaContact;
	}

	virtual bool operator<( const QTreeWidgetItem& other ) const;
private:
	QDate mDate;
	Kopete::MetaContact *mMetaContact;
};

KListViewDateItem::KListViewDateItem(QTreeWidget* parent, QDate date, Kopete::MetaContact *mc)
	: QTreeWidgetItem(parent), mDate(date), mMetaContact(mc) {
	setText( 0, mDate.toString(Qt::ISODate) );
	setText( 1, mMetaContact->displayName() );
}

bool KListViewDateItem::operator<( const QTreeWidgetItem& other ) const {
	QTreeWidget *tw = treeWidget();
	int column =  tw ? tw->sortColumn() : 0;
	if ( column > 0 )
		return text(column) < other.text(column);

	//compare dates - do NOT use ascending var here
	const KListViewDateItem* item = static_cast<const KListViewDateItem*>(&other);
	return ( mDate < item->date() );
}

bool metaContactSort(Kopete::MetaContact *mc1,Kopete::MetaContact *mc2){
	return mc1->displayName()<mc2->displayName();
}

History2Dialog::History2Dialog(Kopete::MetaContact *mc, QWidget* parent)
	: KDialog(parent),
	  mSearching(false) {
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
	mMainWidget = new Ui::History2Viewer();
	mMainWidget->setupUi( w );
	mMainWidget->searchLine->setFocus();
	mMainWidget->searchLine->setTrapReturnKey (true);
	mMainWidget->searchLine->setClearButtonShown(true);

	mMainWidget->contactComboBox->addItem(i18n("All"));
	mMetaContactList = Kopete::ContactList::self()->metaContacts();

	qSort(mMetaContactList.begin(), mMetaContactList.end(),metaContactSort);

	foreach(Kopete::MetaContact *metaContact, mMetaContactList) {
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
	connect(mMainWidget->importHistory2, SIGNAL(clicked()), this, SLOT(slotImportHistory2()));
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

	// Load history2 dates in the listview
	init("");
}

History2Dialog::~History2Dialog() {
	// end the search function, if it's still running
	mSearching = false;
	delete mMainWidget;
}

void History2Dialog::init(QString search) {
	mMainWidget->dateTreeWidget->clear();
	if (mMetaContact){
		QList<QDate> dayList = History2Logger::instance()->getDays(mMetaContact, search);
		for (int i=0; i<dayList.count(); i++) {
			new KListViewDateItem(mMainWidget->dateTreeWidget, dayList[i], mMetaContact);
		}
	} else {
		QList<DMPair> dayList = History2Logger::instance()->getDays(search);
		for (int i=0; i<dayList.count(); i++) {
			new KListViewDateItem(mMainWidget->dateTreeWidget, dayList[i].date(), dayList[i].metaContact());
		}
	}
}

void History2Dialog::dateSelected(QTreeWidgetItem* it) {
	kDebug(14310) ;

	KListViewDateItem *item = static_cast<KListViewDateItem*>(it);

	if (!item) return;

	QDate chosenDate = item->date();

	setMessages(History2Logger::instance()->readMessages(chosenDate, item->metaContact()));
}

void History2Dialog::setMessages(QList<Kopete::Message> msgs) {
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
	foreach(const Kopete::Message& msg, msgs) {
		if ( mMainWidget->messageFilterBox->currentIndex() == 0
		        || ( mMainWidget->messageFilterBox->currentIndex() == 1 && msg.direction() == Kopete::Message::Inbound )
		        || ( mMainWidget->messageFilterBox->currentIndex() == 2 && msg.direction() == Kopete::Message::Outbound ) ) {
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
			if ( msg.from()->metaContact() && msg.from()->metaContact() != Kopete::ContactList::self()->myself() ) {
				name = msg.from()->metaContact()->displayName();
			} else {
				name = msg.from()->displayName();
			}

			QString fontColor;
			if (msg.direction() == Kopete::Message::Outbound) {
				fontColor = Kopete::AppearanceSettings::self()->chatTextColor().dark().name();
			} else {
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

void History2Dialog::slotFilterChanged(int /*index*/) {
	dateSelected(mMainWidget->dateTreeWidget->currentItem());
}

void History2Dialog::slotOpenURLRequest(const KUrl &url, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &) {
	kDebug(14310) << "url=" << url.url();
	new KRun(url, 0, false); // false = non-local files
}

// Disable search button if there is no search text
void History2Dialog::slotSearchTextChanged(const QString& searchText) {
	if (searchText.isEmpty()) {
		mMainWidget->searchButton->setEnabled(false);
		treeWidgetHideElements(false);
	} else {
		mMainWidget->searchButton->setEnabled(true);
	}
}

void History2Dialog::treeWidgetHideElements(bool s) {
	KListViewDateItem *item;
	for ( int i = 0; i < mMainWidget->dateTreeWidget->topLevelItemCount(); i++ ) {
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
void History2Dialog::slotSearch() {

	QString searchFor = mMainWidget->searchLine->text();

	init(searchFor);
	searchFinished();
}

void History2Dialog::searchFinished() {
	mMainWidget->searchButton->setText(i18n("&Search"));
	mSearching = false;
	doneProgressBar();
}

// When a contact is selected in the combobox. Item 0 is All contacts.
void History2Dialog::slotContactChanged(int index) {
	if (index == 0) {
		setCaption(i18n("History for All Contacts"));
		mMetaContact = 0;
		init("");
	} else {
		mMetaContact = mMetaContactList.at(index-1);
		setCaption(i18n("History for %1", mMetaContact->displayName()));
		init("");
	}
}

void History2Dialog::initProgressBar(const QString& text, int nbSteps) {
	mMainWidget->searchProgress->setMaximum(nbSteps);
	mMainWidget->searchProgress->setValue(0);
	mMainWidget->searchProgress->show();
	mMainWidget->statusLabel->setText(text);
}

void History2Dialog::doneProgressBar() {
	mMainWidget->searchProgress->hide();
	mMainWidget->statusLabel->setText(i18n("Ready"));
}

void History2Dialog::slotRightClick(const QString &url, const QPoint &point) {
	KMenu *chatWindowPopup = new KMenu();

	if ( !url.isEmpty() ) {
		mURL = url;
		chatWindowPopup->addAction( mCopyURLAct );
		chatWindowPopup->addSeparator();
	}
	mCopyAct->setEnabled( mHtmlPart->hasSelection() );
	chatWindowPopup->addAction( mCopyAct );

	connect( chatWindowPopup, SIGNAL(aboutToHide()), chatWindowPopup, SLOT(deleteLater()) );
	chatWindowPopup->popup(point);
}

void History2Dialog::slotCopy() {
	QString qsSelection;
	qsSelection = mHtmlPart->selectedText();
	if ( qsSelection.isEmpty() ) return;

	disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
	QApplication::clipboard()->setText(qsSelection, QClipboard::Clipboard);
	QApplication::clipboard()->setText(qsSelection, QClipboard::Selection);
	connect( QApplication::clipboard(), SIGNAL(selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
}

void History2Dialog::slotCopyURL() {
	disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
	QApplication::clipboard()->setText( mURL, QClipboard::Clipboard);
	QApplication::clipboard()->setText( mURL, QClipboard::Selection);
	connect( QApplication::clipboard(), SIGNAL(selectionChanged()), mHtmlPart, SLOT(slotClearSelection()));
}

void History2Dialog::slotImportHistory2(void) {
	QPointer <History2Import> importer = new History2Import(this);
	importer->exec();
	delete importer;
}

QString History2Dialog::highlight(const QString &htmlText, const QString &highlight) const {
	const int highlightLength = highlight.length();
	QString highlightedText;
	int eIndex = -1;
	int sIndex = 0;
	int midLen;

	for (;;) {
		sIndex = htmlText.indexOf("<", eIndex + 1);
		midLen = (sIndex == -1) ? -1 : sIndex - eIndex - 1;

		//Text to highlight
		QString body = htmlText.mid(eIndex + 1, midLen);
		int highlightIndex = 0;
		while ((highlightIndex = body.indexOf(highlight, highlightIndex, Qt::CaseInsensitive)) > -1) {
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

QString History2Dialog::escapeXMLText(const QString& text) const {
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

#include "history2dialog.moc"
