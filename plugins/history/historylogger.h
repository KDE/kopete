/*
    historylogger.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qobject.h>
#include "kopetemessage.h" //TODO: REMOVE

class KopeteContact;
class KopeteMetaContact;
class QFile;
class QDomDocument;

/**
 * One hinstance of this class is opened for every KopeteMessageManager,
 * or for the history dialog
 *
 * @author Olivier Goffart
 */
class HistoryLogger : public QObject
{
Q_OBJECT
public:
//	HistoryLogger( KopeteMetaContact *m , QObject *parent = 0, const char *name = 0);
	HistoryLogger( KopeteContact *c , QObject *parent = 0, const char *name = 0);

	~HistoryLogger();

	// display the log from -start-
	void readLog( int start , int count );

	// count the amount of messages in the log
	int countMessages();

	int totalMessages() {return messages;}

	int currentPos() { return m_currentPos; }

	bool reversed() { return m_reversed; }
	bool hideOutgoing() { return m_hideOutgoing; }

	void setReversed(bool);
	void setHideOutgoing(bool);

private:
//	QFile m_file;
	// xml stuff
	QDomDocument *xmllist;

	// total amount of messages in the log
	int messages;

	int m_currentPos;
	bool m_hideOutgoing;
	bool m_reversed;

	QString m_logFileName;


signals:
	void addMessage(KopeteMessage::MessageDirection dir, QString nick, QString date, QString body);



};

#endif
