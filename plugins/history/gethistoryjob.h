/*
    gethistoryjob.h

    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>

    Kopete    (c) 2009 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef GETHISTORYJOB_H
#define GETHISTORYJOB_H

//#include <KJob>
#include "history.h"
#include <Akonadi/Collection>
#include <QDate>
#include <akonadi/item.h>
#include <Akonadi/Job>

namespace Kopete { class Contact; }


class GetHistoryJob : public Akonadi::Job
{
  Q_OBJECT
  
  public:
    explicit GetHistoryJob( const Akonadi::Collection , const QDate date , QObject *parent = 0 );
    
    virtual ~GetHistoryJob();
    
    History returnHistory();
    Akonadi::Item returnItem();
    
  protected:
    virtual void doStart();
    
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
