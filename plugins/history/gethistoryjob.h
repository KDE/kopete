
#ifndef GETHISTORYJOB_H
#define GETHISTORYJOB_H

#include <KJob>
#include "history.h"
#include <Akonadi/Collection>
#include <QDate>
#include <akonadi/item.h>

namespace Kopete { class Contact; }


class GetHistoryJob : public KJob
{
  Q_OBJECT
  
  public:
    explicit GetHistoryJob( const Akonadi::Collection , const QDate date , QObject *parent = 0 );
    
    virtual ~GetHistoryJob();
    
    void start();
    History returnHistory();
    Akonadi::Item returnItem();
    
  private slots:
    void itemsReceivedSlot(Akonadi::Item::List );
    void itemJobDone(KJob* );
 //   void getHistoryJobDone(KJob*);
  private:
    History m_history;
    Akonadi::Collection m_collection;
    Akonadi::Item m_item;
    QDate m_date;
    
};

#endif
