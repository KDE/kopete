/*
    kopetehistorydialog.cpp - Kopete History Dialog

    Copyright (c) 2002 by  Richard Stellingwerff <remenic@linuxfromscratch.org>
    Copyright (c) 2004 by  Stefan Gehn <metz AT gehn.net>
    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>

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

#include <QtCore/QDir>
#include <QtCore/QTextOStream>
#include <QtGui/QClipboard>
#include <KStandardDirs>

#include <kdebug.h>
#include <krun.h>
#include <kmenu.h>
#include <kaction.h>
#include <QList>
#include <kstandardaction.h>
#include <dom/dom_doc.h>
#include <dom/dom_element.h>
#include <dom/html_document.h>
#include <dom/html_element.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/TransactionSequence>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/itemfetchjob.h>

#include "kopetemetacontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include  <kopete/kopeteappearancesettings.h>

#include "historylogger.h"
#include "historyimport.h"
#include "gethistoryjob.h"
#include "ui_historyviewer.h"

class KListViewDateItem : public QTreeWidgetItem
{
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


HistoryDialog::HistoryDialog(HistoryPlugin* hPlugin, Kopete::MetaContact *mc, QWidget* parent)
        : KDialog(parent), m_hPlugin(hPlugin),
        mSearching(false)
{
    kDebug() << "\nHistoryDialog::HistoryDialog--constructor";

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


    connect(mHtmlPart->browserExtension(), SIGNAL(openUrlRequestDelayed(const KUrl &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &)),
            this, SLOT(slotOpenURLRequest(const KUrl &, const KParts::OpenUrlArguments &, const KParts::BrowserArguments &)));
    connect(mMainWidget->dateTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(dateSelected(QTreeWidgetItem*)));
    connect(mMainWidget->searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
    connect(mMainWidget->searchLine, SIGNAL(returnPressed()), this, SLOT(slotSearch()));
    connect(mMainWidget->searchLine, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearchTextChanged(const QString&)));
    connect(mMainWidget->contactComboBox, SIGNAL(activated(int)), this, SLOT(slotContactChanged(int)));
    connect(mMainWidget->messageFilterBox, SIGNAL(activated(int)), this, SLOT(slotFilterChanged(int )));
    connect(mMainWidget->importHistory, SIGNAL(clicked()), this, SLOT(slotImportHistory()));
    connect(mHtmlPart, SIGNAL(popupMenu(const QString &, const QPoint &)), this, SLOT(slotRightClick(const QString &, const QPoint &)));

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
    kDebug() <<"HistoryDialog distructor";
    // end the search function, if it's still running
    mSearching = false;
    delete mMainWidget;
}

void HistoryDialog::init()
{
    kDebug() <<"HistoryDialog::init()";
    Akonadi::TransactionSequence *transaction = new Akonadi::TransactionSequence;
    connect(transaction,SIGNAL(result(KJob*)), this, SLOT(progressBarSlot(KJob*) ) );

    if (mMetaContact)
    {
//        init(mMetaContact);
        QList<Kopete::Contact *> contacts = mMetaContact->contacts();

        foreach(Kopete::Contact *contact, contacts)
        {        //init(contact);
            Akonadi::Collection c = m_hPlugin->getCollection(contact->account()->accountId(), contact->contactId()) ;
            if ( c.isValid() )
            {
                QVariant v;
                v.setValue<Kopete::Contact *>(contact);
                Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob(c , transaction);
                fetchJob->setProperty("contact",v);
                connect(fetchJob,SIGNAL(result(KJob*)),
                        this,SLOT(initItemJobDone(KJob *)));
            }
        }
    }
    else
    {
        foreach(Kopete::MetaContact *metaContactx, mMetaContactList)
        {
            QList<Kopete::Contact *> contacts = metaContactx->contacts();

            foreach(Kopete::Contact *contact, contacts)
            {        //init(contact);
                Akonadi::Collection c = m_hPlugin->getCollection(contact->account()->accountId(),
                                        contact->contactId());
                if ( c.isValid() )
                {
                    QVariant v;
                    v.setValue<Kopete::Contact *>(contact);
                    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob(c , transaction);
                    fetchJob->setProperty("contact",v);
                    connect(fetchJob,SIGNAL(result(KJob*)),
                            this,SLOT(initItemJobDone(KJob *)));
                }
            }
        }
    }

    transaction->start();
}

void HistoryDialog::progressBarSlot(KJob *job)
{
    if (job->error())
        kDebug() <<"in progress bar, transaction failed";
    else
    {
        //TODO: since the above method goes asynchronous the codes below should
        //	     execute after the jobs are done.
        initProgressBar(i18n("Loading..."),mInit.dateMCList.count());
        QTimer::singleShot(0,this,SLOT(slotLoadDays()));
    }
}

void HistoryDialog::initItemJobDone(KJob *job)
{
    Akonadi::ItemFetchJob *fetchJob = static_cast<Akonadi::ItemFetchJob*>(job);
    Akonadi::Item::List itemList= fetchJob->items();
    if (job->error())
        kDebug() <<"fetch job failed--initItemJobDone"<<job->error();
    else if (itemList.isEmpty() ) {
        kDebug() << "item list is empty";
    }
    else {
        QVariant v;
        v= fetchJob->property("contact");
        Kopete::Contact *c= v.value<Kopete::Contact*>();
        foreach(const Akonadi::Item &item, itemList)
        {
            QDate cDate = QDate(item.modificationTime().toLocalTime().date().year(),item.modificationTime().toLocalTime().date().month(), 1);
            DMPair pair(cDate, c->metaContact());
            mInit.dateMCList.append(pair);
            kDebug() <<cDate;
        }
    }
}


void HistoryDialog::slotLoadDays()
{
    kDebug() <<"HistoryDialog::slotLoadDays";

    if (mInit.dateMCList.isEmpty())
    {
        if (!mMainWidget->searchLine->text().isEmpty())
            QTimer::singleShot(0, this, SLOT(slotSearch()));
        doneProgressBar();
        kDebug() << " now returning";
        return;
    }

    DMPair pair(mInit.dateMCList.first());
//    mInit.dateMCList.pop_front();

    HistoryLogger *hlog = new HistoryLogger::HistoryLogger(m_hPlugin, pair.metaContact());
    m_hlogger=hlog;
    connect(hlog,SIGNAL(getDaysForMonthSignal(QList<int>)),this,SLOT(slotLoadDays2(QList<int>)) );
    hlog->getDaysForMonth(pair.date());
}

void HistoryDialog::slotLoadDays2(QList<int> dayList)
{
    disconnect(m_hlogger,SIGNAL(getDaysForMonthSignal(QList<int>)),this,SLOT(slotLoadDays2(QList<int>)) );
    kDebug() << "slot load days 2";
    DMPair pair(mInit.dateMCList.first());
    mInit.dateMCList.pop_front();

    for (int i=0; i<dayList.count(); i++)
    {
        QDate c2Date(pair.date().year(),pair.date().month(),dayList[i]);
        if (mInit.dateMCList.indexOf(pair) == -1)
            new KListViewDateItem(mMainWidget->dateTreeWidget, c2Date, pair.metaContact());
    }

    mMainWidget->searchProgress->setValue(mMainWidget->searchProgress->value()+1);
    QTimer::singleShot(0,this,SLOT(slotLoadDays()));
    m_hlogger->deleteLater();
}

void HistoryDialog::dateSelected(QTreeWidgetItem* it)
{
    kDebug(14310) ;
    kDebug() <<"void HistoryDialog::dateSelected(QTreeWidgetItem* it)";

    KListViewDateItem *item = static_cast<KListViewDateItem*>(it);

    if (!item) return;

    QDate chosenDate = item->date();

    HistoryLogger *l = new HistoryLogger::HistoryLogger(m_hPlugin,item->metaContact());
    m_hlogger=l;
    connect(m_hlogger,SIGNAL(readMessagesByDateDoneSignal(QList<Kopete::Message>)), this, SLOT(setMessages(QList<Kopete::Message>)) );
    m_hlogger->readMessages(chosenDate);
}

void HistoryDialog::setMessages(QList<Kopete::Message> msgs)
{

    disconnect(m_hlogger,SIGNAL(readMessagesByDateDoneSignal(QList<Kopete::Message>)), this, SLOT(setMessages(QList<Kopete::Message>)) );

    kDebug(14310) ;
    kDebug()<<"HistoryDialog::setMessages(QList<Kopete::Message> msgs)";
    m_hlogger->deleteLater();
    // Clear View
    DOM::HTMLElement htmlBody = mHtmlPart->htmlDocument().body();
    while (htmlBody.hasChildNodes())
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
    kDebug() << "netered slot search";

    QMap<QDate, QList<Kopete::MetaContact*> > monthsSearched;

    // cancel button pressed
    if (mSearching)
    {
        treeWidgetHideElements(false);
        searchFinished();
    }

    if (mMainWidget->dateTreeWidget->topLevelItemCount() == 0) return;

    treeWidgetHideElements(true);

    initProgressBar(i18n("Searching..."), mMainWidget->dateTreeWidget->topLevelItemCount());
    mMainWidget->searchButton->setText(i18n("&Cancel"));
    mSearching = true;

    Akonadi::TransactionSequence *transaction = new Akonadi::TransactionSequence;
    connect(transaction,SIGNAL(result(KJob*)), this, SLOT(transactionDone(KJob*)));
    // iterate over items in the date list widget
    for (int i = 0; i < mMainWidget->dateTreeWidget->topLevelItemCount(); ++i)
    {
        kDebug() << "\n------------------ for loop"<<i;
//        qApp->processEvents();
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
                Akonadi::Collection c = m_hPlugin->getCollection(contact->account()->accountId(),
                                        contact->contactId()) ;
                if ( c.isValid() )
                {
                    Akonadi::Collection coll( c );
                    QVariant v,v1;
                    v.setValue<Kopete::MetaContact *>(curItem->metaContact());
                    v1.setValue<QDate>(month);
                    kDebug() <<"bef his job"<<coll.id()<<month<<v;
                    GetHistoryJob *historyJob = new GetHistoryJob(coll,month,transaction);
                    historyJob->setProperty("date",month);
                    historyJob->setProperty("metaContact",v);
                    connect(historyJob, SIGNAL(result(KJob*)), this, SLOT(getHistoryJobDone(KJob *)));
                }
            }
        }
    }
    kDebug() <<"starting transaction";
    transaction->start();
}
void HistoryDialog::searchFinished()
{
    mMainWidget->searchButton->setText(i18n("Se&arch"));
    mSearching = false;
    doneProgressBar();
}

void HistoryDialog::getHistoryJobDone(KJob *job)
{
    kDebug() <<"HistoryDialog::getHistoryJobDone(KJob *job)";
    if (job->error() )
        kDebug() <<"eror in Historydialog::gethistoryjob"<<job->errorString();
    else
    {
        GetHistoryJob * historyJob= static_cast<GetHistoryJob*>(job);

        History his= historyJob->returnHistory();
        QDate d;
        QVariant v,v1;

        v=historyJob->property("metaContact");
        v1=historyJob->property("date");

        Kopete::MetaContact *c=v.value<Kopete::MetaContact *>();
        d=v1.value<QDate>();

        QList<CHPair> contHis;
        kDebug()<<d<<"HistoryDialog::getHistoryJobDone metaContact"<<c->customDisplayName() ;
        CHPair chpair(c,his);

        if (m_dateHistoryList.contains(d) )
        {
            contHis = m_dateHistoryList.value(d);
        }

        contHis.append(chpair);

        m_dateHistoryList.insert(d,contHis);
    }
}

void HistoryDialog::transactionDone(KJob* job)
{
    kDebug() <<"transactionDone method";
    QMap<QDate, QList<Kopete::MetaContact*> > matches;
    qApp->processEvents();
    if (!mSearching) return;

    if (job->error())
        kDebug() << "failed-HistoryDialog:transactionDone"<<job->errorString();
    else
    {
        QMapIterator<QDate, QList<CHPair> > i(m_dateHistoryList);
        while (i.hasNext())
        {
            kDebug() <<"while";
            i.next();
            QDate month = i.key();
            QList<CHPair> chpair= i.value();
            foreach( const CHPair &chp, chpair)
            {
                Kopete::MetaContact * mc = chp.contact();
                History his = chp.history();
                foreach( const History::Message &msg, his.messages())
                {
                    QString textLine =msg.text();
                    if (textLine.contains(mMainWidget->searchLine->text(), Qt::CaseInsensitive))
                    {
                        matches[QDate(month.year(), month.month(),msg.timestamp().date().day())].push_back(mc);
                    }
                }
            }
            qApp->processEvents();
            if (!mSearching) return;
        }
        for (int i = 0; i < mMainWidget->dateTreeWidget->topLevelItemCount(); ++i)
        {
            KListViewDateItem *curItem = static_cast<KListViewDateItem*>(mMainWidget->dateTreeWidget->topLevelItem(i));
            if (matches[curItem->date()].contains(curItem->metaContact()))
                curItem->setHidden(false);
            mMainWidget->searchProgress->setValue(mMainWidget->searchProgress->value()+1);
        }
        searchFinished();
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

void HistoryDialog::slotImportHistory(void)
{
    HistoryImport importer(this);
    importer.exec();
}

#include "historydialog.moc"
