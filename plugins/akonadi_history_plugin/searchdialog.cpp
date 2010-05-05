/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "searchdialog.h"
#include "gettags.h"
#include "exhaustivesearchjob.h"

#include "ui_searchdialog.h"

//
#include <KDebug>
#include <KHTMLPart>

#include <QLineEdit>
#include <QListWidgetItem>
#include <QStringListModel>

#include <ontologies/nie.h>
#include <ontologies/nmo.h>
#include <ontologies/nco.h>
#include <ontologies/immessage.h>

#include <Nepomuk/Tag>
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/AndTerm>
#include <Nepomuk/Query/OrTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Query/ResourceTerm>
#include <Nepomuk/Query/NegationTerm>
#include <Nepomuk/Query/ResourceTypeTerm>
#include <Nepomuk/Query/QueryParser>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/ResourceManager>

#include <Soprano/QueryResultIterator>
#include <Soprano/Query/QueryLanguage>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NAO>

#include <Akonadi/ItemSearchJob>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <history/history.h>

SearchDialog::SearchDialog(QWidget* parent, Qt::WFlags flags): KDialog(parent, flags)
{
    kDebug() << " ";
    setAttribute (Qt::WA_DeleteOnClose, true);
    setCaption( i18n("Search Through Logs") );
//    setButtons(KDialog::Close);
    setButtons(KDialog::None);

    QWidget *w = new QWidget( this );
    
    m_MainWidget = new Ui::SearchDialog();
    m_MainWidget->setupUi( w );
    
    m_resultModel = new QStringListModel( this ) ;
    m_MainWidget->ListResultWidget->setModel( m_resultModel );

    //if the search button is clicked
    connect(m_MainWidget->SearchButton, SIGNAL(clicked(bool)), this, SLOT(slotSearchButtonClicked()) );
    //when a item is clickd on the result widget
    connect( m_MainWidget->ListResultWidget, SIGNAL(clicked(const QModelIndex&)),
		   this, SLOT(itemSelected(const QModelIndex&)));
    
    //fir check boxes, logs, contactm date and exhaustive
    connect( m_MainWidget->CBoxLogs, SIGNAL(stateChanged(int)), this , SLOT(slotCBoxLogs(int)) ) ;
    connect( m_MainWidget->CBoxContact, SIGNAL(stateChanged(int)), this, SLOT(slotCBoxContact(int)) ) ;
    connect( m_MainWidget->CBoxDate, SIGNAL(stateChanged(int)), this , SLOT(slotCBoxDate(int) ) );
    connect( m_MainWidget->CBoxExhaustive , SIGNAL(stateChanged(int)), this , SLOT(slotCBoxExhaustive(int)));
    
    //set the logs checkbox to be true by default
    m_MainWidget->CBoxLogs->setCheckState( Qt::Checked );
    setMainWidget( w );

    GetTags *getTagJob = new GetTags(this);
    connect(getTagJob, SIGNAL(finished(KJob*)), this, SLOT(slotGetTags(KJob*)) );
    getTagJob->start();
    
    m_chats = true;
    m_contacts = false;
    m_date = false;
    m_exhaustive = false;
    m_searchStrings = QStringList() ;
//    show();
}

SearchDialog::~SearchDialog()
{
    delete m_MainWidget ;
}


void SearchDialog::slotCBoxLogs(int state)
{
    if(state == 0)
	m_chats = false;
    else if(state == 2 )
	m_chats = true ;
    
    kDebug() << state << m_chats ;
}

void SearchDialog::slotCBoxContact(int state)
{
    if(state == 0)
	m_contacts = false;
    else if(state == 2 )
	m_contacts = true ;
    
    kDebug() << state << m_contacts;
}

void SearchDialog::slotCBoxDate(int state)
{
    if(state == 0)
	m_date = false;
    else if(state == 2 )
	m_date = true ;
    
    kDebug() << state << m_date;
}

void SearchDialog::slotCBoxExhaustive(int state)
{
    if(state == 0)
    {
	m_exhaustive = false;
	m_MainWidget->CBoxContact->setEnabled(true);
	m_MainWidget->CBoxDate->setEnabled(true);
	m_MainWidget->CBoxLogs->setEnabled(true);
    }
    else if(state == 2 )
    {
	m_exhaustive = true;
	m_MainWidget->CBoxContact->setDisabled(true);
	m_MainWidget->CBoxDate->setDisabled(true);
	m_MainWidget->CBoxLogs->setDisabled(true);
    }
    
    kDebug() << state << m_exhaustive;
}

void SearchDialog::slotSearchButtonClicked()
{
    kDebug() << " " << m_MainWidget->SearchTextBox->text() ;
    
    //to reset the gui && clear the saved items
    reset(); 
    
    if( !m_exhaustive ) { 
	QString query1 = sparqlQuery( m_MainWidget->SearchTextBox->text() );

	Akonadi::ItemSearchJob *job = new Akonadi::ItemSearchJob( query1 );
	job->fetchScope().fetchFullPayload();
	connect(job, SIGNAL(result(KJob*)), this , SLOT(itemSearchJobDone(KJob*)) );
	}
     else if( m_exhaustive ) {
	ExhaustiveSearchJob *search = new ExhaustiveSearchJob(m_MainWidget->SearchTextBox->text(), this );
	connect(search, SIGNAL(finished(KJob*)), this , SLOT( slotExhaustiveSearchDone(KJob*)) );
	search->start() ;
	}
}

void SearchDialog::slotExhaustiveSearchDone(KJob* job)
{
    kDebug() << " ";
    if( job->error() ) {
	kDebug() << job->errorText();
	return;
	}
    ExhaustiveSearchJob *eJob = static_cast<ExhaustiveSearchJob*>(job);
    QHash<QString, Akonadi::Item::List> hash = eJob->items();
    QHash<QString, Akonadi::Item::List>::iterator i;
    QStringList resultIndex;
    kDebug() << hash.keys() ;
    m_items.clear();
    for (i = hash.begin(); i != hash.end(); ++i)
    {
    // cout << i.key() << ": " << i.value() << endl;
	foreach( const Akonadi::Item&item, i.value() ) 
	{	
	    QString itemLabel = item.id() + ":" + i.key() + ":" + item.payload<History>().remoteContactId();
	    kDebug() << itemLabel ;
	    resultIndex<<itemLabel ;
	    m_items << item;
	}
    }
    m_resultModel->setStringList( resultIndex );
    kDebug() << "end";
}

void SearchDialog::slotGetTags(KJob* job)
{
    kDebug() << " ";
    
    if( job->error() ) {
	job->errorString();
	return;
    }
    GetTags *j = static_cast<GetTags*>(job);
    QStringList tags = j->tags();
    m_MainWidget->LabelBox->addItems( tags );
}

void SearchDialog::reset()
{
    m_searchStrings.clear();
    m_items.clear();
    m_resultModel->setStringList( QStringList() );
    m_MainWidget->DisplayResultWidget->clear();
}

void SearchDialog::itemSearchJobDone(KJob* job)
{
    kDebug() << " ";
    if( job->error() ) {
	kDebug() << job->errorText() ;
	return;
    }
    
    Akonadi::ItemSearchJob * j = static_cast<Akonadi::ItemSearchJob*> (job);
    m_items = j->items() ;
    kDebug() << "sixe=:" <<m_items.size() ;
    QStringList resultIndex;
    foreach( const Akonadi::Item &item , m_items )
    {
	if( !item.hasPayload<History>() )
	    continue;
	
	kDebug() << item.remoteId() ; 
	resultIndex << QString::number( item.id() ) + 
			item.payload<History>().remoteContactId() ; 
    }
    
    m_resultModel->setStringList( resultIndex );
    kDebug() << "end of job" ;
}

void SearchDialog::itemSelected(const QModelIndex& index)
{
    kDebug() << " " ;
    if( !index.isValid() )
	return;
    
    kDebug() << index.row() ;
    
    Akonadi::Item i = m_items.at( index.row() );
    History history = i.payload<History>();

    displayResult(history);
}
//I dont think this function is needed?
void SearchDialog::itemFetched(KJob* job )
{
    if( job->error() ) {
	kDebug() << job->errorText() ;
	return;
    }
    
    Akonadi::ItemFetchJob *f = static_cast<Akonadi::ItemFetchJob*>(job);
    
    Akonadi::Item item = f->items().first() ;
    History history = item.payload<History>();
    displayResult(history);

}

//set the formating of the chat to be displayed
void SearchDialog::displayResult(const History &his)
{
 //   m_MainWidget->DisplayResultWidget->setHtml(); ;
    QString finalHtml ;
    
    QString date = his.date().toString() ;
    QString remoteContact = his.remoteContactId() ;
    
    finalHtml += date + "<br/><b>" + remoteContact + "</b><br/>";
    
    foreach( const History::Message &msg, his.messages() )
    {
	QString time = "<b>" + msg.timestamp().time().toString() + "</b>" ;
	QString nick = msg.nick() ;
	QString sender = msg.sender();
	QString text ="<font color=\"red\">" + msg.text() + "</font>"  ;
	QString in = msg.in() ;

	if( text.contains(m_MainWidget->SearchTextBox->text(), Qt::CaseInsensitive ) )
	{
	    kDebug() << "contains " << m_MainWidget->SearchTextBox->text() << text ;
	    QString text2 = "<span style=\"background-color:blue\">" + m_MainWidget->SearchTextBox->text() + "</span>" ;
	    text.replace(m_MainWidget->SearchTextBox->text() , text2 , Qt::CaseInsensitive) ;
	}
	
	if( in.toInt() ==  0 )
	{
	    finalHtml += time + "<br/>" + "me: " + text + "<br/>" ;
	}
	else
	    finalHtml += time + "<br/>" + nick + ": " + text + "<br/>";
    }
    m_MainWidget->DisplayResultWidget->enableRichTextMode();
    
    m_MainWidget->DisplayResultWidget->setHtml(finalHtml);
}

QString SearchDialog::sparqlQuery(QString searchText)
{
    kDebug() << " " ;
    bool single = true;//to chek if there is one wordor more than onw word in user entered text
    QStringList list = searchText.split(" ", QString::SkipEmptyParts);
    m_searchStrings =  list ;

    if( list.size() > 1 )
	single = false;
    //all this code when exhaustive is false, for exhastive is true, write a diff sparql query
    if( m_chats && single && !m_contacts && !m_date && !m_exhaustive)
    {
	m_searchType = SingleWordInChat;
	
	kDebug() << "of type single word in chat";
	QString q = "select distinct ?r ?reqProp1 \
	    where { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent> ?o .\
	    FILTER REGEX(STR(?o),'" + m_searchStrings.first() + "', 'i') . FILTER isLiteral(?o) . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 . }";
	
	return q ;
    }
    //more than one word in context
    else if( m_chats && !single && !m_contacts && !m_date && !m_exhaustive)
    {
	m_searchType = MultipleWordInChat ;
	kDebug() << "multiple word in chat ";
	
	QString q = "select distinct ?r ?reqProp1 where { ";
	int i=0;
	for( ; i< m_searchStrings.size() - 1 ; i++)
	{
	    QString s = m_searchStrings.at(i);
	    kDebug() << s;
	    q += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent> ?o .\
		FILTER REGEX(STR(?o),'" + s + "', 'i') . FILTER isLiteral(?o) . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 . } ";
	    q += " Union " ;
	}
	
	q += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent> ?o .\
	    FILTER REGEX(STR(?o),'" + m_searchStrings.at(i) + "', 'i') . FILTER isLiteral(?o) . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 . } ";
	q += " } ";
	
	kDebug() << q ;
	return q;
    }
    //if contact search
    else if(m_contacts && single && !m_chats && !m_date && !m_exhaustive )
    {
	m_searchType = SingleWordContact;
	kDebug() << "single contact search" ;
	QString q = "select distinct ?r ?reqProp1  where{ \
	    { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
	    ?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#hasEmailAddress> ?i . \
	    ?i <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress> ?q2 . \
	    FILTER REGEX(STR(?q2),'" + m_searchStrings.first() +"', 'i') . FILTER isLiteral(?q2)  . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 } \
	    Union \
	    { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
	    ?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname> ?i . \
	    FILTER REGEX(STR(?i),'" + m_searchStrings.first() + "', 'i') . FILTER isLiteral(?i)  . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1} }"; 

	return q;
    }
    else if( m_contacts && !single && !m_chats && !m_date && !m_exhaustive)
    {
	m_searchType = MultipleWordContact;
	kDebug() << "multiple word contact";
	QString q = "select distinct ?r ?reqProp1 where { ";
	int i=0;
	for( ; i< m_searchStrings.size() - 1 ; i++)
	{
	    QString s = m_searchStrings.at(i);
	    q += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#hasEmailAddress> ?i . \
		?i <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress> ?q2 . \
		FILTER REGEX(STR(?q2),'" + s + "', 'i') . FILTER isLiteral(?q2)  . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 } \
		Union \
		{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname> ?i . \
		FILTER REGEX(STR(?i),'" + s +  "', 'i') . FILTER isLiteral(?i)  . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 }";
	    q += " Union " ;
	}
	q += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
	    ?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#hasEmailAddress> ?i . \
	    ?i <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress> ?q2 . \
	    FILTER REGEX(STR(?q2),'" + m_searchStrings.at(i) + "', 'i') . FILTER isLiteral(?q2)  . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 } \
	    Union \
	    { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
	    ?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname> ?i . \
	    FILTER REGEX(STR(?i),'" + m_searchStrings.at(i) +  "', 'i') . FILTER isLiteral(?i)  . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 }";
	q += " } ";
	
	kDebug() << q ;
	return q;
    }
    if( m_chats && m_contacts && !m_date && !m_exhaustive)
    {
	kDebug() << "contact+log";
	QString q = "select distinct ?r ?reqProp1 where { ";
	if(single)
	{	kDebug() << "single";
	    q += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent> ?o .\
		FILTER REGEX(STR(?o),'" + m_searchStrings.first() + "', 'i') . FILTER isLiteral(?o) . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 . }";
	    q += " Union " ;
	    q += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#hasEmailAddress> ?i . \
		?i <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress> ?q2 . \
		FILTER REGEX(STR(?q2),'" + m_searchStrings.first() +"', 'i') . FILTER isLiteral(?q2)  . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 } \
		Union \
		{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname> ?i . \
		FILTER REGEX(STR(?i),'" + m_searchStrings.first() + "', 'i') . FILTER isLiteral(?i)  . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 }";
	    q += " } ";
	    
	    kDebug() << q;
	    return q;
	}
	else
	{
	    kDebug() << "not single" ;
	    //for text search in logs
	    foreach( const QString &s, m_searchStrings)
	    {
		kDebug() << s;
		q += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent> ?o .\
		    FILTER REGEX(STR(?o),'" + s + "', 'i') . FILTER isLiteral(?o) . \
		    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 . } ";
		q += " Union " ;
	    }
	    //for contact search
	    int i=0;
	    for( ; i< m_searchStrings.size() - 1 ; i++)
	    {
		QString s = m_searchStrings.at(i);
		q += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		    ?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#hasEmailAddress> ?i . \
		    ?i <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress> ?q2 . \
		    FILTER REGEX(STR(?q2),'" + s + "', 'i') . FILTER isLiteral(?q2)  . \
		    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 } \
		    Union \
		    { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		    ?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname> ?i . \
		    FILTER REGEX(STR(?i),'" + s +  "', 'i') . FILTER isLiteral(?i)  . \
		    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 }";
		q += " Union " ;
	    }
	    q += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#hasEmailAddress> ?i . \
		?i <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress> ?q2 . \
		FILTER REGEX(STR(?q2),'" + m_searchStrings.at(i) + "', 'i') . FILTER isLiteral(?q2)  . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 } \
		Union \
		{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname> ?i . \
		FILTER REGEX(STR(?i),'" + m_searchStrings.at(i) +  "', 'i') . FILTER isLiteral(?i)  . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 }";
	    q += " } ";

	    kDebug() << q ;
	    return q;
	}
    }
    //if date search
    else if(m_date)
    {
	m_searchType = Date ;
	kDebug() << "of type search date";
	QString date = m_MainWidget->SearchTextBox->text();
	QString dateStr;
	QStringList monthList = date.split(" ");
	if( monthList.size() == 1 )
	{
	    dateStr = "[0-9][0-9][0-9][0-9]-[0-9]*";
	    int month = findMonth(date);
	    dateStr += QString::number(month, 10);
	    dateStr += "-[0-9][0-9]";
	    kDebug() << date << dateStr;
	}
	    
	else
	{
	    QDate date = parseDate(monthList);
	    dateStr = date.toString("yyyy-MM-dd");
	    kDebug() << date << dateStr;
	}
	QString q = "select distinct ?r ?o where { \
		    { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#receivedDate> ?o . \
		    FILTER REGEX(STR(?o) , '" + dateStr + "', 'i') . }   \
		    Union \
		    { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sentDate> ?o . \
		    FILTER REGEX(STR(?o) , '" + dateStr + "' , 'i') . } ";
	q += " } ";
	kDebug() << q;
	return q;
	
    }
    //general search
    else if(m_exhaustive)
    {
	m_searchType = Exhaustive ;
    }
}


QDate SearchDialog::parseDate(QStringList monthList)
{
    int month=0, year =0, day =0;
    kDebug() << monthList ;
    
    //QStringList monthList = date.split(" ");
    kDebug() << monthList  ;
    
    foreach ( const QString &s,monthList)
    {
	if(s.contains("jan",Qt::CaseInsensitive ))
	    month=1;
	if(s.contains("feb",Qt::CaseInsensitive) )
	    month=2;
	if(s.contains("mar",Qt::CaseInsensitive) )
	    month=3;
	if(s.contains("apr",Qt::CaseInsensitive)) 
	    month=4;
	if(s.contains("may",Qt::CaseInsensitive) )
	    month=5;
	if(s.contains("june",Qt::CaseInsensitive) )
	    month=6;
	if(s.contains("july",Qt::CaseInsensitive) )
	    month=7;
	if(s.contains("aug",Qt::CaseInsensitive)) 
	    month=8;
	if(s.contains("sep",Qt::CaseInsensitive)) 
	    month=9;
	if(s.contains("oct",Qt::CaseInsensitive) )
	    month=10;
	if(s.contains("nov",Qt::CaseInsensitive))
	    month=11;
	if(s.contains("dec",Qt::CaseInsensitive)) 
	    month=12;
	if( month != 0)
	    {
	    monthList.removeOne(s);break;
	}
    }
    kDebug() << monthList;
    foreach ( const QString &s,monthList)
    {
	if(s.length() == 4 )
	{
	    year = s.toInt();
	    monthList.removeOne(s);
	 }
    }
    kDebug() << monthList  ;
    
    day = monthList.first().toInt();
    kDebug() << year << month << day ;
    return QDate(year, month, day  );
}

int SearchDialog::findMonth(QString s)
{
	if(s.contains("january",Qt::CaseInsensitive ))
	    return 1;
	if(s.contains("february",Qt::CaseInsensitive) )
	    return 2;
	if(s.contains("march",Qt::CaseInsensitive) )
	    return 3;
	if(s.contains("april",Qt::CaseInsensitive)) 
	    return 4;
	if(s.contains("may",Qt::CaseInsensitive) )
	    return 5;
	if(s.contains("june",Qt::CaseInsensitive) )
	    return 6;
	if(s.contains("july",Qt::CaseInsensitive) )
	    return 7;
	if(s.contains("august",Qt::CaseInsensitive)) 
	    return 8;
	if(s.contains("september",Qt::CaseInsensitive)) 
	    return 9;
	if(s.contains("october",Qt::CaseInsensitive) )
	    return 10;
	if(s.contains("november",Qt::CaseInsensitive))
	    return 11;
	if(s.contains("december",Qt::CaseInsensitive)) 
	    return 12;

	}

#include "searchdialog.moc"