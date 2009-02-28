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

#ifndef HISTORYLOGGER_H
#define HISTORYLOGGER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtXml/QDomDocument>

class QDate;
class QTimer;

namespace Kopete { class Message; }
namespace Kopete { class Contact; }
namespace Kopete { class MetaContact; }

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
	explicit HistoryLogger( Kopete::MetaContact *m , QObject *parent = 0 );
	explicit HistoryLogger( Kopete::Contact *c , QObject *parent = 0 );


	~HistoryLogger();

	/**
	 * return or setif yes or no outgoing message are hidden (and not parsed)
	 */
	bool hideOutgoing() const { return m_hideOutgoing; }
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
	void appendMessage( const Kopete::Message &msg , const Kopete::Contact *c=0L );

	/**
	 * read @param lines message from the current position
	 * from Kopete::Contact @param c in the given @param sens
	 */
	QList<Kopete::Message> readMessages(int lines,
		const Kopete::Contact *c=0, Sens sens=Default,
		bool reverseOrder=false, bool colorize=true);

	/** 
	 * Same as the following, but for one date. I did'nt reuse the above function
	 * because its structure is really different.
	 * Read all the messages for the given @param date
	 */
	QList<Kopete::Message> readMessages(QDate date);

	/**
	 * The pausition is set to the last message
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
	QList<int> getDaysForMonth(QDate date);

	/**
	 * Get the filename of the xml file which contains the history from the
	 * contact in the specified @param date. Specify @param date in order to get the filename for
	 * the given date.year() date.month().
	 */
	static QString getFileName(const Kopete::Contact* , QDate date);

private:
	bool m_hideOutgoing;
	Qt::CaseSensitivity m_filterCaseSensitive;
	bool m_filterRegExp;
	QString m_filter;


	/*
	 *contais all QDomDocument, for a KC, for a specified Month
	 */
	QMap<const Kopete::Contact*,QMap<unsigned int , QDomDocument> > m_documents;

	/**
	 * Contains the current message.
	 * in fact, this is the next, still not showed
	 */
	QMap<const Kopete::Contact*, QDomElement>  m_currentElements;

	/**
	 * Get the document, open it is @param canload is true, contain is set to false if the document
	 * is not already contained
	 */
	QDomDocument getDocument(const Kopete::Contact *c, unsigned int month , bool canLoad=true , bool* contain=0L);

	QDomDocument getDocument(const Kopete::Contact *c, const QDate date, bool canLoad=true, bool* contain=0L);

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
	QMap<const Kopete::Contact*, QDomElement>  m_oldElements;
	unsigned int m_oldMonth;
	Sens m_oldSens;
	 
	 /**
	  * the timer used to save the file
	  */ 
	QTimer *m_saveTimer;
	QDomDocument m_toSaveDocument;
	QString m_toSaveFileName;
	unsigned int m_saveTimerTime; //time in ms between each save
	
	/**
	 * workaround for the 31 midnight bug.
	 * it contains the number of the current month.
	 */
	int m_realMonth;

	/*
	 * FIXME:
	 * WORKAROUND
	 * due to a bug in QT, i have to keep the document element in the memory to
	 * prevent crashes
	 */
	QList<QDomElement> workaround;

private slots:
	/**
	 * the metacontact has been deleted
	 */
	void slotMCDeleted();
	
	/**
	 * save the current month's document on the disk. 
	 * connected to the m_saveTimer signal
	 */
	void saveToDisk();
};

#endif
