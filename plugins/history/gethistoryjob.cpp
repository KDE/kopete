
#include "gethistoryjob.h"
#include <KJob>
#include <kdebug.h>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Collection>


GetHistoryJob::GetHistoryJob(const Akonadi::Collection coll , const QDate date, QObject* parent): KJob(parent)
{
  m_collection= coll;
  m_date = date;
  kDebug() << "constructor"<<m_date<<m_collection.name();
}

void GetHistoryJob::start()
{
  kDebug() <<"start";
  Akonadi::ItemFetchJob *fetchjob = new Akonadi::ItemFetchJob(m_collection);
  fetchjob->fetchScope().fetchFullPayload();
  connect (fetchjob, SIGNAL(itemsReceived(Akonadi::Item::List)), this,SLOT(itemsReceivedSlot(Akonadi::Item::List)) );
  connect( fetchjob, SIGNAL(result(KJob* )), this,SLOT(itemJobDone(KJob*)) );
  fetchjob->start();
//  this->suspend();
 
}

void GetHistoryJob::itemsReceivedSlot(Akonadi::Item::List itemList)
{
  kDebug() << "itemsReceivedslot";
  foreach( const Akonadi::Item &item, itemList)
  {
    if ( item.modificationTime().toLocalTime().toString("MMyyyy")== m_date.toString("MMyyyy") )
    {
      m_item=item;
      if(item.hasPayload<History>() )
	m_history = item.payload<History>();
      break;
    }
  }
}

GetHistoryJob::~GetHistoryJob()
{

}

void GetHistoryJob::itemJobDone(KJob* job)
{
  kDebug() <<"itemjobdone";
  if (job->error())
    kDebug() << "gethistoryjob job failed"<<job->errorString();
  emit emitResult();
}

History GetHistoryJob::returnHistory()
{
  return m_history;
}

Akonadi::Item GetHistoryJob::returnItem()
{
  return m_item;
}
