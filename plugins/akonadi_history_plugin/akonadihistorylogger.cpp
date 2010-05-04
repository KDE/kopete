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



#include "akonadihistorylogger.h"
#include "akonadihistoryplugin.h"

#include <QtCore/QTimer>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/Collection>
#include <akonadi/item.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemfetchscope.h>


#include "kopeteglobal.h"
#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetemetacontact.h"
#include "kopetechatsession.h"






AkonadiHistoryLogger::AkonadiHistoryLogger(Kopete::Contact *c, QObject *parent , QObject *hPlugin )
        : QObject(parent)
{
    kDebug() << "historylogger-constructor called";
    m_hPlugin = qobject_cast<AkonadiHistoryPlugin*>(hPlugin);   
    m_kopeteChat = m_hPlugin->getCollection() ;
    m_tosaveInCollection = m_hPlugin->getCollection(c->account()->accountId(), c->contactId());
    kDebug() << "before m_parentCollection.isValid()" << m_parentCollection.isValid();
    m_parentCollection = m_hPlugin->getCollection(c->account()->accountId());
    kDebug() << "after m_parentCollection.isValid()" << m_parentCollection.isValid();

}


AkonadiHistoryLogger::~AkonadiHistoryLogger()
{

    kDebug() <<"~history logger DISTRUCTOR";

}


void AkonadiHistoryLogger::appendMessage( const Kopete::Message &msg , const Kopete::Contact *ct)

{  kDebug() << " ";
  
     if (!msg.from())
        return;
    
    
    const Kopete::Contact *c = ct;
    if (!c && msg.manager() )
    {
        QList<Kopete::Contact*> mb=msg.manager()->members() ;
        c = mb.first();
    }
    if (!c) 
        c =   msg.direction()==Kopete::Message::Outbound ? msg.to().first() : msg.from()  ;


 
    m_message = msg;
    m_contact = c;
  
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
    messagek.setSender(  m_message.from()->contactId() );
    messagek.setText( m_message.plainBody() );
    
    
    
    foreach( const History::Message messagee, m_toSaveHistory.messages())
    {
	 history.addMessage(messagee);
    }
    
    
    history.addMessage(messagek);
    

    
    m_toSaveHistory=history;

    if (!m_saveTimer)
    {
        m_saveTimer=new QTimer(this);
	kDebug() << "\n\n\n\tconnection to slot\n";
        connect( m_saveTimer, SIGNAL( timeout() ) , this, SLOT(appendMessage2()) );
    }
    if (!m_saveTimer->isActive())
    {	
        m_saveTimer->setSingleShot( true );
        m_saveTimer->start(  );
	
    }
 
}

void AkonadiHistoryLogger::appendMessage2()
{
  kDebug() << " ";
  
 
     kDebug() << "m_parentCollection.isValid()1" << m_parentCollection.isValid();
  
  
  
    if ( !m_tosaveInCollection.isValid() )
    {
            kDebug() << "m_parentCollection.isValid()" << m_parentCollection.isValid();
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
	    connect(jobcreatecoll,SIGNAL(result(KJob*)),SLOT(createCollection(KJob*)) );

	   return;
	   
            
   	
	    
    }   
	  


  else if ( !m_tosaveInItem.isValid() )
    {
            kDebug() << " item doesnt exist in the collection for this session.";
             QTimer *t = new QTimer(this);
             connect(t,SIGNAL(timeout()),SLOT(createItem()) );
	      t->setSingleShot(true);
              t->start(2000);

          
	    
    }   
	
  else
  
    {    
      
       
        kDebug() << " item exixts for this session.modifying it"<< m_tosaveInItem.id();
       if ( !m_tosaveInItem.isValid() ) kDebug() << "m_tosave in item is invalid";
        else kDebug() << " m_tosave in item is valid";
	
	
	
	m_tosaveInItem.setPayload< History>(m_toSaveHistory);
	m_tosaveInItem.setModificationTime( QDateTime::currentDateTime() );
	
	Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(m_tosaveInItem);
	modifyJob->disableRevisionCheck();

	modifyJob->start();
	
	
          
    }
          

}



void AkonadiHistoryLogger::createCollection(KJob *job)
{

     kDebug() << " ";
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


void AkonadiHistoryLogger::createItem()
{
  kDebug() << "  ";
    m_tosaveInItem.clearPayload();
    m_tosaveInItem.setMimeType("application/x-vnd.kde.kopetechathistory" );
    m_tosaveInItem.setModificationTime(QDateTime::currentDateTime());
    m_tosaveInItem.setPayload<History>(m_toSaveHistory);

    if (m_tosaveInItem.hasPayload<History>() )
    {
	  Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(m_tosaveInItem,m_tosaveInCollection);
	  connect(createJob,SIGNAL(result(KJob * )),this ,SLOT(itemCreateDone(KJob*)));
    }
}
void AkonadiHistoryLogger::itemCreateDone(KJob* job)
{
 
  kDebug()<<" ";
  Akonadi::ItemCreateJob *createjob = static_cast<Akonadi::ItemCreateJob*> (job);
  if (job->error() )
    kDebug() <<"item create failed";
  else
  {
    m_tosaveInItem = createjob->item();
    kDebug() << "Item created sucessfully";
  
    Akonadi::ItemFetchJob *fetchjob=new Akonadi::ItemFetchJob(m_tosaveInItem);
    fetchjob->fetchScope().fetchFullPayload(History::HeaderPayload);
    connect(fetchjob,SIGNAL(itemsReceived(Akonadi::Item::List)),this,SLOT(itemsReceivedDone(Akonadi::Item::List)));
    fetchjob->start();
    kDebug() << " item ................."<<m_tosaveInItem.id();
  }
}


void AkonadiHistoryLogger::itemsReceivedDone(Akonadi::Item::List itemlist)
{
  kDebug() << " ";
  m_tosaveInItem = itemlist.first();
  kDebug() << m_tosaveInItem.remoteId();
}



#include "akonadihistorylogger.moc"