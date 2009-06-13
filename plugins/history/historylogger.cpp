/*
    historylogger.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@kde.org>

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

#include "kopeteglobal.h"
#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetemetacontact.h"
#include "kopetechatsession.h"

#include "historyconfig.h"
#include <akonadi/collectionfetchjob.h>

bool messageTimestampLessThan(const Kopete::Message &m1, const Kopete::Message &m2)
{
    kDebug() << "\n\nHistorylogger.cpp messagetimestamplessthan \n\n";
    return m1.timestamp() < m2.timestamp();
}

// -----------------------------------------------------------------------------
HistoryLogger::HistoryLogger( Kopete::MetaContact *m,  QObject *parent )
        : QObject(parent)
{
    kDebug() << "\n\nHistorylogger.cpp constructor1 \n\n";
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


HistoryLogger::HistoryLogger( Kopete::Contact *c,  QObject *parent )
        : QObject(parent)
{
    kDebug() << "***Historylogger.cpp constructor2 ( Kopete::Contact *c,  QObject *parent )\n\n";
    m_saveTimer=0L;
    m_saveTimerTime=0;
    m_cachedMonth=-1;
    m_metaContact=c->metaContact();
    m_hideOutgoing=false;
    m_realMonth=QDate::currentDate().month();
    m_oldSens=Default;
    m_filterCaseSensitive=Qt::CaseSensitive;
    m_filterRegExp=false;

    //the contact may be destroyed, for example, if the contact changes its metacontact
    connect(m_metaContact , SIGNAL(destroyed(QObject *)) , this , SLOT(slotMCDeleted()));

    setPositionToLast();
}


HistoryLogger::~HistoryLogger()
{
    if (m_saveTimer && m_saveTimer->isActive())
        modifyItem();
}


void HistoryLogger::setPositionToLast()
{
    kDebug() << "***Historylogger.cpp setpositiontolast \n\n";
    setCurrentMonth(0);
    m_oldSens = AntiChronological;
    m_oldMonth=0;
    m_oldElements.clear();
}


void HistoryLogger::setPositionToFirst()
{
    kDebug() << "Historylogger.cpp setposition to first \n\n";
    setCurrentMonth( getFirstMonth() );
    m_oldSens = Chronological;
    m_oldMonth=m_currentMonth;
    m_oldElements.clear();
}


void HistoryLogger::setCurrentMonth(int month)
{
    kDebug() << "***Historylogger.cpp setcurrentmonth \n\n";
    m_currentMonth = month;
    m_currentElements.clear();
}


History HistoryLogger::getHistory(const Kopete::Contact *c, unsigned int month , bool canLoad , bool* contain)
{
    kDebug() << "\n\n***Historylogger.cpp getdocument1 \n\n";
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
            return History();
    }

    if (!m_metaContact->contacts().contains(const_cast<Kopete::Contact*>(c)))
    {
        if (contain)
            *contain=false;
        return History();
    }

    QMap<unsigned int , History> monthHistory = m_history[c];
    if (monthHistory.contains(month))
    {
        kDebug()<<"\n\n** returning documetns[month] which was no expected";
        return monthHistory[month];
    }
 
    History his =  getHistory(c, QDate::currentDate().addMonths(0-month), canLoad, contain);

    monthHistory.insert(month, his);
    m_history[c]=monthHistory;

    kDebug()<<"\n\n***reached end of getHisotry1. now returning";
    return his;

}

History HistoryLogger::getHistory(const Kopete::Contact *contact, const QDate date , bool canLoad , bool* contain)
{
    kDebug() << "***Historylogger.cpp getdocument2 \n\n";
    //"founditem" to check if the item for a contact exists or not.
    bool founditem = false;

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
    kDebug()<<"**just before getfilename in gethistory2";
///////////////////////////////////////////////
//    QString FileName = getFileName(c, date);
//    QByteArray FileNameFlag;
//    FileNameFlag.append(FileName);

    Akonadi::Collection coll, collcontact;
    //fetch my kopete base collection
    //once they are fetched, and saved in, dont need to search again. 
    //but this is search with every instance of chat, TODO i dont need to search for every instance.
    if ( !m_baseCollection.isValid() )
    {
      Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::FirstLevel );
      if ( job->exec()  )
      {
	  qDebug()<<" collection fetch job for root ececuted";
	  Akonadi::Collection::List collections = job->collections();
	  foreach( const Akonadi::Collection &collection, collections )
	  {
	      if ( collection.name() == "kopeteChat" )
	      {
                coll = collection;
		m_baseCollection = collection;
	      }
	  }
      } else qDebug() << "collection fetch job not executed";
    }else kDebug() << "root collection is lareadythere";

    bool collfound= false;
  
    if ( m_tosaveInCollection.name() != c->contactId() )
    {
      //fetch the collection which belongs to my contact
      Akonadi::CollectionFetchJob *job2 = new Akonadi::CollectionFetchJob( coll , Akonadi::CollectionFetchJob::FirstLevel );
      if ( job2->exec()  )
      {
	  Akonadi::Collection::List collections = job2->collections();
	  foreach( const Akonadi::Collection &collection, collections )
	  {
	      if ( collection.name() == c->contactId() )
	      {
		  collfound = true;
		  collcontact = collection;
		  m_tosaveInCollection= collcontact;
		  kDebug() << " collection for this contact already exists";
		  break;
	      }
	  }
      } else qDebug() << "collection fetch job not executed";
      //if collection for that contact dont exist ceate it
      if ( !collfound )
      {
	  qDebug() << " collection doesnt exist. creating one";
	  Akonadi::Collection collection;
	  collection.setParent( coll );
	  collection.setName( c->contactId() );
  
	  QStringList mimeTypes;
	  mimeTypes  << "application/x-vnd.kde.kopetechathistory";
	  collection.setContentMimeTypes( mimeTypes );


	  Akonadi::CollectionCreateJob *jobcreatecoll = new Akonadi::CollectionCreateJob( collection  );
	  if ( jobcreatecoll->exec() )
	  {
	      collcontact = jobcreatecoll->collection();
	      m_tosaveInCollection = collcontact;
	      kDebug() << "Created successfully";
	  } else kDebug() << "Error occurred in making a collection ***";
      }
    } else kDebug() << "collection for contact is also there";

    //fetch the item in that collection
    //fetch the item which has lest modifitacion time
    History history;
    Akonadi::Item fileItem;
    Akonadi::ItemFetchJob *itemjob = new Akonadi::ItemFetchJob( m_tosaveInCollection );
    itemjob->fetchScope().fetchFullPayload();
    if ( itemjob->exec() )
    {
        Akonadi::Item::List items = itemjob->items();
        if ( !items.isEmpty() )
        {
            kDebug() << "item list is not empty";
            foreach( const Akonadi::Item &item, items )
            {
                if ( item.modificationTime().toString("MMyyyy")== date.toString("MMyyyy") )
                {
                    fileItem = item ;
                    founditem=true;
                    kDebug() << " found item with matching time";
		    break;
                }
            }
            m_tosaveInItem=fileItem;
	    if (fileItem.hasPayload<History>() )
	    {
		     history = fileItem.payload< History >();
	    }

	}
        else
        {
            kDebug() << " item list empty. item will be created while saving";
            Akonadi::Item i;
            m_tosaveInItem = i; //the item now point to a null collection

        }
    }
    else qDebug() << "item fetch job failed";

    //TODO // this has been done above
    //now we laready have an item in the collection.
    //we check if the last modification time of item = current month,year.
    //if not create a new item. Lets do that in saving. lets check it there.

//    QDomDocument doc( "Kopete-History" );
      
      
    if ( founditem )
    {
        kDebug() << "item has been found";

        if ( !m_tosaveInItem.hasPayload() )
        {
            if (contain)
                *contain=false;
            return history;
        }
    }
    kDebug() << " leaving function getDocment";
    return history;
}


void HistoryLogger::appendMessage( const Kopete::Message &msg , const Kopete::Contact *ct )
{
    kDebug() << "\n\n\n***Historylogger.cpp appendmessage \n\n";
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

//    QDomDocument doc=getDocument(c, QDate::currentDate().month() - date.month() - (QDate::currentDate().year() - date.year()) * 12);
      
     History history = getHistory(c, QDate::currentDate().month() - date.month() - (QDate::currentDate().year() - date.year()) * 12);
  
     kDebug()<<"\n*nothin important**\n the date part="<<(QDate::currentDate().month() - date.month() - (QDate::currentDate().year() - date.year()) * 12);

     if ( !history.date().isValid() ) 
       history.setDate(QDate::currentDate());
     if (history.localContactId().isEmpty() )
       history.setLocalContactId( c->account()->myself()->contactId() );
     if (history.remoteContactId().isEmpty() )
       history.setRemoteContactId( c->contactId() );
     
     History::Message messagek;
     messagek.setIn( msg.direction()==Kopete::Message::Outbound ? "0" : "1" );
     messagek.setSender(  msg.from()->contactId() );
     messagek.setNick( msg.from()->property( Kopete::Global::Properties::self()->nickName() ).value().toString() );
     messagek.setTimestamp( msg.timestamp() );
     messagek.setText( msg.plainBody() );

     history.addMessage(messagek);
     QMap <unsigned int, History > monthHistory;
     monthHistory.insert(QDate::currentDate().month() - history.date().month() - (QDate::currentDate().year() - history.date().year()) * 12, history);
     m_history[c]=monthHistory;
     
     QBuffer buff;
     buff.open(QBuffer::ReadWrite);
     HistoryXmlIo::writeHistoryToXml(history,&buff);
     kDebug()<<buff.data();
     buff.close();

      // I'm temporizing the save.
      // On hight-traffic channel, saving can take lots of CPU. (because the file is big)
      // So i wait a time proportional to the time needed to save..
      //    const QString filename=getFileName(c, date);

      /*   if (!m_toSaveFileName.isEmpty() && m_toSaveFileName != filename)
      { //that mean the contact or the month has changed, save it now.
        modifyItem();
      }
      */	//same logic reimplemented
     if ( history.date().month() != QDate::currentDate().month() )
     {
        kDebug()<<" ***\n\n\nof contact change or month change called:";
	modifyItem();
     }
//    modifyItem();
//    m_toSaveFileName=filename;
//    m_toSaveDocument=doc;
  
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
}

void HistoryLogger::modifyItem()
{
    kDebug() << "***Historylogger.cpp smodifyItem \n\n";

    bool itemOnlyModify=true;
//    bool itemCreate = false;

    if (m_saveTimer)
        m_saveTimer->stop();
//    if ( m_toSaveDocument.isNull() ) {;}
      //  return;

    QTime t;
    t.start(); //mesure the time needed to save.

    //entering here means the item is not existing, so createone.
    if (!m_tosaveInItem.isValid() )
    {
	kDebug() << " tosave in item is invalid";
        Akonadi::Item itemToCreate;
        itemToCreate.setMimeType("application/x-vnd.kde.kopetechathistory" );

        Akonadi::ItemCreateJob *createitem = new Akonadi::ItemCreateJob( itemToCreate, m_tosaveInCollection);
        if ( createitem->exec() )
        {
            m_tosaveInItem = createitem->item();
            m_tosaveInItem.setModificationTime(QDateTime::currentDateTime());
            m_tosaveInItem.setPayload<History>(m_toSaveHistory);
            Akonadi::ItemModifyJob *modifyjob = new Akonadi::ItemModifyJob(m_tosaveInItem);
            if ( modifyjob->exec() )
            {
                qDebug() << "Item modified successfully";
                itemOnlyModify=false;
                m_saveTimerTime=qMin(t.elapsed()*1000, 300000);
                kDebug(14310) << m_tosaveInCollection.name() << " saved in " << t.elapsed() << " ms ";
            } else kDebug() << " failed";
	} else kDebug() << " failed in creating a new item";
    }

    //enter here if u needtomify item is true, and change in month.
    if ( itemOnlyModify && m_tosaveInItem.modificationTime().date().toString("MMyyyy") != QDateTime::currentDateTime().date().toString("MMyyyy") )
    {
        itemOnlyModify=false;
        kDebug() << "item date="<<m_tosaveInItem.modificationTime().date().toString("MMyyyy");
        kDebug() << "curent date= " << QDateTime::currentDateTime().date().toString("MMyyyy");
        kDebug() << " date time problem for item creating one";
        Akonadi::Item itemnew;
        itemnew.setMimeType("application/x-vnd.kde.kopetechathistory") ;
        Akonadi::ItemCreateJob *createItem = new Akonadi::ItemCreateJob(itemnew, m_tosaveInCollection);
        if ( createItem->exec() )
        {
            qDebug() << " since datetime problem creating new item";
            m_tosaveInItem = createItem->item();
            m_tosaveInItem.setModificationTime(QDateTime::currentDateTime());
            m_tosaveInItem.setPayload<History>(m_toSaveHistory);

            Akonadi::ItemModifyJob *modifyjob = new Akonadi::ItemModifyJob(m_tosaveInItem);
            if ( modifyjob->exec() )
            {
                qDebug() << "Item modified successfully";
                m_saveTimerTime=qMin(t.elapsed()*1000, 300000);
                kDebug(14310) << m_tosaveInCollection.name() << " saved in " << t.elapsed() << " ms ";
            } else kDebug() << " failed";

        } else kDebug() << "create item in modification time failed";
    }

    //enters here if u have not created newitem, no change in month. so item only modify it true.
    if ( itemOnlyModify)
    {
        Akonadi::ItemFetchJob *fetchjob = new Akonadi::ItemFetchJob(m_tosaveInItem);
        fetchjob->fetchScope().fetchFullPayload();
        if ( fetchjob->exec() )
        {
            Akonadi::Item itemx = fetchjob->items().first();

            itemx.setPayload< History>(m_toSaveHistory);
            itemx.setModificationTime( QDateTime::currentDateTime() );
//      if ( !item.hasFlag ( m_toSaveFileNameFlag ) ) item.setFlag( m_toSaveFileNameFlag );
            // Store back modified item
            Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob( itemx );
            if ( modifyJob->exec() )
            {
                qDebug() << "Item modified successfully";
                m_saveTimerTime=qMin(t.elapsed()*1000, 300000);
                //a time 1000 times supperior to the time needed to save.  but with a upper limit of 5 minutes
                //on a my machine, (2.4Ghz, but old HD) it should take about 10 ms to save the file.
                // So that would mean save every 10 seconds, which seems to be ok.
                // But it may take 500 ms if the file to save becomes too big (1Mb).
                kDebug(14310) << m_tosaveInCollection.name() << " saved in " << t.elapsed() << " ms ";

            }
            else
                qDebug() << "Error occurred in modification during saving to akonadi";
        }
        else kDebug() << "in save item fetch failing";

    }
//    m_toSaveFileName.clear();
//    m_toSaveDocument=QDomDocument();
    m_toSaveHistory = History::History();
}


//this function has been used in history dialog, to show messages by date.
QList<Kopete::Message> HistoryLogger::readMessages(QDate date)
{
    kDebug() << "\n\nHistorylogger.cpp readMessages(QDate) \n\n";
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



//is called every time a new instance of chat is initiated.
QList<Kopete::Message> HistoryLogger::readMessages(int lines,
        const Kopete::Contact *c, Sens sens, bool reverseOrder, bool colorize)
{
    kDebug() << "***Historylogger.cpp readMessages2\n (Kopete::Contact *c, Sens sens, bool reverseOrder, bool colorize) \n\n";
    //QDate dd =  QDate::currentDate().addMonths(0-m_currentMonth);

    QList<Kopete::Message> messages;

    // A regexp useful for this function
    QRegExp rxTime("(\\d+) (\\d+):(\\d+)($|:)(\\d*)"); //(with a 0.7.x compatibility)

    if (!m_metaContact)
    { //this may happen if the contact has been moved, and the MC deleted
        if (c && c->metaContact())
            m_metaContact=c->metaContact();
        else
            return messages;
    }

    if (c && !m_metaContact->contacts().contains(const_cast<Kopete::Contact*>(c)) )
        return messages;

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

    //getting the color for messages:
    QColor fgColor = HistoryConfig::history_color();

    //i need both of these as private members
    const Kopete::Contact *currentContact=c;

    if (!c && m_metaContact->contacts().count()==1)
        currentContact=m_metaContact->contacts().first();
    
    else if (!c && m_metaContact->contacts().count()== 0)
    {
        return messages;
    }

    while (messages.count() < lines)
    {
	History history;
	//enter this only when no of meta contacts is greaterthan one. TODO need to check this
        if (!c && m_metaContact->contacts().count()>1)
        { //we have to merge the differents subcontact history
            QList<Kopete::Contact*> ct=m_metaContact->contacts();

            foreach(Kopete::Contact *contact, ct)
            { //we loop over each contact. we are searching the contact with the next message with the smallest date,
                // it will becomes our current contact, and the contact with the mext message with the second smallest
                // date, this date will bocomes the limit.
                kDebug() << "for each loop in action";

		History his;
                if (m_currentElements.contains(contact))
                    his=m_currentElements[contact];
                else  //there is not yet "next message" register, so we will take the first  (for the current month)
                {
                    kDebug() << "calling getDocument c,date";
                    his=getHistory(contact,m_currentMonth);
                    kDebug() <<"else part";
                }
                if ( !his.messages().isEmpty() )
                {
                    kDebug() << " !his.messages.empty ";
		    rxTime.indexIn( ((sens== Chronological)? his.messages().first().timestamp().toString("d h:m:s") 
							     : his.messages().last().timestamp().toString("d h:m:s")) );
		    QDate d=QDate::currentDate().addMonths(0-m_currentMonth);
		    QDateTime dt( QDate(d.year() , d.month() , rxTime.cap(1).toUInt()), QTime( rxTime.cap(2).toUInt() , rxTime.cap(3).toUInt(), rxTime.cap(5).toUInt()  ) );
		    if (!timestamp.isValid() || ((sens==Chronological )? dt < timestamp : dt > timestamp) )
		    {
                            history=his;
                            currentContact=contact;
                            timeLimit=timestamp;
                            timestamp=dt;			    
		    }
		    else if (!timeLimit.isValid() || ((sens==Chronological) ? timeLimit > dt : timeLimit < dt) )
                    {
                            timeLimit=dt;
		    }
		    
		    break;
                }
            }
        }
        else  //we don't have to merge the history. just take the next item in the contact
        {
            kDebug() << "entered else part as expected";
            if (m_currentElements.contains(currentContact)) //TODO need to take a look in this if
                history=m_currentElements[currentContact];
            else
            {
                kDebug() << "m_current element dosent contain the currentcontact";
		kDebug() << " executing getHisotry";
		History his=getHistory(currentContact,m_currentMonth);
		index =0;
                if ( !his.messages().isEmpty())
		{
		  kDebug() << " his.messages is not empty,inserting into current elements";
		  m_currentElements[currentContact]=his;
		  history=his;
		}
		else kDebug() << "count==1, !his.message.empty";
		
		kDebug() << " exitinf else part";
            }
	    kDebug() <<" end of else part";
        }
	
	kDebug()  <<" hellllllllllllll";
//this is the point where the esearch is over, no we feed onto our search
//TODO will do this later. not required at the moment.
//we don't find ANY messages in any contact for this month. so we change the month
        kDebug() << "before change month index ="<<index;
        if (history.messages().isEmpty() || index >= history.messages().count() || index < 0) 
	{   if (index <0 ) kDebug() <<"inside  change month, index <0";
            if (sens==Chronological)
            {	kDebug() << "m_currentmonth="<<m_currentMonth;
                if (m_currentMonth <= 0)
		{
                    kDebug() <<"chrono his.messages.isempty current month="<<m_currentMonth;
		    break;
		}
		setCurrentMonth(m_currentMonth-1);
		
            }
            else
            {
                if (m_currentMonth >= getFirstMonth(c))
                {
		  kDebug() << "m_current month="<<m_currentMonth << " getfirstmonth="<<getFirstMonth(c)<<"breaking";
                    break; //we don't have any other messages to show
                }
                kDebug() <<"inchrono else currntmonth="<<m_currentMonth;
		setCurrentMonth(m_currentMonth+1);
		
            }
            kDebug()<<" continuing form the top";
	    continue; //begin the loop from the bottom, and find currentContact and timeLimit again
	    
        }


//this is another border line. //in place of !msgElem.isnull() 
      kDebug() << "\n\n****before foreach loop timestamp="<<timestamp <<"timelimit="<<timeLimit;
      kDebug() <<"reverse order="<<reverseOrder;
      kDebug() << " sens="<<Chronological;
      kDebug() <<history.messages().count();
//      reverseOrder = !reverseOrder;
      History::Message msgElem;
      kDebug() << "index="<<index;
      
      if  (index == 0 )
	(sens!=Chronological)? index = history.messages().count() -1 : index=0 ;
      
      for ( index ;
	  (sens==Chronological)? index>=0: index < history.messages().count()  ;
	  (sens=Chronological)? index-- : index++ 
	  )
      {
	kDebug()<<" entered for each loop index=="<< index;
	msgElem = history.messages().at(index);
	kDebug() << " before if";
	if ( !(messages.count() < lines) ) break;
	
        if (
            (messages.count() < lines) &&
            !history.messages().isEmpty() &&
            (!timestamp.isValid() || !timeLimit.isValid() ||
             ((sens==Chronological) ? timestamp <= timeLimit : timestamp >= timeLimit)
            ))
        {
            // break this loop, if we have reached the correct number of messages,
            // if there are no more messages for this contact, or if we reached
            // the timeLimit msgElem is the next message, still not parsed, so
            // we parse it now
            kDebug() << " entered if part";

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
		    kDebug() << "timestamp of message="<<timestamp;
		   
                    Kopete::Message msg(from, to);
                    msg.setTimestamp( timestamp );
                    msg.setDirection( dir );
                    msg.setPlainBody( msgElem.text() );
                    if (colorize)
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

                    if (reverseOrder)
                        messages.prepend(msg);
                    else
                        messages.append(msg);
                }
            }
	    
	    
	    if (!c && (m_metaContact->contacts().count() > 1))
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
	    
	} else kDebug() << "didnt enter the if part :(" ;
      } kDebug() << " end of foreach loop, permessage in message list";
    }
    if (messages.count() < lines)
        m_currentElements.clear(); //current elements are null this can't be allowed
  
	kDebug() << "index="<<index;
    return messages;
}



unsigned int HistoryLogger::getFirstMonth(const Kopete::Contact *c)
{
    kDebug() << "**Historylogger.cpp getfirstmonth(contact) \n\n";
    if (!c)
        return getFirstMonth();

    bool collfound=false;
    Akonadi::Collection coll,collcontact;
    if ( !m_baseCollection.isValid() )
    {
      Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::FirstLevel );
      if ( job->exec()  )
      {
	  qDebug()<<" collection fetch job for root ececuted";
	  Akonadi::Collection::List collections = job->collections();
	  foreach( const Akonadi::Collection &collection, collections )
	  {
	      if ( collection.name() == "kopeteChat" )
	      {
                coll = collection;
		m_baseCollection = collection;
	      }
	  }
      } else qDebug() << "collection fetch job not executed";
    }else kDebug() << "root collection is lareadythere";
  
    if ( m_tosaveInCollection.name() != c->contactId() || !m_tosaveInCollection.isValid())
    {
      //fetch the collection which belongs to my contact
      Akonadi::CollectionFetchJob *job2 = new Akonadi::CollectionFetchJob( coll , Akonadi::CollectionFetchJob::FirstLevel );
      if ( job2->exec()  )
      {
	  Akonadi::Collection::List collections = job2->collections();
	  foreach( const Akonadi::Collection &collection, collections )
	  {
	      if ( collection.name() == c->contactId() )
	      {
		  collfound = true;
		  collcontact = collection;
		  m_tosaveInCollection= collcontact;
		  kDebug() << " collection for this contact already exists";
		  break;
	      }
	  }
      } else kDebug() << "collection fetch job not executed";
    } else kDebug() << "m_tosaveincollection name error/already exists collectioname="<< m_tosaveInCollection.name();

    if (m_tosaveInCollection.name() == c->contactId() ) collfound = true;
    
  if (collfound)
  {
    Akonadi::Item itemx;
    Akonadi::ItemFetchJob *job2 = new Akonadi::ItemFetchJob( m_tosaveInCollection );
    if ( job2->exec() )
    {
        kDebug() << " in get month. job2 executed to to calculate resule";
        Akonadi::Item::List items = job2->items();
        if ( items.isEmpty() )
            return 0;
        itemx=items.first();
        foreach( const Akonadi::Item &item, items )
        {
            if ( item.modificationTime() <= itemx.modificationTime() )
                itemx=item;
        }
        int result = 12*(QDate::currentDate().year() - itemx.modificationTime().date().year()) + QDate::currentDate().month() - itemx.modificationTime().date().month();
        if (result < 0)
        {
            kWarning(14310) << "Kopete only found log file made in the future. Check your date!";
            // break;
        }
        kDebug() << " returning result ="<<result;;
        return result;
    } else kDebug() << "item fetch job failed";
  }
    return 0;
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
    if ( !m_baseCollection.isValid() )
    {
        kDebug() << " invalid base collection lets find the valid one";
        Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::FirstLevel );
        if ( job->exec()  )
        {
            qDebug()<<" collection fetch job for root ececuted";
            Akonadi::Collection::List collections = job->collections();
            foreach( const Akonadi::Collection &collection, collections )
            {
                if ( collection.name() == "kopeteChat" )
                    m_baseCollection = collection;
            }
        }
        else qDebug() << "collection fetch job not executed";
    }

    foreach(Kopete::Contact *contact, contacts)
    {
        if ( !m_baseCollection.isValid() ) kDebug() << "invalid base collection";

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
                            if ( item.modificationTime().toString("yyyyMM") == date.toString("yyyyMM") )
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

QList<History> HistoryLogger::getHistorylist(const Kopete::Contact* c, QDate date)
{
  QList<History> listHistory;

  Akonadi::Collection coll,collcontact;
      Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::FirstLevel );
      if ( job->exec()  )
      {
	  qDebug()<<" collection fetch job for root ececuted";
	  Akonadi::Collection::List collections = job->collections();
	  foreach( const Akonadi::Collection &collection, collections )
	  {
	      if ( collection.name() == "kopeteChat" )
	      {
                coll = collection;
	      }
	  }
      } else qDebug() << "collection fetch job not executed";

    bool collfound= false;
  
      //fetch the collection which belongs to my contact
      Akonadi::CollectionFetchJob *job2 = new Akonadi::CollectionFetchJob( coll , Akonadi::CollectionFetchJob::FirstLevel );
      if ( job2->exec()  )
      {
	  Akonadi::Collection::List collections = job2->collections();
	  foreach( const Akonadi::Collection &collection, collections )
	  {
	      if ( collection.name() == c->contactId() )
	      {
		  collfound = true;
		  collcontact = collection;
		  kDebug() << " collection for this contact already exists";
		  break;
	      }
	  }
      } else qDebug() << "collection fetch job not executed";  

  if(collfound= true)
  {
    Akonadi::ItemFetchJob *itemjob = new Akonadi::ItemFetchJob( collcontact );
    itemjob->fetchScope().fetchFullPayload();
    if ( itemjob->exec() )
    {
        Akonadi::Item::List items = itemjob->items();
        if ( !items.isEmpty() )
        {
	    History history;
            kDebug() << "item list is not empty";
            foreach( const Akonadi::Item &item, items )
            {
	      if (item.hasPayload<History>() )
	      {
		     history = item.payload< History >();
	      }
	      listHistory.append(history);
	    }
	}
        else
        {
            kDebug() << " item list empty. ";
	}
    }
    else kDebug() << "item fetch job failed";
  } kDebug() <<"collection not found";
  
  return listHistory;

}




#include "historylogger.moc"