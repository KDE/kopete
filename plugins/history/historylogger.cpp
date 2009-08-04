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

// -----------------------------------------------------------------------------
HistoryLogger::HistoryLogger(Kopete::MetaContact *m, QHash<QString,Akonadi::Collection> &collMap ,QObject *parent )
        : QObject(parent)
{
    qDebug() << "historylogger-constructor-metacontact";
    m_collectionMap = collMap;
    
    m_hideOutgoing=false;
    m_filterCaseSensitive=Qt::CaseSensitive;
    m_filterRegExp=false;
    m_saveTimer=0L;
    m_saveTimerTime=0;
    m_realMonth=QDate::currentDate().month();
    m_metaContact=m;

    m_cachedMonth=-1;
    m_oldSens=Default;

    //the contact may be destroyed, for example, if the contact changes its metacontact
    connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

    setPositionToLast();
}


HistoryLogger::HistoryLogger(Akonadi::Collection &coll, Kopete::Contact *c,  QObject *parent )
        : QObject(parent)
{
    qDebug() << "historylogger-constructor-"<<c->contactId();
    m_tosaveInCollection = coll;
    
    m_saveTimer=0L;
    m_saveTimerTime=0;
    m_cachedMonth=-1;
    m_metaContact=c->metaContact();
    m_hideOutgoing=false;
    m_realMonth=QDate::currentDate().month();
    m_oldSens=Default;
    m_filterCaseSensitive=Qt::CaseSensitive;
    m_filterRegExp=false;

    connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

    setPositionToLast();
}

Akonadi::Collection HistoryLogger::m_baseCollection;

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

void HistoryLogger::getHistoryx(const Kopete::Contact *c, unsigned int month , bool canLoad , bool* contain)
{
    qDebug() <<"HistoryLogger::gethistoryx()";
    
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
            qDebug() << "GENERATING SIGNAL";
            emit getHistoryxDone();
            return;
        }
    }

    if (!m_metaContact->contacts().contains(const_cast<Kopete::Contact*>(c)))
    {
        if (contain)
            *contain=false;
        m_getHistory = History::History();
        qDebug() << "GENERATING SIGNAL";
        emit getHistoryxDone();
        return;
    }

    QMap<unsigned int , History> monthHistory = m_history[c];
    if (monthHistory.contains(month))
    {
        m_getHistory= monthHistory[month];
        qDebug() << "GENERATING SIGNAL";
        emit getHistoryxDone();
        return;
    }

    Kopete::Contact *con = const_cast<Kopete::Contact*>(c);

    Akonadi::Collection coll;
    coll = m_tosaveInCollection;
    if (!m_tosaveInCollection.isValid()) 
    {
      qDebug() <<"in gethistoryx-m_tosave in coll is invalid, returning";
      emit getHistoryxDone();
      return;
    }

    GetHistoryJob *getjob= new GetHistoryJob(coll,QDate::currentDate().addMonths(0-month));
    connect(getjob, SIGNAL(result(KJob*)), SLOT(emperimentalSlot(KJob*)));  //
    getjob->start();


}
void HistoryLogger::emperimentalSlot(KJob* job )
{
    qDebug() << "Historylogger::experimentalslot";
    GetHistoryJob *xyz = static_cast<GetHistoryJob*>(job);
    qDebug() <<"\nHistoryLogger::emperimentalSlot(KJob* job )";
    if (job->error() )
        qDebug()<<"gethistoryjobfailed";
    else {
        m_getHistory = xyz->returnHistory();
        QMap<unsigned int , History> monthHistory= m_history[m_contact];
        monthHistory.insert(m_month,m_getHistory);
        m_history[m_contact] = monthHistory;
        m_tosaveInItem = xyz->returnItem();
        qDebug() <<"GENERATING SIGNAL";
        emit getHistoryxDone();
    }


}

void HistoryLogger::appendMessage( const Kopete::Message &msg , Akonadi::Collection &coll ,Akonadi::Collection &baseColl, const Kopete::Contact *ct)
{
    qDebug() <<"HistoryLogger::appendMessage( Kopete::Message,Collection,Collection &baseColl,Contact ";
    if (!msg.from())
        return;

    m_tosaveInCollection = coll;
    qDebug() << "append message"<<coll.name();
    m_baseCollection = baseColl;
    qDebug() << "basecoll"<<m_baseCollection.name();

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
    qDebug() << "in append message, before gethistoryx";
    getHistoryx(c, QDate::currentDate().month() - date.month() - (QDate::currentDate().year() - date.year()) * 12);

}
void HistoryLogger::appendMessage2()
{
    disconnect(this, SIGNAL(getHistoryxDone()),this, SLOT(appendMessage2()));
    qDebug() <<"\nAPPENDmessage--2";
    History history;
    if (!m_getHistory.messages().isEmpty() )
    {
        history = m_getHistory;
    } else {
      qDebug() << " m_gethistory .messages is empty";
    }

    m_getHistory = History::History();

    if (history.messages().isEmpty() )
        qDebug() << "messages in history is empty";

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

    if ( history.date().month() != QDate::currentDate().month() )
    {
        qDebug()<<"\n\nof contact change or month change called:";
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
    qDebug() <<"emitting appendMessageDoneSignal";
    emit appendMessageDoneSignal();
}

HistoryLogger::~HistoryLogger()
{//TODO check for date here as well, in case when i started the chat before the month and closed the chat window
    qDebug() <<"~history logger DISTRUCTOR";
    if (m_saveTimer && m_saveTimer->isActive())
    {
      if (m_saveTimer)
	m_saveTimer->stop();
      Akonadi::Item newItem; //TODO : make the codes clean here
      m_tosaveInItem.setPayload<History>(m_toSaveHistory);
      m_tosaveInItem.setModificationTime(QDateTime::currentDateTime());
      Akonadi::ItemModifyJob *mJob = new Akonadi::ItemModifyJob(m_tosaveInItem);
      mJob->disableRevisionCheck();
      mJob->start();
      qDebug() <<"item modify job started. bye bye";
    }
    qDebug() << " going out of scope";
}

void HistoryLogger::modifyItem()
{
    qDebug() << "***Historylogger.cpp MODIFY ITEM()";

    bool itemOnlyModify=true;

    if (m_saveTimer)
        m_saveTimer->stop();
    QTime t;
    t.start(); //mesure the time needed to save.
    m_t.start();

    //Create a new collection, as well as new item.
    if ( !m_tosaveInCollection.isValid() || m_tosaveInItem.modificationTime().toLocalTime().date().toString("MMyyyy") != QDateTime::currentDateTime().date().toString("MMyyyy"))
    {
        itemOnlyModify=false;
        if (!m_tosaveInCollection.isValid() )
        {
            qDebug() << " collection doesnt exist. creating one";
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
            qDebug() <<"date time problem";
        //entering here means the item is not existing, so create one
        qDebug() << " before creating item";
        if (!m_tosaveInCollection.isValid() ) qDebug() << "m_tosave in colllecion is invalid";

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
        if ( !m_tosaveInItem.isValid() ) qDebug() << "m_tosave in item is invalid";
        else qDebug() << " m_tosave in item is valid";
	
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
    qDebug() <<"rev no="<<m_tosaveInItem.revision();
    qDebug() <<"item modification job failed"<<job->errorString();
  }
  else
  {
    qDebug() << "modified successfully\nold revision no="<<m_tosaveInItem.revision();
    m_tosaveInItem = modifyjob->item();
    m_saveTimerTime=qMin(m_t.elapsed()*1000, 300000);
    qDebug() << " saved in " << m_t.elapsed() << " ms "<<"\n"<<m_tosaveInItem.remoteId();
    qDebug()<<"new revision no="<<m_tosaveInItem.revision();
    m_toSaveHistory = History::History();

  }
}


void HistoryLogger::collectionCreateDone(KJob *job)
{
  qDebug()<<"collectioncreate done slot:";
  Akonadi::CollectionCreateJob *createJob = static_cast<Akonadi::CollectionCreateJob*>(job);
  if (job->error() )
  {
    qDebug() << "create job failed"<<job->errorString();
    return;
  }
  else
  {
    m_tosaveInCollection= createJob->collection();
    qDebug() << "\n\n**collection Created successfully YOU NEVER KNOW"<<m_tosaveInCollection.remoteId();

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
  qDebug()<<"itemcreateDoneslot";
  Akonadi::ItemCreateJob *createjob = static_cast<Akonadi::ItemCreateJob*> (job);
  if (job->error() )
    qDebug() <<"item create failed";
  else
  {
    m_tosaveInItem = createjob->item();
    qDebug() << "Item modiCREATED successfully--or in progresss";
    m_saveTimerTime=qMin(m_t.elapsed()*1000, 300000);
    qDebug() << m_tosaveInCollection.name() << " saved in " << m_t.elapsed() << " ms "<<"\n"<<m_tosaveInItem.remoteId();
    
    Akonadi::ItemFetchJob *fetchjob=new Akonadi::ItemFetchJob(m_tosaveInItem);
    fetchjob->fetchScope().fetchFullPayload(History::HeaderPayload);
    connect(fetchjob,SIGNAL(itemsReceived(Akonadi::Item::List)),this,SLOT(itemsReceivedDone(Akonadi::Item::List)));
    fetchjob->start();
  }
}
void HistoryLogger::itemsReceivedDone(Akonadi::Item::List itemlist)
{
  qDebug() << "itemreceived slot";
  m_tosaveInItem = itemlist.first();
  qDebug() << m_tosaveInItem.remoteId();
}

//this function has been used in history dialog, to show messages by date.
//QList<Kopete::Message>
void HistoryLogger::readMessages(QDate date)
{//TODO the problem is as soon as trab begins, object is gone, so no slot
    qDebug() <<"hjisLOGger::readMessages(date)";
    m_readMessagesDate= date;
    QList<Kopete::Message> messages;
    QList<Kopete::Contact*> ct=m_metaContact->contacts();

    Akonadi::TransactionSequence *transaction = new Akonadi::TransactionSequence;
    connect(transaction,SIGNAL(result(KJob*)), this, SLOT(transactionDone(KJob*)));
    
    foreach(Kopete::Contact* contact, ct)
    {	
	QVariant v;
	v.setValue<Kopete::Contact *>(contact);
	
	Akonadi::Collection coll;
	if (m_collectionMap.contains(contact->contactId()) )
	{
	  coll = m_collectionMap.value(contact->contactId());
	  GetHistoryJob *getjob = new GetHistoryJob(coll, date, transaction);
	  connect(getjob,SIGNAL(result(KJob*)) , this, SLOT(getHistoryJobDone(KJob*)) );
	  getjob->setProperty("contact",v);
	}
    }
    qDebug() <<"starting transaction";
    transaction->start();
}

void HistoryLogger::getHistoryJobDone(KJob * job)
{
  qDebug() <<"HistoryLogger::getHistoryJobDone \n for read messages(date)";
  
  GetHistoryJob * getJob = static_cast<GetHistoryJob*> (job);
  if (job->error() )
  {
    qDebug() << "job failed"<<job->errorString();
  }
  else 
  {
    qDebug() << "entered gethistoryjob done, elsepart";
    QVariant v;
    v = getJob->property("contact");
    if (v.isValid())
    {
      Kopete::Contact *contact = v.value<Kopete::Contact *>();
      qDebug() << "contat received"<<contact->contactId() ;
      History his;
      his=getJob->returnHistory();
      m_historyContact.insert( contact , his );
    }
    else
    {
      History his=getJob->returnHistory();
      m_historyList.append(his);
    }
  }
}

void HistoryLogger::transactionDone(KJob *job)
{
  qDebug() << "HistoryLogger::transactionDone(KJob *job)";
  if (job->error() )
  { 
    qDebug() << " transaction failed"<< job->errorString();
  }
  else
  {
      QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibil
      QHashIterator<Kopete::Contact *, History> i(m_historyContact);
      qDebug() <<"before while llop";
      while (i.hasNext())
      {
	i.next();
	qDebug() << "entered while loop";
	Kopete::Contact *contact = i.key();
	qDebug() <<"hell "<< contact->contactId();
	History his = i.value();
	qDebug() <<his.date();
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
  }
  emit readMessagesByDateDoneSignal(m_readmessages);
}

QList< Kopete::Message > HistoryLogger::retrunReadMessages()
{
  QList<Kopete::Message> msg=m_readmessages;
  m_readmessages.clear();
  return msg;
}


//is called every time a new instance of chat is initiated.
void HistoryLogger::readMessages(int lines, Akonadi::Collection &coll,
                                 const Kopete::Contact *c, Sens sens, bool reverseOrder, bool colorize)
{
    qDebug() << "readMessages2 (Kopete::Contact *c, Sens sens, bool reverseOrder, bool colorize)";

    m_lines=lines;
    m_readmessagesContact = c;
    m_readmessagesSens = sens;
    m_reverseOrder = reverseOrder;
    m_colorize = colorize;

    if (!m_tosaveInCollection.isValid() )
    {
      m_tosaveInCollection = coll;
      qDebug() <<"readMessages, m_to save in coll is invalid";
    }
    if (!coll.isValid() )
    {
      qDebug() <<"the passed collection coll is invalid";
    }
    
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
    if (!c && m_metaContact->contacts().count()>1)
        readmessagesBlock2();
    else
        readmessagesBlock3();
}

void HistoryLogger::readmessagesBlock2()
{
    qDebug() << "HistoryLogger::readmessagesBlock2()";

    //TODO need to test this
    if ( (!m_readmessagesContact) && m_metaContact->contacts().count()>1)
    { //we have to merge the differents subcontact history
        QList<Kopete::Contact*> ct=m_metaContact->contacts();

	Akonadi::TransactionSequence *transaction = new Akonadi::TransactionSequence;
        foreach(Kopete::Contact *contact, ct)
        {
            if (m_currentElements.contains(contact))
	    {
		History his;
                his=m_currentElements[contact];
		m_contact_history.insert(contact,his);
	    }
            else  //there is not yet "next message" register, so we will take the first  (for the current month)
            {
		//TODO : this needs to be fixed here
               // his=getHistory(contact,m_currentMonth);
		QMap<unsigned int , History> monthHistory = m_history[contact];
		if (monthHistory.contains(m_currentMonth))
		{
		  History his;
		  his = monthHistory[m_currentMonth];
		  m_contact_history.insert(contact, his);
		}
		else
		{
		  //Kopete::Contact *con = const_cast<Kopete::Contact*>(c);
		  if (m_collectionMap.contains(contact->contactId()))
		  {
		    if( !m_tosaveInCollection.isValid() )
		      m_tosaveInCollection = m_collectionMap.value(contact->contactId());
		    QVariant v;
		    v.setValue<Kopete::Contact *>(contact);
		    GetHistoryJob *getjob= new GetHistoryJob(m_tosaveInCollection,QDate::currentDate().addMonths(0-m_currentMonth), transaction);
		    getjob->setProperty("contact", v);
		    connect(getjob, SIGNAL(result(KJob*)), SLOT(GetJobInReadMessage2Done(KJob*)));
		  }
		}
	    }
	}// end of for loop for each cotact
	connect(transaction,SIGNAL(KJob*),this,SLOT(transaction_in_read_message_block_2_done(KJob*)) );
	transaction->start();
    }// end of if
}
void HistoryLogger::GetJobDoneInReadMessage2Done(KJob* job)
{
  GetHistoryJob *getJob = static_cast<GetHistoryJob *>(job);
  if ( job->error())
    kDebug() <<" get history job failed. reson"<<job->errorString();
  else
  {
    QVariant v;
    History his = getJob->returnHistory();
    v= getJob->property("contact");
    Kopete::Contact *c = v.value<Kopete::Contact *>();
    m_contact_history.insert(c,his);
  }
}
void HistoryLogger::transaction_in_read_message_block_2_done(KJob *job)
{
  QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)

  if (job->error() )
  {
    kDebug() << "transaction_in_read_message_block 2 failed"<<job->errorString();
  }
  else  
  {
    QMapIterator<Kopete::Contact *, History> i(m_contact_history);
    History history;
    while (i.hasNext())
    {
	    History his;
	    i.next();
	    Kopete::Contact *contact = i.key();
	    his = i.value();
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
            }
        }
	m_readmessagesHistory = history;
    }
    readMessagesBlock4();
}

void HistoryLogger::readmessagesBlock3()
{
    qDebug()<<"read messages block 3\n"<<"m_currentmonth="<<m_currentMonth;
    if (m_currentElements.contains(m_currentContact)) //TODO need to take a look in this if
    {
	qDebug() <<"m_currentElementscontains";
        m_readmessagesHistory=m_currentElements[m_currentContact];
	readmessagesBlock31();
    }
    else
    {
        connect(this,SIGNAL(getHistoryxDone()),SLOT(readmessagesBlock31()) );
        getHistoryx(m_currentContact,m_currentMonth);
    }
}
void HistoryLogger::readmessagesBlock31()
{
    qDebug()<<"read messages block 31,signal disconnected"<<"m_currentmonth="<<m_currentMonth;
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
    qDebug() << "readMessages block 4"<<"m_currentmonth="<<m_currentMonth;
    if (m_readmessagesHistory.messages().isEmpty() || index >= m_readmessagesHistory.messages().count() || index < 0)
    {
        if (index <0 ) qDebug() <<"inside  change month, index <0";
	
        if (m_readmessagesSens==Chronological)
        {
            qDebug() << "m_currentmonth="<<m_currentMonth;
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
  qDebug()<<"readmessages block 41\nsignal disconnected";
  disconnect(this,SIGNAL(getfirstmonthwithcontactDoneSignal()),this,SLOT(readMessagesBlock41()));
  if (m_currentMonth >= m_result)
  {
    qDebug() << "returning";
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
    qDebug()<<"read messages block 5";
    History::Message msgElem;
    const Kopete::Contact * currentContact = m_currentContact;
    //getting the color for messages:
    QColor fgColor = HistoryConfig::history_color();
    QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)

    qDebug() << "index="<<index;

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
    } qDebug() << " end of for loop";
    if (m_readmessages.count() < m_lines)
      m_currentElements.clear(); //current elements are null this can't be allowed
      
      qDebug() << "index="<<index;
    
    emit readMessagesDoneSignal(m_readmessages);
}



unsigned int HistoryLogger::getFirstMonth(const Kopete::Contact *c)
{
    qDebug() << "**Historylogger.cpp getfirstmonth(contact) \n\n";
    if (!c)
    {	qDebug() << "no contat specified";
        return getFirstMonth();
    }

    bool collfound=false;
    Akonadi::Collection coll,collcontact;
    
    if (m_tosaveInCollection.isValid() ) 
    { collfound = true; 
    }
    if (collfound)
    {	if ( m_tosaveInCollection.isValid() )
	    qDebug() << " valid collection has been found";
        Akonadi::Item itemx;
        Akonadi::ItemFetchJob *job2 = new Akonadi::ItemFetchJob( m_tosaveInCollection );
	qDebug() << "just before executing fetchjob";
	connect(job2,SIGNAL(itemsReceived(Akonadi::Item::List)),this,SLOT(getMonthExperimental(Akonadi::Item::List)));
	job2->start();
    }
    return 0;
}

void HistoryLogger::getMonthExperimental(Akonadi::Item::List itemList)
{
  qDebug() << "slot getmonth experimental";
  connect(this,SIGNAL(getfirstmonthwithcontactDoneSignal()),SLOT(readMessagesBlock41()));
  if (itemList.isEmpty() )
  {
    qDebug() <<"item list is empty";
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
      qDebug()<<"itemmodicfictaion time="<<item.modificationTime().toLocalTime().toString();
    }
    int result = 12*(QDate::currentDate().year() - itemx.modificationTime().date().year()) + QDate::currentDate().month() - itemx.modificationTime().date().month();
    qDebug() <<"result="<<result;
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
    qDebug() << "\n\nHistorylogger.cpp getfirstmonth() \n\n";
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

//QList<int> 
void HistoryLogger::getDaysForMonth(QDate date)
{
    qDebug() << "\nHistorylogger.cpp getdaysformonth";
    QRegExp rxTime("time=\"(\\d+) \\d+:\\d+(:\\d+)?\""); //(with a 0.7.x compatibility)

    QList<Kopete::Contact*> contacts = m_metaContact->contacts();

    int lastDay=0;

    Akonadi::TransactionSequence *transaction = new Akonadi::TransactionSequence;

    foreach(Kopete::Contact *contact, contacts)
    {
      if(m_collectionMap.contains(contact->contactId()) )
      {
	Akonadi::Collection coll;
	coll = m_collectionMap.value(contact->contactId());
	GetHistoryJob *getHistory = new GetHistoryJob(coll,date,transaction);
	connect(getHistory,SIGNAL(result(KJob*)),this,SLOT(getHistoryJobDone(KJob*)));
      }
      connect(transaction,SIGNAL(result(KJob*)), this, SLOT(getDaysForMonthSlot(KJob*)));
      transaction->start();
    }
}

void HistoryLogger::getDaysForMonthSlot(KJob* job)
{
  QList<int> dayList;
  if (job->error())
    qDebug() << "error--getdays of month slto, transaction fils";
  else
  {
    int lastDay=0;
    foreach( const History &his, m_historyList)
    {
      foreach(const History::Message &msg, his.messages() )
      {
	int day=msg.timestamp().date().day();
	if ( day != lastDay )
	{
	  dayList.append(day);
	  lastDay=day;
	}
      }
    }
    emit getDaysForMonthSignal(dayList);
  }
}

#include "historylogger.moc"
