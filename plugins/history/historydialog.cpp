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

#include <qfile.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qdir.h>
#include <qdatetime.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <klistview.h>

class KListViewMonthItem : public KListViewItem
{
public:
    KListViewMonthItem(KListView* parent, QString text);
	KListViewMonthItem(KListViewItem *item, int month);
    QString key( int column, bool ascending ) const;
	int getMonth() { return mMonth; }
private:
	int mMonth;
};

KListViewMonthItem::KListViewMonthItem(KListViewItem *item, int month)
		: KListViewItem(item, QDate::longMonthName(month))
{
	mMonth = month;
}

QString KListViewMonthItem::key(int column, bool ascending) const
{
	if ( column == 0 )
	{
		return mMonth < 10 ? "0" + QString::number(mMonth) : QString::number(mMonth);
	}
	else
	{
		return QListViewItem::key( column, ascending );
	}
}


HistoryDialog::HistoryDialog(Kopete::MetaContact *mc, int count, QWidget* parent,
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

	mXsltParser = new Kopete::XSLT(styleContents, this);

	mMainWidget = new HistoryViewer(this, "HistoryDialog::mMainWidget");
	setMainWidget(mMainWidget);



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

	mMainWidget->dateListView->setRootIsDecorated(true);
	mMainWidget->dateListView->setSorting(1); // We sort on month number
	
	connect(mMainWidget->dateListView, SIGNAL(clicked(QListViewItem*)), this, SLOT(dateSelected(QListViewItem*)));
	connect(mMainWidget->dateListView, SIGNAL(expanded(QListViewItem*)), this, SLOT(monthExpanded(QListViewItem*)));
	// show the dialog before people get impatient
	show();
	// Load history data
	init();

	resize(QSize(600, 350));
}

bool HistoryDialog::hasChild(KListViewItem *parent, int month)
{
	KListViewMonthItem* item = (KListViewMonthItem*)parent->firstChild();
	do
	{
		kdDebug() << item->getMonth() << " " << month << endl;
		if (item->getMonth() == month)
			return true;
	}
	while(item = (KListViewMonthItem*)item->nextSibling());

	return false;
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
	mMainWidget->dateListView->hideColumn(1);
}

void HistoryDialog::init(Kopete::Contact *c)
{
	
	// Get year and month list
	QRegExp rx( "\\.(\\d\\d\\d\\d)(\\d\\d)" );
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
		if(fi->fileName().contains(c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) )))
		{
			rx.search(fi->fileName());
			
			// We search for an item in the list view with the same year. If then we add the month
			KListViewItem *foundDate = (KListViewItem*)mMainWidget->dateListView->findItem(rx.cap(1), 0);

			if (foundDate)
			{
				if (!hasChild(foundDate, rx.cap(2).toInt()))
					new KListViewMonthItem(foundDate, rx.cap(2).toInt());
				
			}
			else
			{
				KListViewItem *newYearItem = new KListViewItem(mMainWidget->dateListView, rx.cap(1));
				new KListViewMonthItem(newYearItem, rx.cap(2).toInt());
			}
			
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
		if(fi->fileName().contains(c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) )))
		{
			
			rx.search(fi->fileName());
			
			// We search for an item in the list view with the same year. If then we add the month
			KListViewItem *foundDate = (KListViewItem*)mMainWidget->dateListView->findItem(rx.cap(1), 0);
			
			if (foundDate)
			{
				if (!hasChild(foundDate, rx.cap(2).toInt()))
					new KListViewMonthItem(foundDate, rx.cap(2).toInt());
				
			}
			else
			{
				KListViewItem *newYearItem = new KListViewItem(mMainWidget->dateListView, rx.cap(1));
				new KListViewMonthItem(newYearItem, rx.cap(2).toInt());
			}
		}
		++it;
	}
	// END of kopete 0.7.x check
}

void HistoryDialog::dateSelected(QListViewItem* it)
{
	KListViewMonthItem *item = (KListViewMonthItem*)it;
	if (!item) return;

	// Is it really a month (not already with childs) ?
	if (item->parent() && !item->parent()->parent() && !item->firstChild())
	{
		/*
		 * We load the days for the selected month and then expand the tree
		 */
		QDate chosenDate = QDate(item->parent()->text(0).toInt(), item->getMonth(), 1);
		kdDebug() << item->getMonth() << " " << item->parent()->text(0) << endl;
	
		QValueList<int> dayList = mLogger->getDaysForMonth(chosenDate);
		
		for (int i=0; i<dayList.count(); i++)
		{
			new KListViewItem(item, (dayList[i] < 10 ? "0" : "") + QString::number(dayList[i]), "");
		}
		item->setOpen(true);
	}
	else if (item->parent() && item->parent()->parent()) // Then a day is clicked
	{
		QDate currentDate = QDate::currentDate();
		KListViewMonthItem *itemParent = (KListViewMonthItem*)item->parent();
		QDate chosenDate = QDate(item->parent()->parent()->text(0).toInt(), itemParent->getMonth(), item->text(0).toInt());
		QValueList<Kopete::Message> msgs=mLogger->readMessages(chosenDate);
		setMessages(msgs);
	}
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
	QDate d;
	QString accountLabel;
	
	QString resultHTML = "<b><font color=\"red\">" + (*it).timestamp().date().toString() + "</font></b><br/>";;
	DOM::HTMLElement newNode = mHtmlPart->document().createElement(QString::fromLatin1("span"));
			newNode.setAttribute(QString::fromLatin1("dir"), dir);
			newNode.setInnerHTML(resultHTML);

	mHtmlPart->htmlDocument().body().appendChild(newNode);

	for ( it = msgs.begin(); it != msgs.end(); ++it )
	{
		resultHTML = "";

		if ( accountLabel.isEmpty() || accountLabel != (*it).from()->account()->accountLabel())
		{
			if (!accountLabel.isEmpty())
				resultHTML += "<br/><br/><br/>";
			resultHTML += "<b><font color=\"blue\">" + (*it).from()->account()->accountLabel() + "</font></b><br/>";
		}
		accountLabel = (*it).from()->account()->accountLabel();

		resultHTML += "(<b>" + (*it).timestamp().time().toString() + "</b>) " +
				((*it).direction() == Kopete::Message::Outbound ? "<font color=\"navy\"><b>&gt;</b></font> " : "<font color=\"orange\"><b>&lt;</b></font> ") + (*it).parsedBody() + "<br/>";

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

#include "historydialog.moc"
