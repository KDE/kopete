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
    
    m_parentCollection = m_hPlugin->getCollection(c->account()->accountId());
    itemFetched = true;
    itemModifiedOnce = false;

    if( m_tosaveInCollection.isValid() )
    {
      kDebug() << "m_tosave in coll isexists";
      m_tosaveInItem = m_hPlugin->getItem(m_tosaveInCollection.id() );
      
      if(m_tosaveInItem.isValid() )
      {
	  Akonadi::ItemFetchJob *j = new Akonadi::ItemFetchJob( m_tosaveInItem );
	  j->fetchScope().fetchFullPayload() ;
	  itemFetched = false ;
	  connect(j, SIGNAL(itemsReceived(Akonadi::Item::List)), this , SLOT(itemsReceivedDone(Akonadi::Item::List) ) );
      }
    }
}


AkonadiHistoryLogger::~AkonadiHistoryLogger()
{

    kDebug() <<"~history logger DISTRUCTOR";
/*
    m_tosaveInItem.setPayload<History>(m_history);
    m_tosaveInItem.setModificationTime( QDateTime::currentDateTime() );

    Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(m_tosaveInItem);
    modifyJob->disableRevisionCheck();
    modifyJob->start();
*/

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
  
    if (m_history.messages().isEmpty() )
        kDebug() << "messages in history is empty";

    if ( !m_history.date().isValid() )
        m_history.setDate(QDate::currentDate());
    if (m_history.localContactId().isEmpty() )
        m_history.setLocalContactId( m_contact->account()->myself()->contactId() );
    if (m_history.remoteContactId().isEmpty() )
        m_history.setRemoteContactId( m_contact->contactId() );

    History::Message messagek;
    messagek.setIn( m_message.direction()==Kopete::Message::Outbound ? "0" : "1" );
    messagek.setSender(  m_message.from()->contactId() );
    messagek.setNick( m_message.from()->property( Kopete::Global::Properties::self()->nickName() ).value().toString() );
    messagek.setTimestamp( m_message.timestamp() );
    messagek.setSender(  m_message.from()->contactId() );
    messagek.setText( m_message.plainBody() );
    
    
    m_history.addMessage(messagek);
    kDebug() << itemFetched ;
    if( itemFetched && itemModifiedOnce)
    { kDebug() << "itemfetched && item modofied";
      appendMessage2();
    }
    else if( itemFetched && !itemModifiedOnce )
    { kDebug() << "itemfetched && !item modofiedonce";
	History his ;
	if ( m_tosaveInItem.hasPayload<History>() )
	{kDebug() << "of type history";
	      his= m_tosaveInItem.payload<History>();
	      foreach(const History::Message &m, m_history.messages() )
		  his.addMessage( m );
	      m_history = his;
	 }
	 appendMessage2();
    }
    else 
      return;
}

void AkonadiHistoryLogger::appendMessage2()
{

    if ( !m_tosaveInCollection.isValid() )
    {
            kDebug() << "m_tosave in collection invalid";

	    QStringList mimeTypes;
            mimeTypes  << "application/x-vnd.kde.kopetechathistory"<< "inode/directory";
	    
	    if( !m_parentCollection.isValid() )
	    {
		Akonadi::Collection pCollection;
		pCollection.setParent( m_kopeteChat );
		pCollection.setName( m_history.localContactId() );
		pCollection.setContentMimeTypes( mimeTypes );
	    
		Akonadi::CollectionCreateJob *j = new Akonadi::CollectionCreateJob( pCollection );
		connect(j, SIGNAL(result(KJob*)), this, SLOT(pCollectionCreated(KJob*)) );
		return;
	    }
	    
	    kDebug() << m_contact->account() << m_contact->contactId();
	    
            Akonadi::Collection collection;
            collection.setParent( m_parentCollection );	    
            collection.setName( m_history.remoteContactId() );
            collection.setContentMimeTypes( mimeTypes );

            Akonadi::CollectionCreateJob *jobcreatecoll = new Akonadi::CollectionCreateJob( collection  );
	    connect(jobcreatecoll,SIGNAL(result(KJob*)),SLOT(createCollection(KJob*)) );

	   return;
    }   

    else if ( !m_tosaveInItem.isValid() )
    {
            kDebug() << " item doesnt exist in the collection for this session.";
	    createItem();
    }
    
    else if ( m_tosaveInItem.isValid() )
    {   
	m_tosaveInItem.setPayload<History>(m_history);
	m_tosaveInItem.setModificationTime( QDateTime::currentDateTime() );
	
	Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(m_tosaveInItem);
	connect(modifyJob, SIGNAL(result(KJob*)), this , SLOT(slotItemModified(KJob*)) );
	modifyJob->disableRevisionCheck();
	}
}

void AkonadiHistoryLogger::pCollectionCreated(KJob* j)
{
    if( j->error() ) {
	kDebug()<< j->errorText();
	return;
    }
    
    Akonadi::CollectionCreateJob *job = static_cast<Akonadi::CollectionCreateJob*>( j );
    m_parentCollection = job->collection();
    kDebug() << "parent colelction created" << m_parentCollection.remoteId();
    appendMessage2() ;
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
      kDebug() << "\n\n**collection Created successfully "<<m_tosaveInCollection.remoteId();
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
    m_tosaveInItem.setPayload<History>(m_history);
	  
    Akonadi::ItemCreateJob *createJob = new Akonadi::ItemCreateJob(m_tosaveInItem,m_tosaveInCollection);	
    connect(createJob,SIGNAL(result(KJob * )),this ,SLOT(itemCreateDone(KJob*)));
}
void AkonadiHistoryLogger::itemCreateDone(KJob* job)
{
 
  kDebug()<<" ";
  Akonadi::ItemCreateJob *createjob = static_cast<Akonadi::ItemCreateJob*> (job);
  if (job->error() )
    kDebug() <<"item create failed"<<job->errorText();
  else
  {
    m_tosaveInItem = createjob->item();
    kDebug() << "Item created sucessfully";
  
    Akonadi::ItemFetchJob *fetchjob=new Akonadi::ItemFetchJob(m_tosaveInItem);
    fetchjob->fetchScope().fetchFullPayload();
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
  itemFetched = true;
  m_hPlugin->setItem(m_tosaveInCollection.id(), m_tosaveInItem.id() );
}

void AkonadiHistoryLogger::slotItemModified(KJob* j)
{
    if( j->error()) {
	kDebug() << j->errorText();
	return;
    }
    kDebug()<< "item modified sucessfully";
    itemModifiedOnce = true;
    
    Akonadi::ItemModifyJob *job = static_cast<Akonadi::ItemModifyJob*>(j);
    m_tosaveInItem = job->item();
}



#include "akonadihistorylogger.moc"