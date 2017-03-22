/*
    history2logger.cpp

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
#include <QSqlDatabase>
#include <QMutex>

class QDate;
class QTimer;

class DMPair;

namespace Kopete {
class Message;
class Contact;
class Account;
class MetaContact;
}

/**
 * One hinstance of this class is opened for every Kopete::ChatSession,
 * or for the history2 dialog
 *
 * @author Olivier Goffart <ogoffart@kde.org>
 */
class History2Logger : public QObject {
	Q_OBJECT
public:

	static History2Logger* instance() {
		static QMutex mutex;
		if (!m_Instance) {
			mutex.lock();

			if (!m_Instance)
				m_Instance = new History2Logger();

			mutex.unlock();
		}

		return m_Instance;
	}

	static void drop() {
		static QMutex mutex;
		mutex.lock();
		delete m_Instance;
		m_Instance = 0;
		mutex.unlock();
	}

	/**
	 * @return The list of the days for which there is a log for m_metaContact for month of
	 * @param date (don't care of the day)
	 */
	QList<QDate> getDays(const Kopete::MetaContact *c, QString search = "" );

	QList<DMPair> getDays(QString search = "");


	/**
	 * log a message
	 * @param c add a presision to the contact to use, if null, autodetect.
	 */
	void appendMessage( const Kopete::Message &msg , const Kopete::Contact *c=0L, bool skipDuplicate = false);

	bool messageExists( const Kopete::Message &msg , const Kopete::Contact *c=0L);

	void beginTransaction();
	void commitTransaction();

	/**
	 * read @param lines message from the current position
	 * from Kopete::Contact @param c in the given @param sens
	 */
	QList<Kopete::Message> readMessages(int lines,
	                                    int offset=0, const Kopete::MetaContact *c=NULL, bool reverseOrder=true);

	/**
	 * Same as the following, but for one date. I did'nt reuse the above function
	 * because its structure is really different.
	 * Read all the messages for the given @param date
	 */
	QList<Kopete::Message> readMessages(QDate date, const Kopete::MetaContact *c=0);



private:

	History2Logger();

	History2Logger(const History2Logger &); // hide copy constructor
	History2Logger& operator=(const History2Logger &); // hide assign op
	~History2Logger();

	static History2Logger* m_Instance;
	QSqlDatabase m_db;

};

#endif
