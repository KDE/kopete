/*
    statisticscontact.cpp

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@yahoo.fr>


    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"

#include "statisticscontact.h"
#include "statisticsdb.h"


StatisticsContact::StatisticsContact(Kopete::MetaContact *mc, StatisticsDB *db) : m_metaContact(mc),m_db(db), m_oldStatus(Kopete::OnlineStatus::Unknown)
{
	m_isChatWindowOpen = false;
	m_oldStatusDateTime = QDateTime::currentDateTime();

	commonStatsCheck("timebetweentwomessages", m_timeBetweenTwoMessages,	 m_timeBetweenTwoMessagesOn, 0, -1);
	commonStatsCheck("messagelength", m_messageLength, m_messageLengthOn);
	
	// Check for last talk
	QString lastTalk;
	QString dummy = "";
	commonStatsCheck("lasttalk", lastTalk, dummy);
	if (lastTalk.isEmpty())
		m_lastTalk = QDateTime::QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
	else
		m_lastTalk = QDateTime::fromString(lastTalk);
	
	
	// Get last time a message was received
	m_lastMessageReceived = QDateTime::currentDateTime();
}

/**
 * \brief saves contact statistics
 */
StatisticsContact::~StatisticsContact()
{
	kdDebug() << "statistics: save commonstats for " << metaContact()->displayName() << endl;
	commonStatsSave("timebetweentwomessages",QString::number(m_timeBetweenTwoMessages), 
													QString::number(m_timeBetweenTwoMessagesOn));	 
	commonStatsSave("messagelength",QString::number(m_messageLength), QString::number(m_messageLengthOn)); 
	commonStatsSave("lasttalk", m_lastTalk.toString(), "");
}

/// @todo Only do this if there was a change
void StatisticsContact::commonStatsSave(const QString name, const QString statVar1, const QString statVar2)
{
	m_db->query(QString("UPDATE commonstats SET statvalue1 = '%1', statvalue2='%2'"
			"WHERE statname LIKE '%3' AND metacontactid LIKE '%4';").arg(statVar1).arg(statVar2).arg(name).arg(metaContact()->metaContactId()));
	
}

void StatisticsContact::commonStatsCheck(const QString name, int& statVar1, int& statVar2, const int defaultValue1, const int defaultValue2)
{
	QString a = QString::number(statVar1);
	QString b = QString::number(statVar2);
	
	commonStatsCheck(name, a, b, QString::number(defaultValue1), QString::number(defaultValue2));
	
	statVar1 = a.toInt();
	statVar2 = b.toInt();
}

void StatisticsContact::commonStatsCheck(const QString name, QString& statVar1, QString& statVar2, const QString defaultValue1, const QString defaultValue2)
{
	QStringList buffer = m_db->query(QString("SELECT statvalue1,statvalue2 FROM commonstats WHERE statname LIKE '%1' AND metacontactid LIKE '%2';").arg(name, metaContact()->metaContactId()));
	if (!buffer.isEmpty())
	{
		statVar1 = buffer[0];
		statVar2 = buffer[1];
	}
	else
	{

		m_db->query(QString("INSERT INTO commonstats (metacontactid, statname, statvalue1, statvalue2) VALUES('%1', '%2', 0, 0);").arg(metaContact()->metaContactId(), name));
		statVar1 = defaultValue1;
		statVar2 = defaultValue2;
	}
}

/**
 * \brief records informations from the new message
 *
 * Currently it does :
 * <ul>
 *	<li>Recalculate the average time between two messages
 *	It should only calculate this time if a chatwindow is open (sure, it isn't 
 *	perfect, because we could let a chatwindow open a whole day but that's at this *	time the nicest way, maybe we could check the time between the two last messages
 *	and if it is greater than, say, 10 min, do as there where no previous message) 
 *	So we do this when the chatwindow is open. We don't set m_isChatWindowOpen to true
 *	when a new chatwindow is open, but when a new message arrives. However, we set it 
 *	to false when the chatwindow is closed (see StatisticsPlugin::slotViewClosed).
 *
 *	Then it is only a question of some calculations.
 *
 * 	<li>Recalculate the average message lenght
 * 
 *	<li>Change last-talk datetime
 * </ul>
 *
 */
void StatisticsContact::newMessageReceived(Kopete::Message& m)
{
	kdDebug() << "statistics: new message received" << endl;
	QDateTime currentDateTime = QDateTime::currentDateTime();
	
	if (m_timeBetweenTwoMessagesOn != -1 && m_isChatWindowOpen)
	{
		m_timeBetweenTwoMessages = (m_timeBetweenTwoMessages*m_timeBetweenTwoMessagesOn + m_lastMessageReceived.secsTo(currentDateTime))/(1 + m_timeBetweenTwoMessagesOn);

	}
	
	setIsChatWindowOpen(true);
	
	m_timeBetweenTwoMessagesOn += 1;
	m_lastMessageReceived = currentDateTime;
	
	
	// Message lenght
	m_messageLength= (m.plainBody().length() + m_messageLength * m_messageLengthOn)/(1 + m_messageLengthOn);
	m_messageLengthOn++;
	
	// Last talked
	/// @todo do this in message sent too. So we need setLastTalk()
	m_lastTalk = currentDateTime;
}

/**
 * \brief Update the database for this contact when required.
 */
void StatisticsContact::onlineStatusChanged(Kopete::OnlineStatus::StatusType status)
{
	QDateTime currentDateTime = QDateTime::currentDateTime();


	
	
	/// We don't want to log if oldStatus is unknown
	/// the change could not be a real one; see StatisticsPlugin::slotMySelfOnlineStatusChanged
	if (m_oldStatus != Kopete::OnlineStatus::Unknown)
	{
		
		kdDebug() << "statistics - status change for "<< metaContact()->displayName() << " : "<< QString::number(m_oldStatus) << endl;
		m_db->query(QString("INSERT INTO contactstatus "
		"(metacontactid, status, datetimebegin, datetimeend, selfbegin, selfend) "
		"VALUES('%1', '%2', '%3', '%4', '%5'," "'%6');").arg(m_metaContact->metaContactId()).arg(Kopete::OnlineStatus::statusTypeToString(m_oldStatus)).arg(m_oldStatusDateTime.toString()).arg(currentDateTime.toString()).arg(QString::number(1)).arg(QString::number(1)));
	}
	
	m_oldStatus = status;
	m_oldStatusDateTime = currentDateTime;

}

// QDateTime StatisticsContact::nextOfflineEvent()
// {
// 	return nextEvent(Kopete::OnlineStatus::Offline);
// }
// 
// QDateTime StatisticsContact::nextOnlineEvent()
// {
// 	return nextEvent(Kopete::OnlineStatus::Online);
// }

// QDateTime StatisticsContact::nextEvent(const Kopete::OnlineStatus::StatusType& status)
// {
// 	QStringList values;
// 	QDateTime currentDateTime = QDateTime::currentDateTime();
// 	values = m_db->query(QString("SELECT datetimebegin, datetimeend, status FROM contactstatus WHERE metacontactid LIKE '%1' ORDER BY datetimebegin").arg(metaContact()->metaContactId()));
// 	
// 	/// @todo Problem here with the ORDER BY because it works here on strings, not on dates
// 	/// So this doesn't work for now.
// 		
// 	int secsTo = 0;
// 	int nb = 0;
// 	for(int i=0; i < values.count(); i+=3)
// 	{
// 		kdDebug() << "statistics: " << currentDateTime.time().toString();
// 		kdDebug() << "  statistics: " << QTime::fromString(values[i]).toString();
// 		kdDebug() << "  statistics: " << QTime::fromString(values[i+1]).toString();
// 		if (currentDateTime.time() > QTime::fromString(values[i]) && currentDateTime.time() < QTime::fromString(values[i+1])
// 		/*&& currentDateTime.date().dayOfWeek() == QDate::fromString(values[i]).dayOfWeek() */ /// @todo temporary removed for tests
// 		)
// 		// We match the status like the current status and at the same time, and the same day of week, because on sundays, I 
// 		// do not connect like on tuesdays.
// 		{
// 			kdDebug() << "statistics: " << currentDateTime.time().toString();
// 			/// Then we look for the fist \p status (method parameter) event
// 			int j = i+3;
// 			while(j < values.count())
// 			{
// 				if (values[j+3] == Kopete::OnlineStatus::statusTypeToString(status))
// 				{
// 					
// 					// Not really good for events at about midnight
// 					secsTo = (nb*secsTo + currentDateTime.time().secsTo(QDateTime::fromString(values[j]).time()))/(nb+1);
// 					nb++;
// 					break;
// 				}
// 				
// 				j+=3;
// 			}
// 		}
// 	}
// 	
// 	return currentDateTime.addSecs(secsTo);
// }