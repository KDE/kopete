/*
    historylogger.cpp

    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@kde.org>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>
    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef HISTORYLOGGER_H
#define HISTORYLOGGER_H

#include "historyplugin.h"
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <akonadi/item.h>
#include <Akonadi/Collection>
#include <QDateTime>
#include "kopetecontact.h"
#include "kopetemessage.h"
#include <history/history.h>
#include <KJob>

class QDate;
class QTimer;

namespace Kopete {
class Message;
}
namespace Kopete {
class Contact;
}
namespace Kopete {
class MetaContact;
}


/**
 * One hinstance of this class is opened for every Kopete::ChatSession,
 * or for the history dialog
 *
 * @author Olivier Goffart <ogoffart@kde.org>
 */
class HistoryLogger : public QObject
{
    Q_OBJECT
public:

    /**
     * - Chronological: messages are read from the first to the last, in the time order
     * - AntiChronological: messages are read from the last to the first, in the time reversed order
     */
    enum Sens { Default , Chronological , AntiChronological };

    /**
     * Constructor, takes the contact, and the color of messages
     */
    explicit HistoryLogger(Kopete::MetaContact *m , QObject *parent = 0, QObject *hPlugin = 0 );
    explicit HistoryLogger(Kopete::Contact *c , QObject *parent = 0, QObject *hPlugin = 0 );

    ~HistoryLogger();

    /**
     * return or setif yes or no outgoing message are hidden (and not parsed)
     */
    bool hideOutgoing() const {
        return m_hideOutgoing;
    }
    void setHideOutgoing(bool);

    /**
     * set a searching  filter
     * @param filter is the string to search
     * @param caseSensitive say if the case is important
     * @param isRegExp say if the filter is a QRegExp, or a simle string
     */
    void setFilter(const QString& filter, bool caseSensitive=false , bool isRegExp=false);
    QString filter() const;
    bool filterCaseSensitive() const ;
    bool filterRegExp() const;



    //----------------------------------

    /**
     * log a message
     * @param c add a presision to the contact to use, if null, autodetect.
     */
    void appendMessage( const Kopete::Message &msg , const Kopete::Contact *c=0L  );

    /**
     * read @param lines message from the current position
     * from Kopete::Contact @param c in the given @param sens
     */
    void readMessages(int lines,
                      const Kopete::Contact *c=0, Sens sens=Default,
                      bool reverseOrder=false, bool colorize=true, bool quote=false);

    /**
     * Same as the following, but for one date. I did'nt reuse the above function
     * because its structure is really different.
     * Read all the messages for the given @param date
     */
    
    void readMessages(QDate date, int pos=0);

    /**
     * The position is set to the last message
     */
    void setPositionToLast();

    /**
     * The position is set to the first message
     */
    void setPositionToFirst();

    /**
     * Set the current month  (in number of month since the actual month)
     */
    void setCurrentMonth(int month);

    /**
     * @return The list of the days for which there is a log for m_metaContact for month of
     * @param date (don't care of the day)
     */
    void getDaysForMonth(QDate date, int pos=0);
    
private:
    bool m_hideOutgoing;
    Qt::CaseSensitivity m_filterCaseSensitive;
    bool m_filterRegExp;
    QString m_filter;


    /**
     *contais all History, for a KC, for a specified Month
     */
    
    QMap<const Kopete::Contact*,QMap<unsigned int, History> > m_history;
    
    /**
     * Get the document  //TODO: add comment
     */
    void getHistory(const Kopete::Contact *c, unsigned int month);
    void getHistory(const Kopete::Contact *c, const QDate date );

    /**
     * look over items fetched to get the last month for this contact
     */
    unsigned int getFirstMonth(const Kopete::Contact *c);
    unsigned int getFirstMonth();
    
    /**
     * the current month
     */
    unsigned int m_currentMonth;

    /**
     * the cached getFirstMonth
     */
    int m_cachedMonth;
    
    /**
     * the metacontact we are using
     */
    Kopete::MetaContact *m_metaContact;

    /**
    * //TODO Y DO WE NEED these 2 Variables ??? 
    */
    unsigned int m_oldMonth;
    Sens m_oldSens;

    /**
    * The two variables in readmesage method to save the timestamp.
    */
    //TODO:add comment to these 2 variables
    QDateTime timeLimit,m_timestamp;
    int m_index;

    /**
     * the timer used to save the file
     */
    QPointer<QTimer> m_saveTimer;
    
    /**
    * The history Object that stores the chat history
    */
    History m_toSaveHistory;

    unsigned int m_saveTimerTime; //time in ms between each save

    Akonadi::Item m_tosaveInItem;
    Akonadi::Collection m_tosaveInCollection;
    Akonadi::Collection m_parentCollection;
    Akonadi::Collection m_kopeteChat;
    /**
     * workaround for the 31 midnight bug.
     * it contains the number of the current month.
     */
    int m_realMonth;

    //for the append message function
    History m_getHistory;
    Kopete::Message m_message;
    const Kopete::Contact * m_contact;
    unsigned int m_month;

    //for the read message function
    QList<Kopete::Message> m_readmessages;
    History m_readmessagesHistory;
    int m_lines;
    const Kopete::Contact *m_readmessagesContact;
    const Kopete::Contact *m_currentContact;
    Sens m_readmessagesSens;
    bool m_reverseOrder;
    bool m_colorize;

    //used in read message block 2
    /**
    * When, history is fetched for meta contacts, the map that is used to save
    * the contact and history connection
    *
    * @author kaushik saurabh <roideuniverse@gmail.com>
    * IT IS A STUPID IDEA USING THIS VARIABLE, SO I AM REMOVING IT
    */
    QHash< Kopete::Contact *, History> m_contact_history;

    
    QTime m_t;
    //in readMessages(qdate)
//	QList<History> m_fetchedHistories;
    QDate m_readMessagesDate;
    QList<Kopete::Message> m_readMessagesDateList;

    //used in getdays for month
    QList<History> m_historyList;
    
    QPointer<HistoryPlugin> m_hPlugin;
        
    /**
    * when items are fetched from akonadi server, this saves the mapping between the 
    * contact and the list of items exist, for that contact.
    *
    */
    
    QHash<const Kopete::Contact * , Akonadi::Item::List> m_contactItemList;
    
    /**
    * used in the readmessages by date, to save the position of the contact, for which to fetch
    * the items from Akonadi
    */
    
    int m_pos;
    
    /**
    * used in the readmessages by GetdaysForMonth, to save the position of the contact, for which to fetch
    * the items from Akonadi
    */
    
    unsigned int m_pos_GetDaysForMonth;
    
     /**
    * used in the readmessages by date, to save the position of the contact, for which to fetch
    * the items from Akonadi
    */
    
    QList<int> m_dayList;
    QDate m_dateGetDaysForMonth;
    
    
   
    

private slots:
    /**
     * the metacontact has been deleted
     */
    void slotMCDeleted();

    /**
     * save the current month's document on the disk.
     * connected to the m_saveTimer signal
     */
    void modifyItem();

//    void getJobDoneSlot(KJob*);
    void appendMessage2();

    void getHistoryForMetacontacts();;
//    void GetJobDoneInReadMessage2Done(KJob* job);
    
    /**
    * when items are fetched will full payload in readmesage, for metacontacts
    */
    void fetchItemFullPayloadSlot(KJob*);
    
    /**
    * When in read messages, for metacontacts fetchs the items for all contacts
    * this slot is called, when that transaction is over
    */
    void transationsFetchItemsDone(KJob*);
    
    void transaction_in_read_message_block_2_done(KJob*);
    
    void getHistoryForGivenContact();
    void getHistoryDone();
    
    void changeMonth();
//    void changeMonth2();
    
    void readMessagesDone();

//    void getMonthSlot(Akonadi::Item::List);

    void collectionCreateDone(KJob*);
    void itemCreateDone(KJob*);
    void itemsReceivedDone(Akonadi::Item::List);
    void itemModifiedDone(KJob*);

    //slot for readmessages(date)
    void getHistoryJobDone();
    void transactionDone();

    //for getDaysForMonth
    void getDaysForMonthSlot();
    
    void createItem();
    
    void parentCollCreated(KJob*);
    
    
    void fetchItemHeaderSlot(KJob*);
    void itemFetchSlot(KJob*);
    
    /**
    * In the fetch job for metacontacts, slot to fetch items for a contacts
    */
    void itemFetchTestSlot(KJob*);

signals:
    void readMessagesDoneSignal(QList<Kopete::Message>);
    void appendMessageDoneSignal();
    void getHistoryxDone();
    void getfirstmonthwithcontactDoneSignal();

    void collectionCreatedSignal();

    void readMessagesByDateDoneSignal( QList<Kopete::Message> );

    void getDaysForMonthSignal(QList<int>);

};

Q_DECLARE_METATYPE(Kopete::Contact *)

#endif
