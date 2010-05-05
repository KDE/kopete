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

#include "exhaustivesearchjob.h"

#include <QStringList>
#include <QRegExp>
#include <KDebug>
#include <QDate>

#include <Nepomuk/ResourceManager>
#include <Soprano/Query/QueryLanguage>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Akonadi/ItemSearchJob>
#include <Akonadi/TransactionSequence>
#include <Akonadi/ItemFetchScope>

ExhaustiveSearchJob::ExhaustiveSearchJob(QString text, QObject* parent): KJob(parent), m_text(text)
{
    m_list = text.split(" ", QString::SkipEmptyParts);
}

void ExhaustiveSearchJob::start()
{
    kDebug() << m_list;
    QStringList dateList;
    
    QStringList days = m_list.filter(QRegExp("^\\d{1,2}$"));
    kDebug() <<"days=" <<days;
    
    foreach( const QString& d , days )
	dateList << "[0-9]{4}-[0-9]{2}-" + QDate(2010,1,d.toInt()).toString("dd");

    QStringList years = m_list.filter(QRegExp("^\\d{4,4}$"));
    kDebug() << "years" <<years;
 
    foreach( const QString& y , years )
	dateList << y + "-[0-9]{2}-[0-9]{2}";
 
    //assuming anybody will enter month only once
//    QRegExp r("jan|feb|mar|apr|may|june|july|aug|sep|oct|nov|dec");
//    int pos = r.indexIn( m_text );
    
    QStringList month;
    
    foreach(const QString &s, m_list)
    {
	if( s.contains("jan", Qt::CaseInsensitive))
	    month<< "01";
	else if( s.contains("feb", Qt::CaseInsensitive))
	    month<< "02";
	else if( s.contains("mar", Qt::CaseInsensitive))
	    month<< "03";
	else if( s.contains("apr", Qt::CaseInsensitive))
	    month<< "04";
	else if( s.contains("may", Qt::CaseInsensitive))
	    month<< "05";
	else if( s.contains("june", Qt::CaseInsensitive))
	    month<< "06";
	else if( s.contains("july", Qt::CaseInsensitive))
	    month<< "07";
	else if( s.contains("aug", Qt::CaseInsensitive))
	    month<< "08";
	else if( s.contains("sep", Qt::CaseInsensitive))
	    month<< "09";
	else if( s.contains("oct", Qt::CaseInsensitive))
	    month<< "10";
	else if( s.contains("nov", Qt::CaseInsensitive))
	    month<< "11";
	else if( s.contains("dec", Qt::CaseInsensitive))
	    month<< "12";
    }
    kDebug() << month;
    
    foreach( const QString &m, month )
	dateList << "[0-9]{4}-" + QDate(2010,m.toInt(),1).toString("MM") + "-[0-9]{2}";
    
    foreach( const QString &s, years )
    {
	foreach( const QString &m, month )
	    dateList << QDate(s.toInt(),m.toInt(),1).toString("yyyy-MM") +"-[0-9]{2}";
	
	foreach( const QString &d, days)
	    dateList << QDate(s.toInt(), 1, d.toInt()).toString("yyyy")+ "-[01][0-9]-" + QDate(s.toInt(), 1, d.toInt()).toString("dd") ;
    }
    
    foreach( const QString &m, month)
    {
	foreach(const QString &d, days )
	    dateList << "[09]{4}-"+ QDate(QDate::currentDate().year(),m.toInt(),d.toInt()).toString("MM-dd");
    }
    
    kDebug() << dateList ;
    
    QString qText = "select distinct ?r ?reqProp1 where { ";
    int i=0;
    for( ; i< m_list.size() - 1 ; i++)
    {
	QString s = m_list.at(i);
	kDebug() << s;
	    qText += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent> ?o .\
		FILTER REGEX(STR(?o),'" + s + "', 'i') . FILTER isLiteral(?o) . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 . } ";
	    qText += " Union " ;
    }
	
	qText += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#plainTextMessageContent> ?o .\
	    FILTER REGEX(STR(?o),'" + m_list.at(i) + "', 'i') . FILTER isLiteral(?o) . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 . } ";
	qText += " } ";
	
	kDebug() << qText ;

    QString queryContact = "select distinct ?r ?reqProp1 where { ";
	i=0;
	for( ; i< m_list.size() - 1 ; i++)
	{
	    QString s = m_list.at(i);
	    queryContact += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#hasEmailAddress> ?i . \
		?i <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress> ?q2 . \
		FILTER REGEX(STR(?q2),'" + s + "', 'i') . FILTER isLiteral(?q2)  . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 } \
		Union \
		{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
		?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname> ?i . \
		FILTER REGEX(STR(?i),'" + s +  "', 'i') . FILTER isLiteral(?i)  . \
		?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 }";
	    queryContact += " Union " ;
	}
	queryContact += "{ ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
	    ?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#hasEmailAddress> ?i . \
	    ?i <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#emailAddress> ?q2 . \
	    FILTER REGEX(STR(?q2),'" + m_list.at(i) + "', 'i') . FILTER isLiteral(?q2)  . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 } \
	    Union \
	    { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sender> ?q1 . \
	    ?q1 <http://www.semanticdesktop.org/ontologies/2007/03/22/nco#fullname> ?i . \
	    FILTER REGEX(STR(?i),'" + m_list.at(i) +  "', 'i') . FILTER isLiteral(?i)  . \
	    ?r <http://akonadi-project.org/ontologies/aneo#akonadiItemId> ?reqProp1 }";
	queryContact += " } ";

    
    QStringList queriesFinal;
    
    foreach( const QString &date, dateList)
    {
	QString q = "select distinct ?r where { \
		    { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#receivedDate> ?o . \
		    FILTER REGEX(STR(?o) , '" + date + "', 'i') . }   \
		    Union \
		    { ?r <http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#sentDate> ?o . \
		    FILTER REGEX(STR(?o) , '" + date + "' , 'i') . } ";
		q += " } ";
		
	queriesFinal << q;
    }
    
//    kDebug() << queriesFinal ;

    Akonadi::TransactionSequence *t = new Akonadi::TransactionSequence;
    connect(t,SIGNAL(finished(KJob*)), this, SLOT(slotTransactionOver(KJob*)));
//    Nepomuk::ResourceManager::instance()->init();
    
    foreach( const QString &query , queriesFinal )
    {
	kDebug() << "executing query\n" << query;
	Akonadi::ItemSearchJob *sJob = new Akonadi::ItemSearchJob(query, t );
	sJob->fetchScope().fetchFullPayload(); ;
	connect(sJob, SIGNAL(finished(KJob*)), this , SLOT(slotDateSearch(KJob*)));
    }
    
    Akonadi::ItemSearchJob *tSearch = new Akonadi::ItemSearchJob(qText, t);
    tSearch->fetchScope().fetchFullPayload();
    connect(tSearch, SIGNAL(finished(KJob*)), this, SLOT(slotTextSearch(KJob*)) );
    
    Akonadi::ItemSearchJob *cSearch = new Akonadi::ItemSearchJob(queryContact, t);
    cSearch->fetchScope().fetchFullPayload();
    connect(cSearch, SIGNAL(finished(KJob*)), this, SLOT(slotContactSearch(KJob*)) );
    
    t->start();
}



void ExhaustiveSearchJob::slotDateSearch(KJob* job)
{kDebug() << " ";
    if(job->error() ) {
	kDebug() << job->errorString();
	return;
	}
	
    Akonadi::ItemSearchJob *j = static_cast<Akonadi::ItemSearchJob*>(job);
    Akonadi::Item::List list = m_items.value("date");
    list.append(j->items());
    m_items.insert("date", list);
	
}


void ExhaustiveSearchJob::slotContactSearch(KJob* job)
{
    kDebug() << " ";
    if(job->error() ) {
	kDebug() << job->errorString();
	return;
	}

    Akonadi::ItemSearchJob *j = static_cast<Akonadi::ItemSearchJob*>(job);
    m_items.insert("contact", j->items() );
}

void ExhaustiveSearchJob::slotTextSearch(KJob* job)
{kDebug() << " ";
    if(job->error() ) {
	kDebug() << job->errorString();
	return;
	}
    Akonadi::ItemSearchJob *j = static_cast<Akonadi::ItemSearchJob*>(job);
    m_items.insert("text", j->items());
}

void ExhaustiveSearchJob::slotTransactionOver(KJob* job)
{kDebug() << " ";
    if(job->error() ) {
	kDebug() << job->errorString();
	return;
	}
    kDebug() << m_items.size();
    emitResult();
}



















