/*
    historylogger.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

//#include "gethistoryjob.h"
#include "historylogger.h"
#include "historyplugin.h"

#include <QtCore/QRegExp>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QTextStream>
#include <QtCore/QList>
#include <QtCore/QDate>

#include <Akonadi/CollectionCreateJob>
#include <Akonadi/Collection>
#include <Akonadi/TransactionSequence>
#include <akonadi/item.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemcreatejob.h>
#include <QBuffer>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <ksavefile.h>
#include <Akonadi/Collection>
#include <KRandom>

#include "kopeteglobal.h"
#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetemetacontact.h"
#include "kopetechatsession.h"

#include "historyconfig.h"
#include <Akonadi/CollectionFetchJob>

bool messageTimestampLessThan(const Kopete::Message &m1, const Kopete::Message &m2)
{
    return m1.timestamp() < m2.timestamp();
}

bool func(const History::Message &m1, const History::Message &m2)
{ 
    return m1.timestamp() < m2.timestamp();
}

// -----------------------------------------------------------------------------
HistoryLogger::HistoryLogger(Kopete::MetaContact *m , QObject *parent , QObject *hPlugin )
        : QObject(parent)
{
    kDebug() << "historylogger-constructor-metacontact";
    
    m_hPlugin = qobject_cast<HistoryPlugin*>( hPlugin );
    
    m_hideOutgoing=false;
    m_filterCaseSensitive=Qt::CaseSensitive;
    m_filterRegExp=false;
    m_saveTimer=0L;
    m_saveTimerTime=0;
    m_realMonth=QDate::currentDate().month();
    m_metaContact=m;
    m_index=0;
//    m_indexPrev=0;
    m_readmessagesContact = 0L;
    
    m_cachedMonth=-1;
    m_oldSens=Default;

    m_kopeteChat = m_hPlugin->getCollection();
    
    //the contact may be destroyed, for example, if the contact changes its metacontact
    connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

    setPositionToLast();
}


HistoryLogger::HistoryLogger(Kopete::Contact *c, QObject *parent , QObject *hPlugin )
        : QObject(parent)
{
    kDebug() << "historylogger-constructor-"<<c->contactId();
       
    m_hPlugin = qobject_cast<HistoryPlugin*>(hPlugin);
 
    m_saveTimer=0L;
    m_saveTimerTime=0;
    m_cachedMonth=-1;
    m_metaContact=c->metaContact();
    m_hideOutgoing=false;
    m_realMonth=QDate::currentDate().month();
    m_oldSens=Default;
    m_filterCaseSensitive=Qt::CaseSensitive;
    m_filterRegExp=false;
    m_index=0;
//    m_indexPrev=0;
    m_readmessagesContact=0L;
    
    m_tosaveInCollection = m_hPlugin->getCollection(c->account()->accountId(), c->contactId());
    m_parentCollection = m_hPlugin->getCollection(c->account()->accountId());
    m_kopeteChat = m_hPlugin->getCollection() ;
    kDebug() << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"<<m_kopeteChat.remoteId()<<"\n\n\n\n\n----";
    
    connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

    setPositionToLast();
    
    kDebug() << m_cachedMonth;
}

void HistoryLogger::setPositionToLast()
{
    kDebug() << " ";
    setCurrentMonth(0);
    m_oldSens = AntiChronological;
    m_oldMonth=0;
    timeLimit = QDateTime::QDateTime();
//    m_oldElements.clear();
}


void HistoryLogger::setPositionToFirst()
{
    setCurrentMonth( getFirstMonth() );
    m_oldSens = Chronological;
    m_oldMonth=m_currentMonth;
//    m_oldElements.clear();
}


void HistoryLogger::setCurrentMonth(int month)
{
    kDebug() << " " << month;
    m_currentMonth = month;
//    m_currentElements.clear();
}

void HistoryLogger::getHistory(const Kopete::Contact *c, unsigned int month )
{
    kDebug() <<"\nHistoryLogger::gethistoryx()";
    m_month = month;
    m_contact = c;
    
    if (m_realMonth!=QDate::currentDate().month())
    { //We changed month, our index is not correct anymore, clean memory.
        // or we will see what i called "the 31 midnight bug"(TM) :-)  -Olivier
        m_history.clear();
//	m_documents.clear();
        m_cachedMonth=-1;
        m_currentMonth++; //Not usre it's ok, but should work;
        m_oldMonth++;     // idem
        m_realMonth=QDate::currentDate().month();
    }

    if (!m_metaContact)
    { //this may happen if the contact has been moved, and the MC deleted
        if (c && c->metaContact())
            m_metaContact=c->metaContact();
        else
        {
            m_getHistory = History::History();
            kDebug() << "GENERATING SIGNAL";
            emit getHistoryxDone();
            return;
        }
    }

    if (!m_metaContact->contacts().contains(const_cast<Kopete::Contact*>(c)))
    {
        m_getHistory = History::History();
        emit getHistoryxDone();
        return;
    }
    if ( m_history.isEmpty() )
	kDebug() << "m_history is empty";
    else
	kDebug() << "m_history is not empty";

    if ( m_history.contains(c) )
	kDebug() << "m_history contains c";
    else kDebug() << "dosent contain c";
    
    QMap<unsigned int , History> monthHistory = m_history.value(const_cast<Kopete::Contact*>(c));
    kDebug() << "month="<<month;
    if ( monthHistory.isEmpty() )
	kDebug() << "month history is empty";
    else
	kDebug() << "month his size=" << monthHistory.size();

    if (monthHistory.contains(month))
    {kDebug() << "contains the month";
        m_getHistory= monthHistory[month];
	if ( !m_tosaveInItem.isValid() )
	{
	    kDebug() << "to save in item invalid, now setting it up";
	    Akonadi::Item::List list = m_contactItemList.value(c);
	    foreach ( const Akonadi::Item &item , list )
	    {
		if ( item.payload<History>().date().year() == QDate::currentDate().year() && item.payload<History>().date().month() == QDate::currentDate().month() )
		{
		      m_tosaveInItem = item;
		      break;
		}
	    }
	}
        emit getHistoryxDone();
        return;
    }
    else kDebug() << "dosent contain the month :( ";

    Kopete::Contact *con = const_cast<Kopete::Contact*>(c);

    Akonadi::Collection coll;
//    coll = m_tosaveInCollection;
    if (!m_tosaveInCollection.isValid()) 
    {
	coll = m_hPlugin->getCollection(con->account()->accountId() , con->contactId() ) ;
	if ( !coll.isValid() )
	{
	    emit getHistoryxDone();
	    return;
	}
    }
    else 
	coll = m_tosaveInCollection;
    
    Akonadi::Item _item;
    QDate d(QDate::currentDate().addMonths(0-month));
    
    // if itemlist is not empty, then that means that, we have already fetched 
    // the list of items from the collection once
    // And if we dont have the item
    // the the item for this month dosent exit, so get the item of previous month
    if ( !m_contactItemList.value(c).isEmpty() && !m_tosaveInItem.isValid() )
    {
	foreach ( const Akonadi::Item &item, m_contactItemList.value(c))
	{
	    History his;
	    if ( item.hasPayload<History>() )
		his = item.payload<History>() ;
	     if ( his.date().year() == d.year() && his.date().month() == d.month())
	     {
		  _item = item;
		  break;
	     }
	}
	if ( _item.isValid() )
	{
	    Akonadi::ItemFetchJob *f = new Akonadi::ItemFetchJob( _item );
	    f->fetchScope().fetchFullPayload();
	    connect(f,SIGNAL(finished(KJob*)), SLOT(itemFetchSlot(KJob*)) );
	    return;
	}
	else
	{
	    emit getHistoryxDone();
	    return;
	}
    }
    
    QVariant v,v1;
    v.setValue<QDate>(QDate::currentDate().addMonths(0-month));
    v1.setValue<Kopete::Contact*>(con);
    
    Akonadi::ItemFetchJob * fetchjob = new Akonadi::ItemFetchJob( coll );
    fetchjob->fetchScope().fetchPayloadPart(History::HeaderPayload);
    fetchjob->setProperty("date", v);
    fetchjob->setProperty("contact", v1);
    connect(fetchjob, SIGNAL(finished(KJob*)), SLOT(fetchItemHeaderSlot(KJob*)));
        
}


void HistoryLogger::fetchItemHeaderSlot(KJob* job )
{
    kDebug() << " ";
    if (job->error())
    {
	kDebug() << job->errorString();
	return;
    }
    
    QVariant v,v1;
    v = job->property("date");
    QDate d = v.toDate();
    v1 = job->property("contact");
    Kopete::Contact * c = v1.value<Kopete::Contact*>() ;
    kDebug() << c->contactId();
    
    Akonadi::ItemFetchJob *fetchjob = static_cast<Akonadi::ItemFetchJob*>(job);
    Akonadi::Item::List items = fetchjob->items();
    kDebug() << "no of items="<<items.size();
    
    m_contactItemList.insert(c,items);

    foreach ( const Akonadi::Item &item, items)
    {
	History his;
	if ( item.hasPayload<History>() )
	    his = item.payload<History>() ;
	else 
	    kDebug() << "not of payload type history";
	kDebug() << his.date() << d ;
	if ( his.date().year() == d.year() && his.date().month() == d.month())
	{
	    m_tosaveInItem = item;
	    break;
	}
    }
    
    if ( !m_tosaveInItem.isValid() )
    {
	kDebug() << "m_to save in item is invalid";
	emit getHistoryxDone();
	return;
    }
    
    Akonadi::ItemFetchJob * fetchJob = new Akonadi::ItemFetchJob( m_tosaveInItem );
    fetchJob->fetchScope().fetchFullPayload() ;
    connect(fetchJob, SIGNAL(finished(KJob*)), SLOT(itemFetchSlot(KJob*)));
}


void HistoryLogger::itemFetchSlot(KJob* job)
{
    kDebug() << " ";
    if (job->error())
    {
	kDebug() <<"" << job->errorString();
	return;
    }
//    m_index=0;
    Akonadi::ItemFetchJob *fetchjob = static_cast<Akonadi::ItemFetchJob*>(job);
    Akonadi::Item item = fetchjob->items().first();
    
    if ( !item.hasPayload<History>() )
    {
	kWarning() << "item has payload not of type history";
	return;
    }
    
    m_getHistory = item.payload<History>();
    QMap<unsigned int , History> monthHistory= m_history[m_contact];
    monthHistory.insert(m_month,m_getHistory);
    m_history[m_contact] = monthHistory;
    
    kDebug() <<"GENERATING SIGNAL, month inserted ="<<m_month;
    emit getHistoryxDone();
}

void HistoryLogger::appendMessage( const Kopete::Message &msg , const Kopete::Contact *ct)
{
    kDebug() <<" ";
    if (!msg.from())
        return;
    
    // If no contact are given: If the manager is availiable, use the manager's
    // first contact (the channel on irc, or the other contact for others protocols
    const Kopete::Contact *c = ct;
    if (!c && msg.manager() )
    {
        QList<Kopete::Contact*> mb=msg.manager()->members() ;
        c = mb.first();
    }
    if (!c) //If the contact is still not initialized, use the message author.
        c =   msg.direction()==Kopete::Message::Outbound ? msg.to().first() : msg.from()  ;


    if (!m_metaContact)
    { //this may happen if the contact has been moved, and the MC deleted
        if (c && c->metaContact())
            m_metaContact=c->metaContact();
        else
            return;
    }


    if (!c || !m_metaContact->contacts().contains(const_cast<Kopete::Contact*>(c)) )
    {
        /*QPtrList<Kopete::Contact> contacts= m_metaContact->contacts();
        QPtrListIterator<Kopete::Contact> it( contacts );
        for( ; it.current(); ++it )
        {
        	if( (*it)->protocol()->pluginId() == msg.from()->protocol()->pluginId() )
        	{
        		c=*it;
        		break;
        	}
        }*/
        //if(!c)

        kWarning(14310) << "No contact found in this metacontact to" <<
        " append in the history" << endl;
        return;
    }

    QDate date = msg.timestamp().date();

    m_message = msg;
    m_contact = c;
    connect(this, SIGNAL(getHistoryxDone()), SLOT(appendMessage2()) );
    getHistory(c, QDate::currentDate().month() - date.month() - (QDate::currentDate().year() - date.year()) * 12);

}
void HistoryLogger::appendMessage2()
{
    disconnect(this, SIGNAL(getHistoryxDone()),this, SLOT(appendMessage2()));
    kDebug() <<"APPENDmessage--2";
    History history;
    if (!m_getHistory.messages().isEmpty() )
    {
        history = m_getHistory;
    } else {
      kDebug() << " m_gethistory .messages is empty";
    }

    m_getHistory = History::History();

    if (history.messages().isEmpty() )
        kDebug() << "messages in history is empty";

    if ( !history.date().isValid() )
        history.setDate(QDate::currentDate());
    if (history.localContactId().isEmpty() )
        history.setLocalContactId( m_contact->account()->myself()->contactId() );
    if (history.remoteContactId().isEmpty() )
        history.setRemoteContactId( m_contact->contactId() );

    History::Message messagek;
    messagek.setIn( m_message.direction()==Kopete::Message::Outbound ? "0" : "1" );
    messagek.setSender(  m_message.from()->contactId() );
    messagek.setNick( m_message.from()->property( Kopete::Global::Properties::self()->nickName() ).value().toString() );
    messagek.setTimestamp( m_message.timestamp() );
    messagek.setText( m_message.plainBody() );

    history.addMessage(messagek);
    QMap <unsigned int, History > monthHistory;
    monthHistory = m_history[m_contact];
    monthHistory.insert(QDate::currentDate().month() - history.date().month() - (QDate::currentDate().year() - history.date().year()) * 12, history);
    m_history[m_contact]=monthHistory;

    if ( history.date().month() != QDate::currentDate().month() )
    {
        kDebug()<<"\n\nof contact change or month change called:";
        modifyItem();
    }

    m_toSaveHistory=history;

    if (!m_saveTimer)
    {
        m_saveTimer=new QTimer(this);
	kDebug() << "\n\n\n\tconnection to slot\n";
        connect( m_saveTimer, SIGNAL( timeout() ) , this, SLOT(modifyItem()) );
    }
    if (!m_saveTimer->isActive())
    {	kDebug() <<  "\n\n\t\t\ttimer time =" <<m_saveTimerTime ; 
        m_saveTimer->setSingleShot( true );
        m_saveTimer->start( m_saveTimerTime );
	m_t.start();
    }
    kDebug() <<"emitting appendMessageDoneSignal";
    emit appendMessageDoneSignal();
}

HistoryLogger::~HistoryLogger()
{//TODO check for date here as well, in case when i started the chat before the month and closed the chat window
    kDebug() <<"~history logger DISTRUCTOR";
    if (m_saveTimer && m_saveTimer->isActive())
    {
      if (m_saveTimer)
	m_saveTimer->stop();
      Akonadi::Item newItem(m_tosaveInItem); 
      m_tosaveInItem.setPayload<History>(m_toSaveHistory);
      m_tosaveInItem.setModificationTime(QDateTime::currentDateTime());
      Akonadi::ItemModifyJob *mJob = new Akonadi::ItemModifyJob(m_tosaveInItem);
      mJob->disableRevisionCheck();
      mJob->start();
      kDebug() <<"item modify job started. bye bye";
    }
    else 
      kDebug() << "didnt enter the if";
    kDebug() << " going out of scope";
}

void HistoryLogger::modifyItem()
{
    kDebug() << "***Historylogger.cpp MODIFY ITEM()";

    bool itemOnlyModify=true;

    if (m_saveTimer)
        m_saveTimer->stop();
    QTime t;
    t.start(); //mesure the time needed to save.
//    m_t.start();
    kDebug() <<"time starts now" <<m_t.toString();
    if ( !m_parentCollection.isValid() )
    {
	if ( !m_kopeteChat.isValid() )
	{
	    m_kopeteChat = m_hPlugin->getCollection();
	    if ( !m_kopeteChat.isValid() ) 
	    {
		kWarning() << "resource is not configured, please configure";
		return;
	    }
	}
	
	QStringList mimeTypes;
	mimeTypes << "application/x-vnd.kde.kopetechathistory"<< "inode/directory";
	
	itemOnlyModify=false;
	Akonadi::Collection c;
	c.setParent(m_kopeteChat);
	c.setName( m_toSaveHistory.localContactId() );
	c.setContentMimeTypes(mimeTypes);
	
	Akonadi::CollectionCreateJob *createJob = new Akonadi::CollectionCreateJob(c);
	connect(createJob, SIGNAL(finished(KJob*)) , SLOT(parentCollCreated(KJob*)) ); 
	
	return;
    }
    
    //Create a new collection, as well as new item.
    if ( m_tosaveInItem.isValid() )
	kDebug() << "to save in tiem is valid";
    else
	kDebug() << "m_to save in item is invalid";
    
    if ( !m_tosaveInCollection.isValid() || m_tosaveInItem.modificationTime().toLocalTime().date().toString("MMyyyy") != QDateTime::currentDateTime().date().toString("MMyyyy"))
    {
	kDebug() << "inside some create part";
        itemOnlyModify=false;
        if (!m_tosaveInCollection.isValid() )
        {
            kDebug() << " collection doesnt exist. creating one";

	    QStringList mimeTypes;
            mimeTypes  << "application/x-vnd.kde.kopetechathistory"<< "inode/directory";

	    kDebug() << m_toSaveHistory.remoteContactId();
	    kDebug() << m_contact->account() << m_contact->contactId();
	    
            Akonadi::Collection collection;
            collection.setParent( m_parentCollection );	    
            collection.setName( m_toSaveHistory.remoteContactId() );
            collection.setContentMimeTypes( mimeTypes );

            Akonadi::CollectionCreateJob *jobcreatecoll = new Akonadi::CollectionCreateJob( collection  );
	    connect(jobcreatecoll,SIGNAL(result(KJob*)),SLOT(collectionCreateDone(KJob*)) );

	    return;
	}
	
	History history;
	if ( !m_tosaveInItem.hasPayload<History>() )
	{
	    kDebug() << "m_tosave in iten item is not of payload history returning";
	    return;
	}
	history = m_tosaveInItem.payload<History>();
	
        if ( history.date().toString("MMyyyy") != QDateTime::currentDateTime().date().toString("MMyyyy") )
	{
	    kDebug() <<"date time problem, creating new item";
	    //entering here means the item is not existing, so create one
//	    m_tosaveInItem = Akonadi::Item::Item();
	    m_tosaveInItem.setMimeType("application/x-vnd.kde.kopetechathistory" );
	    m_tosaveInItem.setModificationTime(QDateTime::currentDateTime());
	    m_tosaveInItem.setPayload<History>(m_toSaveHistory);
	    
	    Akonadi::ItemCreateJob *createitem = new Akonadi::ItemCreateJob( m_tosaveInItem, m_tosaveInCollection);
	    connect(createitem,SIGNAL(result(KJob*)), SLOT(itemCreateDone(KJob*)) );
	}
    }
    if ( itemOnlyModify)
    {
        if ( !m_tosaveInItem.isValid() ) kDebug() << "m_tosave in item is invalid";
        else kDebug() << " m_tosave in item is valid";
	
	m_tosaveInItem.setPayload< History>(m_toSaveHistory);
	m_tosaveInItem.setModificationTime( QDateTime::currentDateTime() );
	
	Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(m_tosaveInItem);
	modifyJob->disableRevisionCheck();
	connect(modifyJob,SIGNAL(result(KJob*)),SLOT(itemModifiedDone(KJob*)) );
	modifyJob->start();
    }
}

void HistoryLogger::parentCollCreated(KJob* job)
{
    if ( job->error() ) {
	kDebug() << "parent coll creation job failed" << job->errorString();
	return;
    }
    
    Akonadi::CollectionCreateJob *cJob = static_cast<Akonadi::CollectionCreateJob*>(job);
    Akonadi::Collection c = cJob->collection();
    m_parentCollection = c;
    
    QTimer *t = new QTimer(this);
    connect(t,SIGNAL(timeout()),SLOT(modifyItem()) );
    t->setSingleShot(true);
    t->start(2000);
    
}


void HistoryLogger::itemModifiedDone(KJob* job )
{
  Akonadi::ItemModifyJob *modifyjob = static_cast<Akonadi::ItemModifyJob*>(job);
  if (job->error() )
  {
    kDebug() <<"rev no="<<m_tosaveInItem.revision();
    kDebug() <<"item modification job failed"<<job->errorString();
  }
  else
  {
    kDebug() << "modified successfully\nold revision no="<<m_tosaveInItem.revision();
    m_tosaveInItem = modifyjob->item();
    m_saveTimerTime=qMin(m_t.elapsed()*1000, 60000);
    kDebug() << " saved in " << m_t.elapsed() << " ms "<<"\n"<<m_tosaveInItem.remoteId();
    kDebug() << m_saveTimer << m_saveTimerTime ;
    kDebug()<<"new revision no="<<m_tosaveInItem.revision();
    m_toSaveHistory = History::History();
  }
}


void HistoryLogger::collectionCreateDone(KJob *job)
{
  kDebug()<<"collectioncreate done slot:";
  Akonadi::CollectionCreateJob *createJob = static_cast<Akonadi::CollectionCreateJob*>(job);
  if (job->error() )
  {
    kDebug() << "create job failed"<<job->errorString();
    return;
  }
  else
  {
    m_tosaveInCollection= createJob->collection();
    if (m_tosaveInCollection.isValid() )
      kDebug() << "\n\n**collection Created successfully YOU NEVER KNOW"<<m_tosaveInCollection.remoteId();
    else kDebug() << "collection created is invalid";
    
    QTimer *t = new QTimer(this);
    connect(t,SIGNAL(timeout()),SLOT(createItem()) );
    t->setSingleShot(true);
    t->start(2000);
  }
}
void HistoryLogger::createItem()
{
    
    m_tosaveInItem.setMimeType("application/x-vnd.kde.kopetechathistory" );
    m_tosaveInItem.setModificationTime(QDateTime::currentDateTime());
    m_tosaveInItem.setPayload<History>(m_toSaveHistory);

    if (m_tosaveInItem.hasPayload<History>() )
    {
	  Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(m_tosaveInItem,m_tosaveInCollection);
	  connect(createJob,SIGNAL(result(KJob * )),this ,SLOT(itemCreateDone(KJob*)));
    }
}
void HistoryLogger::itemCreateDone(KJob* job)
{
  kDebug()<<"itemcreateDoneslot";
  Akonadi::ItemCreateJob *createjob = static_cast<Akonadi::ItemCreateJob*> (job);
  if (job->error() )
    kDebug() <<"item create failed";
  else
  {
    m_tosaveInItem = createjob->item();
    kDebug() << "Item modiCREATED successfully--or in progresss";
    m_saveTimerTime=qMin(m_t.elapsed()*1000, 20000);
    kDebug() << m_tosaveInCollection.name() << " saved in " << m_t.elapsed() << " ms "<<"\n"<<m_tosaveInItem.remoteId();
    
    Akonadi::ItemFetchJob *fetchjob=new Akonadi::ItemFetchJob(m_tosaveInItem);
    fetchjob->fetchScope().fetchFullPayload(History::HeaderPayload);
    connect(fetchjob,SIGNAL(itemsReceived(Akonadi::Item::List)),this,SLOT(itemsReceivedDone(Akonadi::Item::List)));
    fetchjob->start();
  }
}
void HistoryLogger::itemsReceivedDone(Akonadi::Item::List itemlist)
{
  kDebug() << "itemreceived slot";
  m_tosaveInItem = itemlist.first();
  kDebug() << m_tosaveInItem.remoteId();
}

//this function has been used in history dialog, to show messages by date.
//QList<Kopete::Message>
void HistoryLogger::readMessages(QDate date, int pos)
{
    kDebug() <<"hjisLOGger::readMessages(date)";
    m_readMessagesDate= date;
    m_pos=pos;
    
    QList<Kopete::Contact*> ct=m_metaContact->contacts();
    kDebug() << m_pos << ct.size() ;
    
    Kopete::Contact* contact;
    if ( m_pos <= ct.size() )
	contact = ct.at(m_pos);

    unsigned int month = QDate::currentDate().month() - date.month() + 12*(QDate::currentDate().year() - date.year());
    if ( month >= 0 )
    {
	kDebug() << month;
	connect( this, SIGNAL(getHistoryxDone()), SLOT(getHistoryJobDone())); 
	getHistory(contact, month );
    }
    
}

void HistoryLogger::getHistoryJobDone()
{

    disconnect( this, SIGNAL(getHistoryxDone()),this, SLOT(getHistoryJobDone()));     
    QList<Kopete::Contact*> ct = m_metaContact->contacts();
    
    History his= m_getHistory;
    m_getHistory = History::History();
    m_contact_history.insert(ct.at(m_pos), his);
    m_pos++;
    if ( m_pos < ct.size() )
	readMessages(m_readMessagesDate, m_pos);
    else
      transactionDone();
}

void HistoryLogger::transactionDone()
{
    kDebug() << "HistoryLogger::transactionDone(KJob *job)";
	QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibil
	QHashIterator<Kopete::Contact *, History> i(m_contact_history);
	kDebug() <<"before while llop";
	while (i.hasNext())
	{
	    i.next();
	    kDebug() << "entered while loop";
	    Kopete::Contact *contact = i.key();
	    History his = i.value();
	    kDebug() <<his.date();
	    foreach (const History::Message &msgElem2, his.messages() )
	    {
		rxTime.indexIn(msgElem2.timestamp().toString("d h:m:s") );
		QDateTime dt( QDate(m_readMessagesDate.year() , m_readMessagesDate.month() , rxTime.cap(1).toUInt()), QTime( rxTime.cap(2).toUInt() , rxTime.cap(3).toUInt(), rxTime.cap(5).toUInt()  ) );
		if (dt.date() != m_readMessagesDate)
		{
		    continue;
		}
		Kopete::Message::MessageDirection dir = (msgElem2.in() == "1") ?
                                                    Kopete::Message::Inbound : Kopete::Message::Outbound;

		if (!m_hideOutgoing || dir != Kopete::Message::Outbound)
		{ //parse only if we don't hide it

		    QString f=msgElem2.sender();
		    const Kopete::Contact *from=f.isNull()? 0L : contact->account()->contacts().value( f );

		    if (!from)
			from = (dir == Kopete::Message::Inbound) ? contact : contact->account()->myself();

		    Kopete::ContactPtrList to;
		    to.append( dir==Kopete::Message::Inbound ? contact->account()->myself() : contact );

		    Kopete::Message msg(from, to);
		
		    msg.setPlainBody( msgElem2.text() );
		    msg.setHtmlBody( QString::fromLatin1("<span title=\"%1\">%2</span>")
                                 .arg( dt.toString(Qt::LocalDate), msg.escapedBody() ));
		    msg.setTimestamp( dt );
		    msg.setDirection( dir );

		    m_readmessages.append(msg);
		}
	    }
	}
	qSort(m_readmessages.begin(), m_readmessages.end(), messageTimestampLessThan);
    emit readMessagesByDateDoneSignal(m_readmessages);
}


//is called every time a new instance of chat is initiated.
void HistoryLogger::readMessages(int lines,
                                 const Kopete::Contact *c, Sens sens, bool reverseOrder, bool colorize, 
				 bool quote)
{
    kDebug() << " " << "\n sens= " << sens;
    if ( sens == Chronological ) kDebug() << "sens is chronological";
    else kDebug() << "not chronological";//<<m_index<< " "<< m_indexPrev;    

    m_lines=lines;
    if( !m_readmessagesContact)
    {
	kDebug() << "\n\n\tSETTING THE VALUE OF READ MESSAGE CONTACT";
	m_readmessagesContact = c;
    }
    else
	kDebug() << "\n\n\t READ MESAGE CONTACT NOT SET";
    
    m_readmessagesSens = sens;
    m_reverseOrder = reverseOrder;
    m_colorize = colorize;

    if (!m_tosaveInCollection.isValid() )
    {
      kDebug() <<"readMessages, m_to save in coll is invalid";
    }
    else
      kDebug() << "\n\n\n\n\n\n The collection is Valid";
    
    if (!m_metaContact)
    { //this may happen if the contact has been moved, and the MC deleted
        if (c && c->metaContact())
            m_metaContact=c->metaContact();
        else
        {
            m_readmessages.clear();
            return;
        }
    }

    if (c && !m_metaContact->contacts().contains(const_cast<Kopete::Contact*>(c)) )
    {
        m_readmessages.clear();
        return;
    }

    if (sens == Default ) //if no sens are selected, just continue in the previous sens
        sens = m_oldSens ;
    
    if ( m_oldSens != Default && sens != m_oldSens )
    { //we changed our sens! so retrieve the old position to fly in the other way
//        m_currentElements= m_oldElements;
        m_currentMonth=m_oldMonth;
	kDebug() << "change in sense";
	timeLimit = m_timestamp;
    }
    else
    {
        m_oldMonth=m_currentMonth;
    }
    m_oldSens=sens;

    //i need both of these as private members
    m_currentContact=c;

    if (!c && m_metaContact->contacts().count()==1)
        m_currentContact=m_metaContact->contacts().first();

    else if (!c && m_metaContact->contacts().count()== 0)
    {
        if (!m_readmessages.empty())
            m_readmessages.clear();
        return;
    }
    if (!c && m_metaContact->contacts().count()>1)
        getHistoryForMetacontacts();
    else
        getHistoryForGivenContact();
    
    
}

void HistoryLogger::getHistoryForMetacontacts()
{
    kDebug() << "HistoryLogger::readMessagesForMetacontacts()";
    

    //TODO need to test this
    if ( (!m_readmessagesContact) && m_metaContact->contacts().count()>1)
    { //we have to merge the differents subcontact history
        QList<Kopete::Contact*> ct=m_metaContact->contacts();

	Akonadi::TransactionSequence *transaction = new Akonadi::TransactionSequence;
	kDebug() << "inside iftransaction set";
	QVariant vDate;
	vDate.setValue<QDate>(QDate::currentDate().addMonths(0-m_currentMonth));
	transaction->setProperty("date", vDate);
	
        foreach(Kopete::Contact *contact, ct)
        {
	    //TODO : this needs to be fixed here
	    
		if ( m_history.contains(contact) )
		    kDebug() << "m_history contains contact";
		else
		    kDebug() << "m_history dosent contain the contact"<<contact->contactId();
		
		QMap<unsigned int , History> monthHistory = m_history.value(contact);
		
		if ( monthHistory.isEmpty() ) kDebug() << "month hitory is mepty";
		else kDebug() << "month history is not empty";
		
		if (monthHistory.contains(m_currentMonth))
		{
		    History his;
		    his = monthHistory[m_currentMonth];
		    m_contact_history.insert(contact, his);
		    kDebug() << "inside if statement";
		  //TODO :if this dosent goto else statement, then where will this function lead
		  // to YOU IDIOT
		}
		else
		{
		    kDebug() << "dosent contain the m_current month"<<m_currentMonth;
		    Akonadi::Collection coll; 
		    coll = m_hPlugin->getCollection(contact->account()->accountId(), contact->contactId());
		    if ( coll.isValid() )
		    {
			QVariant vContact;
			vContact.setValue<Kopete::Contact *>(contact);
			
			//also add this to the transaction, as a parent.
			Akonadi::ItemFetchJob *f = new Akonadi::ItemFetchJob( coll , transaction );
			f->setProperty("contact", vContact);
			//for the moment i am keeping is fetch header payload
			f->fetchScope().fetchPayloadPart(History::HeaderPayload);
			connect(f, SIGNAL(finished(KJob*)), SLOT(itemFetchTestSlot(KJob*)) );
		    }
		}
	}// end of for loop for each cotact
	connect(transaction,SIGNAL(result(KJob*)),this,SLOT(transationsFetchItemsDone(KJob*)) ) ;
	transaction->start();
    }// end of if
    else
      kDebug() << "didnt enter the if statement :(";
}


void HistoryLogger::itemFetchTestSlot(KJob* job)
{
    kDebug() << " " ;
    if(job->error())
    {
	kDebug() << "failed" << job->errorString();
	return;
    }
    Akonadi::ItemFetchJob *f = static_cast<Akonadi::ItemFetchJob*>(job);
    
    QVariant vContact;
    vContact = job->property("contact");
    Kopete::Contact *c = vContact.value<Kopete::Contact*>();
    m_contactItemList.insert(c,f->items()) ;
    
}

void HistoryLogger::transationsFetchItemsDone(KJob* job)
{
    kDebug() << " " ;
    if ( job->error() )
    {
	kDebug() << " transaction failed" << job->errorString() ;
	return;
    }
    
    QHashIterator<const Kopete::Contact* , Akonadi::Item::List> i(m_contactItemList);
    QVariant vDate;
    vDate = job->property("date");
    QDate d = vDate.toDate();
    
    Akonadi::TransactionSequence *tr = new Akonadi::TransactionSequence ;
    connect(tr,SIGNAL(finished(KJob*)),SLOT(transaction_in_read_message_block_2_done(KJob*)) );
    //in here we fetch the item full payload. If it has already been fetched it should be in m_history,
    //so if its not in it, fetch it.
    while ( i.hasNext() )
    {	
	kDebug() << "inside while";
	i.next();
	Akonadi::Item::List items = i.value();
	const Kopete::Contact *contact = i.key();
	Kopete::Contact * c = const_cast<Kopete::Contact*>(contact);
	
	QMap<unsigned int, History> monthHistory;
	if (m_history.contains(contact) )
	    monthHistory= m_history.value(contact);
	
	if( monthHistory.contains(m_currentMonth) )
	{
	    History his = monthHistory.value(m_currentMonth);
	    Kopete::Contact *con = const_cast<Kopete::Contact*>(contact);
	    m_contact_history.insert(con, his);
	}
	else 
	{
	  foreach ( const Akonadi::Item &item , items )
	  {
	    if ( !item.hasPayload<History>() )
	    {
		kDebug() << " not of type payload history";
		continue;
	    }
	  
	    History his;
	    his = item.payload<History>();

	    if ( his.date().year() == d.year() && his.date().month() == d.month() )
	    {
		QVariant vContact;
		vContact.setValue<Kopete::Contact*>(c);
		
		Akonadi::ItemFetchJob *f = new Akonadi::ItemFetchJob( item , tr );
		f->fetchScope().fetchFullPayload();
		f->setProperty("contact" , vContact);
		connect(f,SIGNAL(finished(KJob*)),SLOT(fetchItemFullPayloadSlot(KJob*)) );
	    }
	  }
	}
    }    
}


void HistoryLogger::fetchItemFullPayloadSlot(KJob* job)
{
    kDebug() << " " ;
    if ( job->error() )
    {
	kDebug() << "fetch job failed ";
	return;
    }
    
    QVariant vContact, vDate;
    vContact = job->property("contact");
    Kopete::Contact *c = vContact.value<Kopete::Contact*>();
    
    Akonadi::ItemFetchJob * f = static_cast<Akonadi::ItemFetchJob*>(job);
    
    History history;
    if ( !f->items().first().hasPayload<History>() )
	return;
    
    history = f->items().first().payload<History>() ;
    QMap<unsigned int , History> monthHistory;
    monthHistory = m_history.value(c);
    monthHistory.insert( QDate::currentDate().month() - history.date().month() 
		      + (QDate::currentDate().year() - history.date().year()) *12 , history);
    m_history[c] = monthHistory;
    m_contact_history.insert(c, history);
    
//    if ( !m_tosaveInItem.isValid() );
//	m_tosaveInItem = f->items().first();
    

}

void HistoryLogger::transaction_in_read_message_block_2_done(KJob *job)
{
  kDebug () << " ";
  QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)

  QList<History::Message> messageList;
  if (job->error() )
  {
      kDebug() << "transaction_in_read_message_block 2 failed"<<job->errorString();
      return;
  }
  else  
  {
    QHashIterator<Kopete::Contact *, History> i(m_contact_history);
    while (i.hasNext())
    {
	    History his;
	    i.next();
	    Kopete::Contact *contact = i.key();
	    m_currentContact = contact;
	    his = i.value();
	    kDebug()<<"while";
	    kDebug() << his.remoteContactId() << his.messages().size();
	    
	    if( m_readmessagesHistory.localContactId().isEmpty() )
		m_readmessagesHistory.localContactId() = his.localContactId();
	    if ( m_readmessagesHistory.remoteContactId().isEmpty() )
		m_readmessagesHistory.remoteContactId() = his.remoteContactId();
	    if ( !m_readmessagesHistory.date().isValid() )
		m_readmessagesHistory.date() = his.date();
		
            if ( !his.messages().isEmpty() )
            {
                rxTime.indexIn( ((m_readmessagesSens== Chronological)? his.messages().first().timestamp().toString("d h:m:s")
                                 : his.messages().last().timestamp().toString("d h:m:s")) );
				 
                QDate d=QDate::currentDate().addMonths(0-m_currentMonth);
                QDateTime dt( QDate(d.year() , d.month() , rxTime.cap(1).toUInt()), QTime( rxTime.cap(2).toUInt() , rxTime.cap(3).toUInt(), rxTime.cap(5).toUInt()  ) );
 /*               if (!timestamp.isValid() || ((m_readmessagesSens==Chronological )? dt < timestamp : dt > timestamp) )
                {
                    history=his;
                    m_currentContact=contact;
                    timeLimit=timestamp;
                    timestamp=dt;
                }
                else if (!timeLimit.isValid() || ((m_readmessagesSens==Chronological) ? timeLimit > dt : timeLimit < dt) )
                {
                    timeLimit=dt;
                }
		*/
		foreach ( const History::Message &msg, his.messages() )
		{
		    messageList.append(msg);
		}
            }
    }
  }
    kDebug() << "calling qsort" << messageList.size();
    qSort(messageList.begin() , messageList.end(), func) ;
    kDebug() << "qsort done";
//  qSort(messageList.begin(), messageList.end(), messageTimestampLessThan);
    foreach (const History::Message &m, messageList)
	m_readmessagesHistory.addMessage(m);

  m_index=0;
  m_contact_history.clear();
  changeMonth();
}

void HistoryLogger::getHistoryForGivenContact()
{
    kDebug()<<"read messages block 3\n"<<"m_currentmonth="<<m_currentMonth;
/*    if (m_currentHistories.contains(m_currentContact)) //TODO need to take a look in this if
    {
	kDebug() <<"m_currentElementscontains";
        m_readmessagesHistory=m_currentHistories[m_currentContact];
	getHistoryDone();
    }
    else */
//    {
        connect(this,SIGNAL(getHistoryxDone()),SLOT(getHistoryDone()) );
        getHistory(m_currentContact,m_currentMonth);
//    }
}
void HistoryLogger::getHistoryDone()
{
    kDebug()<<"read messages block 31,signal disconnected"<<"m_currentmonth="<<m_currentMonth;
    disconnect(this,SIGNAL(getHistoryxDone()),this,SLOT(getHistoryDone()) );
//    index =0;// when we fetch the histories, at that time we need to set index=0;
    if ( !m_getHistory.messages().isEmpty())
    {
	kDebug() << "here you need this";
//        m_currentElements[m_currentContact]=m_getHistory;
	if ( m_getHistory.messages().isEmpty() )  
	    kDebug()<< " \n\n\t\tgethistory is empty";
	if ( !m_readmessagesHistory.messages().isEmpty() )
	{
	    foreach( const History::Message &msg , m_getHistory.messages() )
		m_readmessagesHistory.addMessage( msg );
	}
	else
	    m_readmessagesHistory= m_getHistory;
	
        m_getHistory=History::History();
    }
    changeMonth();

}

void HistoryLogger::changeMonth()
{
    //we don't find ANY messages in any contact for this month. so we change the month
    kDebug() << " " ;
    kDebug() << "m_currentmonth="<<m_currentMonth;
 //   kDebug() << " \n\n\t\t last month = " << getLastMonth(m_readmessagesContact);
    
    if (m_readmessagesHistory.messages().isEmpty() || m_index >= m_readmessagesHistory.messages().size() || m_index < 0)
    {
        if (m_index <0 ) kDebug() <<"inside  change month, m_index <0"<<m_index;//<<"prev ined="<<m_indexPrev;
	if ( m_readmessagesHistory.messages().isEmpty() ) kDebug() << " readmessagehistoryList is mepty";
	if ( m_index >= m_readmessagesHistory.messages().size()) kDebug() <<" index is >";
	
        if (m_readmessagesSens==Chronological)
        {
            kDebug() << " in if part----m_currentmonth="<<m_currentMonth;
            if (m_currentMonth <= 0)
	    {
		kDebug() << "returning";
		emit readMessagesDoneSignal(m_readmessages);
		m_readmessages.clear();
		m_readmessagesHistory = History::History();
                return;
            }
	    kDebug() << "set current month";
            setCurrentMonth(m_currentMonth-1);
        }
        else
        {
	    kDebug() << " in else part of change month";	    
	    if (m_currentMonth >= getFirstMonth(m_readmessagesContact) )
	    {
		kDebug() << "current month="<<m_currentMonth;
		kDebug() << " current month >= first month returning ";
		emit readMessagesDoneSignal(m_readmessages);
		m_readmessages.clear();
		m_readmessagesHistory = History::History();
		return;
	    }
	    setCurrentMonth(m_currentMonth+1);
        }
	
	if (!m_readmessagesContact && m_metaContact->contacts().count()>1)
	{
	    getHistoryForMetacontacts();
	}
	else
	    getHistoryForGivenContact();
    }
    else readMessagesDone();
}

void HistoryLogger::readMessagesDone()
{
    kDebug()<<"The messages in m_readmessage history are ";
    foreach ( const History::Message &msg , m_readmessagesHistory.messages() )
	kDebug() << msg.text() <<"\t"<<msg.timestamp().toString("d h:m:s");
    
    History::Message msgElem;
    QDateTime timestamp;
    const Kopete::Contact * currentContact = m_currentContact;
    QColor fgColor = HistoryConfig::history_color();
    QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)

    m_timestamp = QDateTime::QDateTime();
/*    if ( m_oldSens != m_readmessagesSens )
    {
	kDebug() << "change in sens "<< m_timestamp;
	timeLimit = m_timestamp;
	m_timestamp = QDateTime::QDateTime();
    }
    else
      kDebug() << "\n no changes in sens"<<m_oldSens << m_readmessagesSens;
*/    
 //   if  (m_index == 0 )
        (m_readmessagesSens!=Chronological)? m_index = m_readmessagesHistory.messages().count() -1 : m_index=0 ;

    if ( m_readmessagesSens == Chronological ) kDebug() << " chronological oreder, index ="<<m_index;
    else kDebug() << "not choronological, m_index="<<m_index;
    
    kDebug() << "NO_OF_LINES="<<m_lines;
    
    for ( ; (m_readmessagesSens!=Chronological)? m_index>=0: m_index < m_readmessagesHistory.messages().count()  ;
            (m_readmessagesSens!=Chronological)? m_index-- : m_index++ )
    {	kDebug() << "start of for loop m_index="<<m_index;
        msgElem = m_readmessagesHistory.messages().at(m_index);
	
	 QDate dx=QDate::currentDate().addMonths(0-m_currentMonth);
	 timestamp=QDateTime( QDate(dx.year() , dx.month() , msgElem.timestamp().date().day() ), 
				    QTime (
						msgElem.timestamp().time().hour() ,
						msgElem.timestamp().time().minute(), 
						msgElem.timestamp().time().second() 
					  )
				   ) ;
	if (
            (m_readmessages.size() < m_lines) &&
            !m_readmessagesHistory.messages().isEmpty() &&
            (!timestamp.isValid() || !timeLimit.isValid() ||
             ((m_readmessagesSens==Chronological) ? timestamp >= timeLimit : timestamp <= timeLimit) )
	    )
        {
            // break this loop, if we have reached the correct number of messages,
            // if there are no more messages for this contact, or if we reached
            // the timeLimit msgElem is the next message, still not parsed, so
            // we parse it now
//	    kDebug() << "inside if part";
//	    kDebug() << "\n\ttimestamp"<<timestamp<<"\n\ttimelimit="<<timeLimit;
	    if (!timestamp.isValid() ) kDebug() << "timestamp invalid";
	    if (!timeLimit.isValid() ) kDebug() << "timeLimit invalid";
	    if (timestamp<=timeLimit ) kDebug() << "timestamp is <= timelimit\ntimestamp="<<timestamp <<" timeLimit="<<timeLimit;
	    else kDebug() << "timestamp > timeLimit \ntimestamp="<<timestamp <<" timeLimit="<<timeLimit;
	    
            Kopete::Message::MessageDirection dir = (msgElem.in() == "1") ?
                                                    Kopete::Message::Inbound : Kopete::Message::Outbound;

            if (!m_hideOutgoing || dir != Kopete::Message::Outbound)
            { //parse only if we don't hide it
//		kDebug() << " inside outbound if part";
                if ( m_filter.isNull() || ( m_filterRegExp? msgElem.text().contains(QRegExp(m_filter,m_filterCaseSensitive)) : msgElem.text().contains(m_filter,m_filterCaseSensitive) ))
                {
                    Q_ASSERT(currentContact);
                    QString f=msgElem.sender();
                    const Kopete::Contact *from=f.isNull() ? 0L : currentContact->account()->contacts().value(f);

                    if ( !from )
                        from = (dir == Kopete::Message::Inbound) ? currentContact : currentContact->account()->myself();

                    Kopete::ContactPtrList to;
                    to.append( dir==Kopete::Message::Inbound ? currentContact->account()->myself() : const_cast<Kopete::Contact*>(currentContact) );

                    //parse timestamp only if it was not already parsed
                    rxTime.indexIn(msgElem.timestamp().toString("d h:m:s"));
                    QDate d=QDate::currentDate().addMonths(0-m_currentMonth);
                    timestamp=QDateTime( QDate(d.year() , d.month() , rxTime.cap(1).toUInt()), QTime( rxTime.cap(2).toUInt() , rxTime.cap(3).toUInt() , rxTime.cap(5).toUInt() ) );

                    Kopete::Message msg(from, to);
                    msg.setTimestamp( timestamp );
                    msg.setDirection( dir );
                    msg.setPlainBody( msgElem.text() );
                    if (m_colorize)
                    {
                        msg.setHtmlBody( QString::fromLatin1("<span style=\"color:%1\" title=\"%2\">%3</span>")
                                         .arg( fgColor.name(), timestamp.toString(Qt::LocalDate), msg.escapedBody() ));
                        msg.setForegroundColor( fgColor );
                        msg.addClass( "history" );
                    }
                    else
                    {
                        msg.setHtmlBody( QString::fromLatin1("<span title=\"%1\">%2</span>")
                                         .arg( timestamp.toString(Qt::LocalDate), msg.escapedBody() ));
                    }

		    kDebug() << msg.plainBody();
		    timeLimit = timestamp;
		    if ( !m_timestamp.isValid() )
		    {
			kDebug() << "m_timestamo invalid";
			m_timestamp = timestamp;
		    }
                    if (m_reverseOrder)
		    {	kDebug() << "reverse order";
                        m_readmessages.prepend(msg);
		    }
                    else
		    {kDebug() << "not reverse order";
                        m_readmessages.append(msg);
		    }
                }
            }
        }
	else 
	{
	    kDebug() << "\n\n\nDIDNT ENTER THE IF STATEMENT, SO NO MESSAGE ATTACHED";
	    kDebug() << m_readmessages.size() ;
	    kDebug() << "timestamp="<<timestamp;
	    kDebug() << "timelimit="<<timeLimit;
	}
	if ( !(m_readmessages.count() < m_lines) )
	{
	    kDebug() << "\n\n\t\tbreaking";
	    break;
	}
	
    } kDebug() << " end of for loop";
    kDebug() << "after end of for loop, timestam, timelimit"<<timestamp <<timeLimit;
    
    if (m_readmessages.count() < m_lines)
    {
	m_readmessagesHistory = History::History();
	changeMonth();
    }
      
      kDebug() << "m_index="<<m_index;
    if ( m_readmessages.count() >= m_lines )
    {
	kDebug() << " emitting";
	emit readMessagesDoneSignal(m_readmessages);
	m_readmessages.clear();
	m_readmessagesHistory = History::History();
    }
}


unsigned int HistoryLogger::getFirstMonth(const Kopete::Contact *c)
{
    kDebug() << "**Historylogger.cpp WITH contact" ;
    kDebug() << "cached month= " << m_cachedMonth;
    
    if (!c)
    {	
	kDebug() << "no contat specified, calling getfirstmonth()";
        return getFirstMonth();
    }
    
    History history;
    Kopete::Contact *con = const_cast<Kopete::Contact*>(c);
    
    if ( !m_contactItemList.contains(con) )
    {
	kDebug() << "dosent contain the contact";
	return 0;
    }
    
    
    if ( m_contactItemList.value(con).first().hasPayload<History>() )
	history = m_contactItemList.value(con).first().payload<History>();
    
    foreach(const Akonadi::Item &i, m_contactItemList.value(con))
    {
	
	if (!i.hasPayload<History>())
	{
	    kDebug() << "not of payload type history";
	    continue;
	}
      
	History his;
	his = i.payload<History>();
	if ( his.date() <= history.date()  )
	    history = his;
    }
    
    int result = 12*(QDate::currentDate().year() - history.date().year()) + QDate::currentDate().month() - history.date().month();
    kDebug() <<"result="<<result;
    if (result < 0) 
	kWarning(14310) << "Kopete only found log file made in the future. Check your date!";
    return result;

}

//TODO : this method is for meta contact. so change it to adjust to meta contact
unsigned int HistoryLogger::getFirstMonth()
{
    kDebug() << "WITHOUT ontact" << m_cachedMonth;
    if (m_cachedMonth!=-1)
    {
	kDebug() << m_cachedMonth << "returninh";
        return m_cachedMonth;
    }

    if (!m_metaContact)
        return 0;

    int m=0;
    QList<Kopete::Contact*> contacts=m_metaContact->contacts();

    kDebug() << "before foreach loop";
    foreach(Kopete::Contact* contact, contacts)
    {
	kDebug() << "for contact="<<contact->contactId();
        int m2=getFirstMonth(contact);
        if (m2>m) m=m2;
    }
    m_cachedMonth=m;
    kDebug() << "\n before returning m=" << m;
    return m;
}

void HistoryLogger::setHideOutgoing(bool b)
{
    m_hideOutgoing = b;
}

void HistoryLogger::slotMCDeleted()
{
    m_metaContact = 0;
}

void HistoryLogger::setFilter(const QString& filter, bool caseSensitive , bool isRegExp)
{
    m_filter=filter;
    m_filterCaseSensitive=caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    m_filterRegExp=isRegExp;
}

QString HistoryLogger::filter() const
{
    return m_filter;
}

bool HistoryLogger::filterCaseSensitive() const
{
    return (m_filterCaseSensitive == Qt::CaseSensitive);
}

bool HistoryLogger::filterRegExp() const
{
    return m_filterRegExp;
}

void HistoryLogger::getDaysForMonth(QDate date, int pos)
{
    kDebug() << " "<<date;
    m_dateGetDaysForMonth = date;
    m_pos_GetDaysForMonth=pos;
    
    QList<Kopete::Contact*> contacts = m_metaContact->contacts();
    unsigned int month = QDate::currentDate().month() - date.month() + 12*(QDate::currentDate().year() - date.year() );
    if ( month >= 0 )
    {
	connect(this, SIGNAL(getHistoryxDone()), SLOT(getDaysForMonthSlot()) ) ;
	getHistory(contacts.at(m_pos_GetDaysForMonth), month);
    }
}

void HistoryLogger::getDaysForMonthSlot()
{

    disconnect(this, SIGNAL(getHistoryxDone()), this, SLOT(getDaysForMonthSlot()) ) ;  
    int lastDay=0;
   
    History his = m_getHistory;
    m_getHistory = History::History();
    
      foreach(const History::Message &msg, his.messages() )
      {
	int day=msg.timestamp().date().day();
	if ( day != lastDay )
	{
	  if ( !m_dayList.contains(day) )
	      m_dayList.append(day);
	  lastDay=day;
	}
    }
    m_pos_GetDaysForMonth++;
    if ( m_pos_GetDaysForMonth < m_metaContact->contacts().size() )
    {
	getDaysForMonth(m_dateGetDaysForMonth, m_pos_GetDaysForMonth);
    }
    else
    {
	emit getDaysForMonthSignal(m_dayList);
	kDebug() << m_dayList.size();
	m_dayList.clear();
    }
}

#include "historylogger.moc"
