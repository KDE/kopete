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

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>
//#include <QtXml/QDomDocument>
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
    explicit HistoryLogger(Kopete::MetaContact *m ,QHash<QString,Akonadi::Collection> &collMap, QObject *parent = 0 );
    explicit HistoryLogger(Akonadi::Collection &coll, Kopete::Contact *c , QObject *parent = 0 );
//	explicit HistoryLogger(QObject *parent=0);

//	void Initialize(Kopete::MetaContact *m ,QHash<QString,Akonadi::Collection> &collMap);

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
    void appendMessage( const Kopete::Message &msg , Akonadi::Collection &coll,Akonadi::Collection &baseColl, const Kopete::Contact *c=0L  );

    /**
     * read @param lines message from the current position
     * from Kopete::Contact @param c in the given @param sens
     */
    void readMessages(int lines, Akonadi::Collection &coll,
                      const Kopete::Contact *c=0, Sens sens=Default,
                      bool reverseOrder=false, bool colorize=true);

    QList<Kopete::Message> retrunReadMessages();

    /**
     * Same as the following, but for one date. I did'nt reuse the above function
     * because its structure is really different.
     * Read all the messages for the given @param date
     */
//	QList<Kopete::Message>
    void readMessages(QDate date);

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
    void getDaysForMonth(QDate date);

    /**
     * Get the filename of the xml file which contains the history from the
     * contact in the specified @param date. Specify @param date in order to get the filename for
     * the given date.year() date.month().
     */
//	static QString getFileName(const Kopete::Contact* , QDate date);
    void ModifyItem(HistoryLogger *historylogger);

private:
    bool m_hideOutgoing;
    Qt::CaseSensitivity m_filterCaseSensitive;
    bool m_filterRegExp;
    QString m_filter;


    /*
     *contais all QDomDocument, for a KC, for a specified Month
     */
    QMap<const Kopete::Contact*,QMap<unsigned int, History> > m_history;
//	History m_historyx;

//	QMap<const Kopete::Contact*,QMap<unsigned int , QDomDocument> > m_documents;

    /**
     * Contains the current message.
     * in fact, th
     {
       is is the next, still not showed
     */
//	QMap<const Kopete::Contact*, QDomElement>  m_currentElements;
    QMap<const Kopete::Contact*, History>  m_currentElements;

    /**
     * Get the document, open it is @param canload is true, contain is set to false if the document
     * is not already contained
     */
//	QDomDocument getDocument(const Kopete::Contact *c, unsigned int month , bool canLoad=true , bool* contain=0L);
//	QDomDocument getDocument(const Kopete::Contact *c, const QDate date, bool canLoad=true, bool* contain=0L);
    void getHistoryx(const Kopete::Contact *c, unsigned int month, bool canLoad=true , bool* contain=0L);
//	History getHistory(const Kopete::Contact *c, unsigned int month , bool canLoad=true , bool* contain=0L);
//	History getHistory(const Kopete::Contact *c, const QDate date, bool canLoad=true, bool* contain=0L);

    /**
     * look over files to get the last month for this contact
     */
    unsigned int getFirstMonth(const Kopete::Contact *c);
    unsigned int getFirstMonth();
    /*
     * the current month
     */
    unsigned int m_currentMonth;

    /*
     * the cached getFirstMonth
     */
    int m_cachedMonth;
    /*
     * the metacontact we are using
     */
    Kopete::MetaContact *m_metaContact;

    /*
     * keep the old position in memory, so if we change the sens, we can begin here
     */
    QMap<const Kopete::Contact*, History>  m_oldElements;
    unsigned int m_oldMonth;
    Sens m_oldSens;

    // the two time variables i have used in the getmessages
    QDateTime timeLimit,timestamp;
    int index;

    /**
     * the timer used to save the file
     */
    QTimer *m_saveTimer;
//	QDomDocument m_toSaveDocument;
    //roide
    History m_toSaveHistory;
//	QString m_toSaveFileName;
    unsigned int m_saveTimerTime; //time in ms between each save

    Akonadi::Item m_tosaveInItem;
    Akonadi::Collection m_tosaveInCollection;
    static Akonadi::Collection m_baseCollection;
    QHash<QString, Akonadi::Collection> m_collectionMap;
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
    QMap< Kopete::Contact *, History> m_contact_history;

    //for the getmonth function
    unsigned int m_result;

    //in modifyitem
    QTime m_t;
    //in readMessages(qdate)
//	QList<History> m_fetchedHistories;
    QDate m_readMessagesDate;
    QList<Kopete::Message> m_readMessagesDateList;
    QHash<Kopete::Contact *,History> m_historyContact;

    //used in getdays for month
    QList<History> m_historyList;

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

    void getJobDoneSlot(KJob*);
    void appendMessage2();

    void getHistoryForMetacontacts();;
    void GetJobDoneInReadMessage2Done(KJob* job);
    void transaction_in_read_message_block_2_done(KJob*);
    
    void getHistoryForGivenContact();
    void getHistoryDone();
    
    void changeMonth();
    void changeMonth2();
    
    void readMessagesDone();

    void getMonthSlot(Akonadi::Item::List);

    void collectionCreateDone(KJob*);
    void itemCreateDone(KJob*);
    void itemsReceivedDone(Akonadi::Item::List);
    void itemModifiedDone(KJob*);

    //slot for readmessages(date)
    void getHistoryJobDone(KJob*);
    void transactionDone(KJob* );

    //for getDaysForMonth
    void getDaysForMonthSlot(KJob*);

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
