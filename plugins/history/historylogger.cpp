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

#include "gethistoryjob.h"
#include "historylogger.h"
#include "historyxmlio.h"
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

// -----------------------------------------------------------------------------
HistoryLogger::HistoryLogger( Kopete::MetaContact *m,  QObject *parent )
        : QObject(parent)
{
    m_hideOutgoing=false;
    m_filterCaseSensitive=Qt::CaseSensitive;
    m_filterRegExp=false;
    m_saveTimer=0L;
    m_saveTimerTime=0;
    m_realMonth=QDate::currentDate().month();
    m_metaContact=m;

    m_cachedMonth=-1;
    m_oldSens=Default;

    //calling a function that initializes the qmap between collection name and colletion
    mapContactCollection();

//     connect(this, SIGNAL(getHistoryxDone()), SLOT(appendMessage2()) );

    //the contact may be destroyed, for example, if the contact changes its metacontact
    connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

    setPositionToLast();
}


HistoryLogger::HistoryLogger( Kopete::Contact *c,  QObject *parent )
        : QObject(parent)
{
    m_saveTimer=0L;
    m_saveTimerTime=0;
    m_cachedMonth=-1;
    m_metaContact=c->metaContact();
    m_hideOutgoing=false;
    m_realMonth=QDate::currentDate().month();
    m_oldSens=Default;
    m_filterCaseSensitive=Qt::CaseSensitive;
    m_filterRegExp=false;

    //calling a function that initializes the qmap between collection name and colletion
    if (!m_baseCollection.isValid() || m_collectionMap.isEmpty() )
        mapContactCollection();

//    connect(this, SIGNAL(getHistoryxDone()), SLOT(appendMessage2()) );

    //the contact may be destroyed, for example, if the contact changes its metacontact
    connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

    setPositionToLast();
}


HistoryLogger::~HistoryLogger()
{
    kDebug() <<"DISTRUCTOR";
    if (m_saveTimer && m_saveTimer->isActive())
        modifyItem();
    kDebug() << " going out of scope";
}

QMap<QString, Akonadi::Collection > HistoryLogger::m_collectionMap;

Akonadi::Collection HistoryLogger::m_baseCollection;

void HistoryLogger::mapContactCollection()
{
    Akonadi::Collection::List collectionList;
    Akonadi::Collection::Id baseId;
    if ( !m_baseCollection.isValid() )
    {
        kDebug() << "m_base collection is invalid so getting base collection";
        Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive );
        if ( job->exec()  )
        {
            collectionList = job->collections();
            foreach( const Akonadi::Collection collection, collectionList )
            {
                if ( collection.name() == "kopeteChat" )
                {
                    m_baseCollection = collection;
                    baseId = collection.id();
                }
            }
        } else kDebug() << "collection fetch job not executed";
    } else kDebug() << "root collection is lareadythere";

    if ( HistoryLogger::m_collectionMap.isEmpty() )
    {
        kDebug() <<"m_collection map is empty";
        foreach( const Akonadi::Collection collection, collectionList )
        {
            if (collection.parent() == baseId )
            {
                HistoryLogger::m_collectionMap.insert(collection.name() , collection);
                kDebug() <<collection.name();
            }
        }
    }
}

void HistoryLogger::setPositionToLast()
{
    setCurrentMonth(0);
    m_oldSens = AntiChronological;
    m_oldMonth=0;
    m_oldElements.clear();
}


void HistoryLogger::setPositionToFirst()
{
    setCurrentMonth( getFirstMonth() );
    m_oldSens = Chronological;
    m_oldMonth=m_currentMonth;
    m_oldElements.clear();
}


void HistoryLogger::setCurrentMonth(int month)
{
    m_currentMonth = month;
    m_currentElements.clear();
}

History HistoryLogger::getHistory(const Kopete::Contact *c, unsigned int month , bool canLoad , bool* contain)
{
    return History::History();
}

void HistoryLogger::getHistoryx(const Kopete::Contact *c, unsigned int month , bool canLoad , bool* contain)
{
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
        if (contain)
            *contain=false;
        m_getHistory = History::History();
        kDebug() << "GENERATING SIGNAL";
        emit getHistoryxDone();
        return;
    }

    QMap<unsigned int , History> monthHistory = m_history[c];
    if (monthHistory.contains(month))
    {
        m_getHistory= monthHistory[month];
        kDebug() << "GENERATING SIGNAL";
        emit getHistoryxDone();
        return;
    }

    Kopete::Contact *con = const_cast<Kopete::Contact*>(c);

    Akonadi::Collection coll;

    coll=m_collectionMap[con->contactId()];

    GetHistoryJob *getjob= new GetHistoryJob(coll,QDate::currentDate().addMonths(0-month));
    connect(getjob, SIGNAL(result(KJob*)), SLOT(emperimentalSlot(KJob*)));  //
//   getjob->setProperty(month);
    getjob->start();

//    History his =  getHistory(c, QDate::currentDate().addMonths(0-month), canLoad, contain);

//    monthHistory.insert(month, his);
//    m_history[c]=monthHistory;

//    return his;

}
void HistoryLogger::emperimentalSlot(KJob* job )
{
    GetHistoryJob *xyz = static_cast<GetHistoryJob*>(job);
    kDebug() <<"\n******\n***********\n***********HUHOAHAHAHA";
    if (job->error() )
        kDebug()<<"gethistoryjobfailed";
    else {
        m_getHistory = xyz->returnHistory();
        QMap<unsigned int , History> monthHistory= m_history[m_contact];
        monthHistory.insert(m_month,m_getHistory);
        m_history[m_contact] = monthHistory;
        m_tosaveInItem = xyz->returnItem();
        kDebug() <<"GENERATING SIGNAL";
        emit getHistoryxDone();
    }


}


History HistoryLogger::getHistory(const Kopete::Contact *contact, const QDate date , bool canLoad , bool* contain)
{
    kDebug() << "GET HISTORY";
    bool itemfound=false;

    Kopete::Contact *c = const_cast<Kopete::Contact*>(contact);
    if (!m_metaContact)
    { //this may happen if the contact has been moved, and the MC deleted
        if (c && c->metaContact())
            m_metaContact=c->metaContact();
        else
            return History();
    }

    if (!m_metaContact->contacts().contains(c))
    {
        if (contain)
            *contain=false;
        return History();
    }

    if (!canLoad)
    {
        if (contain)
            *contain=false;
        return History();
    }

    if ( HistoryLogger::m_collectionMap.contains( c->contactId() ) )
    {
        m_tosaveInCollection = m_collectionMap[c->contactId()];
        kDebug() << "collection found in m_collection map";
    }
    else
    {
        kDebug() << " making m_tosave in collection null";
        m_tosaveInCollection = Akonadi::Collection::Collection();
    }

    History history;
    Akonadi::Item itemx;
    if ( m_tosaveInCollection.isValid() && m_toSaveHistory.messages().isEmpty())
    {
        kDebug() <<"m_tosave in collection is valid";
        Akonadi::ItemFetchJob *itemjob = new Akonadi::ItemFetchJob( m_tosaveInCollection );
        itemjob->fetchScope().fetchFullPayload();

        if ( itemjob->exec() )
        {
            Akonadi::Item::List items = itemjob->items();
            foreach( const Akonadi::Item &item, items )
            {
                if ( item.modificationTime().toLocalTime().toString("MMyyyy")== date.toString("MMyyyy") )
                {
                    itemx = item ;
                    itemfound=true;
                    kDebug() << " found item with matching time";
                    break;
                }
            }
            m_tosaveInItem=itemx;

            if (itemx.hasPayload<History>() )
                history = itemx.payload< History >();
        }
        else kDebug() << " item fetch job failed";
    }  else kDebug() << "invalid collection, so no item ";

    if ( itemfound )
    {
        kDebug() << "item has been found";
        if ( !m_tosaveInItem.hasPayload() )
        {
            if (contain)
                *contain=false;
            return history;
        }
    }
    else
    {
        kDebug() << " since item was not found, making m_tosave in item null";
        m_tosaveInItem = Akonadi::Item::Item(); //the item now point to a null collection
    }

    return history;
}


void HistoryLogger::appendMessage( const Kopete::Message &msg , const Kopete::Contact *ct )
{
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
    getHistoryx(c, QDate::currentDate().month() - date.month() - (QDate::currentDate().year() - date.year()) * 12);


    //    History history = getHistory(c, QDate::currentDate().month() - date.month() - (QDate::currentDate().year() - date.year()) * 12);
//     kDebug()<<"\n*nothin important**\n the date part="<<(QDate::currentDate().month() - date.month() - (QDate::currentDate().year() - date.year()) * 12);
}
void HistoryLogger::appendMessage2()
{
    disconnect(this, SIGNAL(getHistoryxDone()),this, SLOT(appendMessage2()));
    kDebug() <<"\n\n\nAPPENDmessage@22";
    History history;
    if (!m_getHistory.messages().isEmpty() )
        history = m_getHistory;

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
    monthHistory.insert(QDate::currentDate().month() - history.date().month() - (QDate::currentDate().year() - history.date().year()) * 12, history);
    m_history[m_contact]=monthHistory;

    // I'm temporizing the save.
    //same logic reimplemented
    if ( history.date().month() != QDate::currentDate().month() )
    {
        kDebug()<<" ***\n\n\nof contact change or month change called:";
        modifyItem();
    }

    m_toSaveHistory=history;

    if (!m_saveTimer)
    {
        m_saveTimer=new QTimer(this);
        connect( m_saveTimer, SIGNAL( timeout() ) , this, SLOT(modifyItem()) );
    }
    if (!m_saveTimer->isActive())
    {
        m_saveTimer->setSingleShot( true );
        m_saveTimer->start( m_saveTimerTime );
    }
    kDebug() <<"emitting appendMessageDoneSignal";
    emit appendMessageDoneSignal();
}

void HistoryLogger::ModifyItem(HistoryLogger *historylogger)
{
  if (m_saveTimer)
    m_saveTimer->stop();
  historylogger->modifyItem();
 connect(this,SIGNAL(itemModifiedSignal(HistoryLogger*)),SLOT(deleteHistoryInstance(HistoryLogger*)));
}

void HistoryLogger::deleteHistoryInstance(HistoryLogger* his)
{
  his->deleteLater();
  this->deleteLater();
}

void HistoryLogger::modifyItem()
{
    kDebug() << "***Historylogger.cpp MODIFY ITEM()";

    bool itemOnlyModify=true;
//    bool itemCreate = false;

    if (m_saveTimer)
        m_saveTimer->stop();
    QTime t;
    t.start(); //mesure the time needed to save.
    m_t.start();

    kDebug() << m_baseCollection.remoteId();
    if (m_collectionMap.contains(m_contact->contactId()))
        m_tosaveInCollection = m_collectionMap[m_contact->contactId()];

    //Create a new collection, as well as new item.
    if ( !m_tosaveInCollection.isValid() || m_tosaveInItem.modificationTime().toLocalTime().date().toString("MMyyyy") != QDateTime::currentDateTime().date().toString("MMyyyy"))
    {
        itemOnlyModify=false;
        if (!m_tosaveInCollection.isValid() )
        {
            kDebug() << " collection doesnt exist. creating one";
            Akonadi::Collection collection;
            collection.setParent( m_baseCollection );
            collection.setName( m_toSaveHistory.remoteContactId() );
            QStringList mimeTypes;
            mimeTypes  << "application/x-vnd.kde.kopetechathistory"<< "inode/directory";;
            collection.setContentMimeTypes( mimeTypes );

            Akonadi::CollectionCreateJob *jobcreatecoll = new Akonadi::CollectionCreateJob( collection  );
	    connect(jobcreatecoll,SIGNAL(result(KJob*)),SLOT(collectionCreateDone(KJob*)) );
	    jobcreatecoll->start();
	    return;
	}

        if ( m_tosaveInItem.modificationTime().toLocalTime().date().toString("MMyyyy") != QDateTime::currentDateTime().date().toString("MMyyyy") )
            kDebug() <<"date time problem";
        //entering here means the item is not existing, so create one
        kDebug() << " before creating item";
        if (!m_tosaveInCollection.isValid() ) kDebug() << "m_tosave in colllecion is invalid";

//	  Akonadi::Item itemToCreate;
        m_tosaveInItem.setMimeType("application/x-vnd.kde.kopetechathistory" );
        m_tosaveInItem.setModificationTime(QDateTime::currentDateTime());
        m_tosaveInItem.setPayload<History>(m_toSaveHistory);

        Akonadi::ItemCreateJob *createitem = new Akonadi::ItemCreateJob( m_tosaveInItem, m_tosaveInCollection);
	connect(createitem,SIGNAL(result(KJob*)), SLOT(itemCreateDone(KJob*)) );
	createitem->start();
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
    m_saveTimerTime=qMin(m_t.elapsed()*1000, 300000);
    kDebug() << " saved in " << m_t.elapsed() << " ms "<<"\n"<<m_tosaveInItem.remoteId();
    kDebug()<<"new revision no="<<m_tosaveInItem.revision();
    m_toSaveHistory = History::History();
//    emit itemModifiedSignal();

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
    m_collectionMap.insert(m_tosaveInCollection.name() , m_tosaveInCollection);
    kDebug() << "\n\n**collection Created successfully YOU NEVER KNOW"<<m_tosaveInCollection.remoteId();

    m_tosaveInItem.setMimeType("application/x-vnd.kde.kopetechathistory" );
    m_tosaveInItem.setModificationTime(QDateTime::currentDateTime());
    m_tosaveInItem.setPayload<History>(m_toSaveHistory);

    Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(m_tosaveInItem,m_tosaveInCollection);
    connect(createJob,SIGNAL(result(KJob * )),this ,SLOT(itemCreateDone(KJob*)));
    createJob->start();
    
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
    m_saveTimerTime=qMin(m_t.elapsed()*1000, 300000);
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
QList<Kopete::Message> HistoryLogger::readMessages(QDate date)
{
    QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)
    QList<Kopete::Message> messages;
    QList<Kopete::Contact*> ct=m_metaContact->contacts();

    foreach(Kopete::Contact* contact, ct)
    {
        History his=getHistory(contact, date, true, 0L);

        foreach ( const History::Message &msgElem2, his.messages() )
        {

            rxTime.indexIn(msgElem2.timestamp().toString("d h:m:s") );
            QDateTime dt( QDate(date.year() , date.month() , rxTime.cap(1).toUInt()), QTime( rxTime.cap(2).toUInt() , rxTime.cap(3).toUInt(), rxTime.cap(5).toUInt()  ) );

            if (dt.date() != date)
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

                messages.append(msg);
            }
        }
        // end while on messages

    }

    qSort(messages.begin(), messages.end(), messageTimestampLessThan);
    return messages;
}

QList< Kopete::Message > HistoryLogger::retrunReadMessages()
{
  return m_readmessages;
}


//is called every time a new instance of chat is initiated.
void HistoryLogger::readMessages(int lines,
                                 const Kopete::Contact *c, Sens sens, bool reverseOrder, bool colorize)
{
    kDebug() << "readMessages2 (Kopete::Contact *c, Sens sens, bool reverseOrder, bool colorize)";
//    QList<Kopete::Message> messages;

    m_lines=lines;
    m_readmessagesContact = c;
    m_readmessagesSens = sens;
    m_reverseOrder = reverseOrder;
    m_colorize = colorize;

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
        m_currentElements= m_oldElements;
        m_currentMonth=m_oldMonth;
    }
    else
    {
        m_oldElements=m_currentElements;
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
    //TODO this is the end of first block.
    if (!c && m_metaContact->contacts().count()>1)
        readmessagesBlock2();
    else
        readmessagesBlock3();
}

void HistoryLogger::readmessagesBlock2()
{
    // A regexp useful for this function
    QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)

    History history;
    //enter this only when no of meta contacts is greaterthan one. TODO need to check this
    if ( (!m_readmessagesContact) && m_metaContact->contacts().count()>1)
    { //we have to merge the differents subcontact history
        QList<Kopete::Contact*> ct=m_metaContact->contacts();

        foreach(Kopete::Contact *contact, ct)
        {
            History his;
            if (m_currentElements.contains(contact))
                his=m_currentElements[contact];
            else  //there is not yet "next message" register, so we will take the first  (for the current month)
            {
                his=getHistory(contact,m_currentMonth);
            }
            if ( !his.messages().isEmpty() )
            {
                rxTime.indexIn( ((m_readmessagesSens== Chronological)? his.messages().first().timestamp().toString("d h:m:s")
                                 : his.messages().last().timestamp().toString("d h:m:s")) );
                QDate d=QDate::currentDate().addMonths(0-m_currentMonth);
                QDateTime dt( QDate(d.year() , d.month() , rxTime.cap(1).toUInt()), QTime( rxTime.cap(2).toUInt() , rxTime.cap(3).toUInt(), rxTime.cap(5).toUInt()  ) );
                if (!timestamp.isValid() || ((m_readmessagesSens==Chronological )? dt < timestamp : dt > timestamp) )
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

                break;
            }
        }
    }
}

void HistoryLogger::readmessagesBlock3()
{
    kDebug()<<"read messages block 3\n"<<"m_currentmonth="<<m_currentMonth;
    if (m_currentElements.contains(m_currentContact)) //TODO need to take a look in this if
        m_readmessagesHistory=m_currentElements[m_currentContact];
    else
    {
        connect(this,SIGNAL(getHistoryxDone()),SLOT(readmessagesBlock31()) );
        getHistoryx(m_currentContact,m_currentMonth);
    }
}
void HistoryLogger::readmessagesBlock31()
{
    kDebug()<<"read messages block 31,signal disconnected"<<"m_currentmonth="<<m_currentMonth;
    disconnect(this,SIGNAL(getHistoryxDone()),this,SLOT(readmessagesBlock31()) );
    index =0;
    if ( !m_getHistory.messages().isEmpty())
    {
        m_currentElements[m_currentContact]=m_getHistory;
        m_readmessagesHistory=m_getHistory;
        m_getHistory=History::History();
    }
    readMessagesBlock4();

}

void HistoryLogger::readMessagesBlock4()
{
    //we don't find ANY messages in any contact for this month. so we change the month
    kDebug() << "readMessages block 4"<<"m_currentmonth="<<m_currentMonth;
    if (m_readmessagesHistory.messages().isEmpty() || index >= m_readmessagesHistory.messages().count() || index < 0)
    {
        if (index <0 ) kDebug() <<"inside  change month, index <0";
	
        if (m_readmessagesSens==Chronological)
        {
            kDebug() << "m_currentmonth="<<m_currentMonth;
            if (m_currentMonth <= 0)
            {
                //    break;//replacing break with return
                return;
            }
            setCurrentMonth(m_currentMonth-1);
	    if (!m_readmessagesContact && m_metaContact->contacts().count()>1)
	      readmessagesBlock2();
	    else
	      readmessagesBlock3();
        }
        else
        {
	    getFirstMonth(m_readmessagesContact);	    
        }
//        continue; //begin the loop from the bottom, and find currentContact and timeLimit again
    }
    else readMessagesBlock5();
}
void HistoryLogger::readMessagesBlock41()
{
  kDebug()<<"readmessages block 41\nsignal disconnected";
  disconnect(this,SIGNAL(getfirstmonthwithcontactDoneSignal()),this,SLOT(readMessagesBlock41()));
  if (m_currentMonth >= m_result)
  {
    kDebug() << "returning";
    return;
  }
  setCurrentMonth(m_currentMonth+1);
  
  if (!m_readmessagesContact && m_metaContact->contacts().count()>1)
    readmessagesBlock2();
  else
    readmessagesBlock3();
}
void HistoryLogger::readMessagesBlock5()
{	
    kDebug()<<"read messages block 5";
    History::Message msgElem;
    const Kopete::Contact * currentContact = m_currentContact;
    //getting the color for messages:
    QColor fgColor = HistoryConfig::history_color();
    QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)

    kDebug() << "index="<<index;

    if  (index == 0 )
        (m_readmessagesSens!=Chronological)? index = m_readmessagesHistory.messages().count() -1 : index=0 ;

    for ( ; (m_readmessagesSens==Chronological)? index>=0: index < m_readmessagesHistory.messages().count()  ;
            (m_readmessagesSens=Chronological)? index-- : index++ )
    {
        msgElem = m_readmessagesHistory.messages().at(index);
        if ( !(m_readmessages.count() < m_lines) ) break;

        if (
            (m_readmessages.count() < m_lines) &&
            !m_readmessagesHistory.messages().isEmpty() &&
            (!timestamp.isValid() || !timeLimit.isValid() ||
             ((m_readmessagesSens==Chronological) ? timestamp <= timeLimit : timestamp >= timeLimit)
            ))
        {
            // break this loop, if we have reached the correct number of messages,
            // if there are no more messages for this contact, or if we reached
            // the timeLimit msgElem is the next message, still not parsed, so
            // we parse it now

            Kopete::Message::MessageDirection dir = (msgElem.in() == "1") ?
                                                    Kopete::Message::Inbound : Kopete::Message::Outbound;

            if (!m_hideOutgoing || dir != Kopete::Message::Outbound)
            { //parse only if we don't hide it
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

                    if (m_reverseOrder)
                        m_readmessages.prepend(msg);
                    else
                        m_readmessages.append(msg);
                }
            }


            if (!m_readmessagesContact && (m_metaContact->contacts().count() > 1))
            {
                // In case of hideoutgoing messages, it is faster to do
                // this, so we don't parse the date if it is not needed
                QRegExp rx("(\\d+) (\\d+):(\\d+):(\\d+)");
                rx.indexIn(msgElem.timestamp().toString("d h:m:s"));

                QDate d = QDate::currentDate().addMonths(0-m_currentMonth);
                timestamp = QDateTime(
                                QDate(d.year(), d.month(), rx.cap(1).toUInt()),
                                QTime( rx.cap(2).toUInt(), rx.cap(3).toUInt() ) );
            }

        }
    } kDebug() << " end of for loop";
    if (m_readmessages.count() < m_lines)
      m_currentElements.clear(); //current elements are null this can't be allowed
      
      kDebug() << "index="<<index;
    
    emit readMessagesDoneSignal();
}



unsigned int HistoryLogger::getFirstMonth(const Kopete::Contact *c)
{
    kDebug() << "**Historylogger.cpp getfirstmonth(contact) \n\n";
    if (!c)
    {	kDebug() << "no contat specified";
        return getFirstMonth();
    }

    bool collfound=false;
    Akonadi::Collection coll,collcontact;

    if (m_collectionMap.contains(c->contactId()) )
    {
        collfound = true;
        m_tosaveInCollection=m_collectionMap[c->contactId()];
    }

    if (collfound)
    {	if ( m_tosaveInCollection.isValid() )
	    kDebug() << " valid collection has been found";
        Akonadi::Item itemx;
        Akonadi::ItemFetchJob *job2 = new Akonadi::ItemFetchJob( m_tosaveInCollection );
	kDebug() << "just before executing fetchjob";
	connect(job2,SIGNAL(itemsReceived(Akonadi::Item::List)),this,SLOT(getMonthExperimental(Akonadi::Item::List)));
	job2->start();
    }
    return 0;
}

void HistoryLogger::getMonthExperimental(Akonadi::Item::List itemList)
{
  kDebug() << "slot getmonth experimental";
  connect(this,SIGNAL(getfirstmonthwithcontactDoneSignal()),SLOT(readMessagesBlock41()));
  if (itemList.isEmpty() )
  {
    kDebug() <<"item list is empty";
    kWarning() << "itemlist empty";
    m_result = 0;
    return;
  }
  else
  {
    Akonadi::Item itemx;
    itemx=itemList.first();
    foreach( const Akonadi::Item &item, itemList )
    {
      if ( item.modificationTime() <= itemx.modificationTime() )
	itemx=item;
      kDebug()<<"itemmodicfictaion time="<<item.modificationTime().toLocalTime().toString();
    }
    int result = 12*(QDate::currentDate().year() - itemx.modificationTime().date().year()) + QDate::currentDate().month() - itemx.modificationTime().date().month();
    kDebug() <<"result="<<result;
    if (result < 0)
    {
      kWarning(14310) << "Kopete only found log file made in the future. Check your date!";
      // break;
    }
    m_result = result;
  }
  emit getfirstmonthwithcontactDoneSignal();
}

unsigned int HistoryLogger::getFirstMonth()
{
    kDebug() << "\n\nHistorylogger.cpp getfirstmonth() \n\n";
    if (m_cachedMonth!=-1)
        return m_cachedMonth;

    if (!m_metaContact)
        return 0;

    int m=0;
    QList<Kopete::Contact*> contacts=m_metaContact->contacts();

    foreach(Kopete::Contact* contact, contacts)
    {
        int m2=getFirstMonth(contact);
        if (m2>m) m=m2;
    }
    m_cachedMonth=m;
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

QList<int> HistoryLogger::getDaysForMonth(QDate date)
{
    kDebug() << "\n\nHistorylogger.cpp getdaysformonth \n\n";
    QRegExp rxTime("time=\"(\\d+) \\d+:\\d+(:\\d+)?\""); //(with a 0.7.x compatibility)

    QList<int> dayList;

    QList<Kopete::Contact*> contacts = m_metaContact->contacts();

    int lastDay=0;

    foreach(Kopete::Contact *contact, contacts)
    {
        Akonadi::CollectionFetchJob * collFetch = new Akonadi::CollectionFetchJob( m_baseCollection );
        if ( collFetch->exec() )
        {
            Akonadi::Collection::List collections = collFetch->collections();
            foreach( const Akonadi::Collection &collection, collections )
            {
                if ( collection.name() == contact->contactId() )
                {
                    Akonadi::ItemFetchJob *itemFetch = new Akonadi::ItemFetchJob( collection );
                    itemFetch->fetchScope().fetchFullPayload();
                    if (itemFetch->exec() )
                    {
                        Akonadi::Item::List items = itemFetch->items();
                        foreach ( const Akonadi::Item &item , items)
                        {
                            if ( item.modificationTime().toLocalTime().toString("yyyyMM") == date.toString("yyyyMM") )
                            {
                                QByteArray data= item.payloadData();
                                QBuffer buff;
                                buff.open(QIODevice::ReadWrite);
                                buff.write(data);
                                buff.reset();

                                QTextStream stream(&buff);
                                QString fullText = stream.readAll();
                                buff.close();

                                int pos = 0;
                                while ( (pos = rxTime.indexIn(fullText, pos)) != -1)
                                {
                                    pos += rxTime.matchedLength();
                                    int day=rxTime.capturedTexts()[1].toInt();
                                    if ( day !=lastDay && dayList.indexOf(day) == -1) // avoid duplicates
                                    {
                                        dayList.append(rxTime.capturedTexts()[1].toInt());
                                        lastDay=day;
                                    }
                                }
                            }//end of if itemmodification time
                        }
                    }	else kDebug() << " item fetch failed";
                }
            }
        } else kDebug() << "collection fetch failed";
    } //end of for reach conact loop
    return dayList;
}

#include "historylogger.moc"
