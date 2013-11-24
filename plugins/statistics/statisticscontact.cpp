/*
    statisticscontact.cpp

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@gmail.com>

    Copyright (c) 2007      by the Kopete Developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "statisticscontact.h"

#include <stdlib.h>

#include <q3valuelist.h>

#include <kdebug.h>
#include <klocale.h>

#include "kopetemetacontact.h"
#include "kopeteonlinestatus.h"

#include "statisticsdb.h"

StatisticsContact::StatisticsContact(Kopete::MetaContact *mc, StatisticsDB *db) : m_metaContact(mc), m_metaContactId(mc->metaContactId()), m_db(db), m_oldStatus(Kopete::OnlineStatus::Unknown)
{
	m_isChatWindowOpen = false;
	m_oldStatusDateTime = QDateTime::currentDateTime();

	// Last*Changed are always false at start
	m_timeBetweenTwoMessagesChanged  = false;
	m_lastTalkChanged = false;
	m_lastPresentChanged  = false;
	m_messageLengthChanged = false;

	m_timeBetweenTwoMessages = 0;
	m_timeBetweenTwoMessagesOn = 0;
	m_messageLength = 0;
	m_messageLengthOn = 0;

	commonStatsCheck("timebetweentwomessages", m_timeBetweenTwoMessages, m_timeBetweenTwoMessagesOn, 0, -1);
	commonStatsCheck("messagelength", m_messageLength, m_messageLengthOn, 0, 0);
	
	// Check for last talk
	QString lastTalk;
	QString dummy = "";
	commonStatsCheck("lasttalk", lastTalk, dummy);
	if (lastTalk.isEmpty())
	{
		m_lastTalk.setTime_t(0);
		m_lastTalkChanged = true;
	}
	else
		m_lastTalk = QDateTime::fromString(lastTalk);
	
	
	// Get last time a message was received
	m_lastMessageReceived = QDateTime::currentDateTime();


	// Check for lastPresent
	QString lastPresent = "";
	commonStatsCheck("lastpresent", lastPresent, dummy);
	if (lastPresent.isEmpty())
	{
		m_lastPresent.setTime_t(0);
		m_lastPresentChanged = true;
	}
	else
		m_lastPresent = QDateTime::fromString(lastPresent);
}

/**
 * \brief saves contact statistics
 */
StatisticsContact::~StatisticsContact()
{
	commonStatsSave("timebetweentwomessages",QString::number(m_timeBetweenTwoMessages), 
	QString::number(m_timeBetweenTwoMessagesOn), m_timeBetweenTwoMessagesChanged);	 
	commonStatsSave("messagelength",QString::number(m_messageLength), QString::number(m_messageLengthOn), m_messageLengthChanged); 
	commonStatsSave("lasttalk", m_lastTalk.toString(), "", m_lastTalkChanged);
	commonStatsSave("lastpresent", m_lastPresent.toString(), "", m_lastPresentChanged);
}

void StatisticsContact::commonStatsSave(const QString &name, const QString &statVar1, const QString &statVar2, const bool statVarChanged)
{
	// Only update the database if there was a change
	if (!statVarChanged) return;

	m_db->query(QString("UPDATE commonstats SET statvalue1 = '%1', statvalue2='%2'"
			"WHERE statname LIKE '%3' AND metacontactid LIKE '%4';").arg(statVar1).arg(statVar2).arg(name).arg(m_metaContactId));
	
}

void StatisticsContact::commonStatsCheck(const QString &name, int& statVar1, int& statVar2, const int defaultValue1, const int defaultValue2)
{
	QString a = QString::number(statVar1);
	QString b = QString::number(statVar2);
	
	commonStatsCheck(name, a, b, QString::number(defaultValue1), QString::number(defaultValue2));
	
	statVar1 = a.toInt();
	statVar2 = b.toInt();
}

void StatisticsContact::commonStatsCheck(const QString &name, QString& statVar1, QString& statVar2, const QString &defaultValue1, const QString &defaultValue2)
{
	QStringList buffer = m_db->query(QString("SELECT statvalue1,statvalue2 FROM commonstats WHERE statname LIKE '%1' AND metacontactid LIKE '%2';").arg(name, m_metaContactId));
	if (!buffer.isEmpty())
	{
		statVar1 = buffer[0];
		statVar2 = buffer[1];
	}
	else
	{
		m_db->query(QString("INSERT INTO commonstats (metacontactid, statname, statvalue1, statvalue2) VALUES('%1', '%2', 0, 0);").arg(m_metaContactId, name));
		statVar1 = defaultValue1;
		statVar2 = defaultValue2;
	}
}

/**
 * \brief records information from the new message
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
 * 	<li>Recalculate the average message length
 * 
 *	<li>Change last-talk datetime
 * </ul>
 *
 */
void StatisticsContact::newMessageReceived(Kopete::Message& m)
{
	kDebug(14315) << "statistics: new message received";
	QDateTime currentDateTime = QDateTime::currentDateTime();
	
	if (m_timeBetweenTwoMessagesOn != -1 && m_isChatWindowOpen)
	{
		m_timeBetweenTwoMessages = (m_timeBetweenTwoMessages*m_timeBetweenTwoMessagesOn + m_lastMessageReceived.secsTo(currentDateTime))/(1 + m_timeBetweenTwoMessagesOn);

	}
	
	setIsChatWindowOpen(true);
	
	m_timeBetweenTwoMessagesOn += 1;
	m_lastMessageReceived = currentDateTime;
	
	
	// Message length
	m_messageLength= (m.plainBody().length() + m_messageLength * m_messageLengthOn)/(1 + m_messageLengthOn);
	m_messageLengthOn++;
	
	// Last talked
	/// @todo do this in message sent too. So we need setLastTalk()
	m_lastTalk = currentDateTime;
	
	m_messageLengthChanged = true;
	m_lastTalkChanged = true;
	m_timeBetweenTwoMessagesChanged = true;
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
		
		kDebug(14315) << "statistics - status change for "<< m_metaContactId << " : "<< QString::number(m_oldStatus);
		m_db->query(QString("INSERT INTO contactstatus "
		"(metacontactid, status, datetimebegin, datetimeend) "
				"VALUES('%1', '%2', '%3', '%4'" ");").arg(m_metaContactId).arg(Kopete::OnlineStatus::statusTypeToString(m_oldStatus)).arg(QString::number(m_oldStatusDateTime.toTime_t())).arg(QString::number(currentDateTime.toTime_t())));
	}
	
	if (m_oldStatus == Kopete::OnlineStatus::Online
		|| m_oldStatus == Kopete::OnlineStatus::Away
		|| m_oldStatus == Kopete::OnlineStatus::Busy)
	// If the last status was Online or Away, the last time contact was present is the time he goes offline
	{
		m_lastPresent = currentDateTime;
		m_lastPresentChanged = true;
	}

	m_oldStatus = status;
	m_oldStatusDateTime = currentDateTime;

}

bool StatisticsContact::wasStatus(QDateTime dt, Kopete::OnlineStatus::StatusType status)
{
	QStringList values = m_db->query(QString("SELECT status, datetimebegin, datetimeend "
			"FROM contactstatus WHERE metacontactid LIKE '%1' AND datetimebegin <= %2 AND datetimeend >= %3 "
			"AND status LIKE '%4' "
			"ORDER BY datetimebegin;"
		).arg(m_metaContactId).arg(dt.toTime_t()).arg(dt.toTime_t()).arg(Kopete::OnlineStatus::statusTypeToString(status)));	
	
	if (!values.isEmpty()) return true;
	
	return false;
}

QString StatisticsContact::statusAt(QDateTime dt)
{
	QStringList values = m_db->query(QString("SELECT status, datetimebegin, datetimeend "
			"FROM contactstatus WHERE metacontactid LIKE '%1' AND datetimebegin <= %2 AND datetimeend >= %3 "
			"ORDER BY datetimebegin;"
			).arg(m_metaContactId).arg(dt.toTime_t()).arg(dt.toTime_t()));	
	
	if (!values.isEmpty()) return Kopete::OnlineStatus(Kopete::OnlineStatus::statusStringToType(values[0])).description();
	else return "";
}

QString StatisticsContact::mainStatusDate(const QDate& date)
{
	QDateTime dt1(date, QTime(0,0,0));
	QDateTime dt2(date.addDays(1), QTime(0,0,0));
	kDebug(14315) << "dt1:" << dt1.toString() << " dt2:" << dt2.toString();
	QString request = QString("SELECT status, datetimebegin, datetimeend, metacontactid "
			"FROM contactstatus WHERE metacontactid = '%1' AND "
			"(datetimebegin >= %2 AND datetimebegin <= %3 OR "
			"datetimeend >= %4 AND datetimeend <= %5) "
			"ORDER BY datetimebegin;"
							 ).arg(m_metaContactId).arg(dt1.toTime_t()).arg(dt2.toTime_t()).arg(dt1.toTime_t()).arg(dt2.toTime_t());
	kDebug(14315) << request;
	QStringList values = m_db->query(request);
	
	unsigned int online = 0, offline = 0, away = 0, busy = 0;
	for(int i=0; i<values.count(); i+=4)
	{
		unsigned int datetimebegin = values[i+1].toInt(), datetimeend = values[i+2].toInt();
		kDebug(14315) << "statistics: id "<< values[i+3]<< " status " << values[i] << " datetimeend " << QString::number(datetimeend) << " datetimebegin " << QString::number(datetimebegin);
		if (datetimebegin <= dt1.toTime_t()) datetimebegin = dt1.toTime_t();
		if (datetimeend >= dt2.toTime_t()) datetimeend = dt2.toTime_t();
		
		
		
		if (values[i]==Kopete::OnlineStatus::statusTypeToString(Kopete::OnlineStatus::Online))
			online += datetimeend - datetimebegin;
		else if (values[i]==Kopete::OnlineStatus::statusTypeToString(Kopete::OnlineStatus::Away))
			away += datetimeend - datetimebegin;
		else if (values[i]==Kopete::OnlineStatus::statusTypeToString(Kopete::OnlineStatus::Busy))
			busy += datetimeend - datetimebegin;
		else if (values[i]==Kopete::OnlineStatus::statusTypeToString(Kopete::OnlineStatus::Offline))
			offline += datetimeend - datetimebegin;
	}

	if (online > away && online > offline && online > busy) return i18n("Online");
	else if (away > online && away > offline && away > busy) return i18n("Away");
	else if (offline > online && offline > away && offline > busy) return i18n("Offline");
	else if (busy > online && busy > offline && busy > away) return i18n("Busy");
	
	return QString();
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
// 	
// }

QList<QTime> StatisticsContact::mainEvents(const Kopete::OnlineStatus::StatusType& status)
{
	QStringList buffer;
	QList<QTime> mainEvents;
	
	
	QDateTime currentDateTime = QDateTime::currentDateTime();
	buffer = m_db->query(QString("SELECT datetimebegin, datetimeend, status FROM contactstatus WHERE metacontactid LIKE '%1' ORDER BY datetimebegin").arg(m_metaContactId));
	
	
	// Only select the events for which the previous is not Unknown AND the status is status.
	QStringList values;
	for (int i=0; i<buffer.count(); i += 3)
	{
		if (buffer[i+2] == Kopete::OnlineStatus::statusTypeToString(status)
			&& abs(buffer[i+1].toInt()-buffer[i].toInt()) > 120)
		{
			values.push_back(buffer[i]);
		}
	}
	
	// No entries for this contact ...
	if (!values.count()) return mainEvents;

	// First we compute the average number of events/day : avEventsPerDay;
	int avEventsPerDay = 0;
	QDateTime dt1, dt2;
	dt1.setTime_t(values[0].toInt());
	dt2.setTime_t(values[values.count()-1].toInt());
	
	avEventsPerDay = qRound((double)values.count()/(double)dt1.daysTo(dt2));
	kDebug(14315) << "statistics: average events per day : " <<avEventsPerDay;
	
	// We want to work on hours
	QList<int> hoursValues;
	for (int i=0; i<values.count(); i++)
	{
		QDateTime dt;
		dt.setTime_t(values[i].toInt());
		hoursValues.push_back(QTime(0, 0, 0).secsTo(dt.time()));
	}
	
	// Sort the list
	//qSort(hoursValues);
	
	// Then we put some centroids (centroids in [0..24[)
	QList<int> centroids;
	int incr=qRound((double)hoursValues.count()/(double)avEventsPerDay);
	incr = incr ? incr : 1;
	for (int i=0; i<hoursValues.count(); i+=incr)
	{
		centroids.push_back(hoursValues[i]);
		kDebug(14315) << "statistics: add a centroid : " << centroids[centroids.count()-1];
	}
	
	
	// We need to compute the centroids
	centroids = computeCentroids(centroids, hoursValues);
	
	// Convert to QDateTime
	for (int i=0; i<centroids.count(); i++)
	{
		kDebug(14315) << "statistics: new centroid : " << centroids[i];

		QTime dt(0, 0, 0);
		dt = dt.addSecs(centroids[i]);
		mainEvents.push_back(dt);
	}
	
	
	return mainEvents;
}

QList<int> StatisticsContact::computeCentroids(const QList<int>& centroids, const QList<int>& values)
{
	kDebug(14315) << "statistics: enter compute centroids";

	QList<int> whichCentroid; // whichCentroid[i] = j <=> values[i] has centroid j for closest one
	QList<int> newCentroids;

    QList<int>::ConstIterator it = values.begin();
    QList<int>::ConstIterator end = values.end();
	for ( ; it != end; ++it )
	// Iterates over the values. For each one we need to get the closest centroid.
	{
		int value = *it;
		int distanceToNearestCentroid = abs(centroids[0]-value);
		int nearestCentroid = 0;
		for (int j=1; j<centroids.count(); j++)
		{
			if (abs(centroids[j]-value) < distanceToNearestCentroid)
			{
				distanceToNearestCentroid = abs(centroids[j]-value);
				nearestCentroid = j;
			}
		}
		whichCentroid.push_back(nearestCentroid);
	}
	
	// Recompute centroids
	newCentroids = centroids;
	
	for (int i=0; i<newCentroids.count(); i++)
	{
		kDebug(14315) << "statistics: compute new centroids"<< i;
		int weight = 0;
		for (int j=0; j<values.count(); j++)
		{
			int value = values[j];
			if (whichCentroid[j] == i)
			{
				newCentroids[i] = qRound((double)(value + newCentroids[i]*weight)/(double)(weight + 1));
				weight++;
				
			}
		}
	}
	
	
	
	// Should we recompute or are we OK ?
	int dist = 0;
	for (int i=0; i < newCentroids.count(); i++)
		dist += abs(newCentroids[i]-centroids[i]);
	
	if (dist > 10) 
		return computeCentroids(newCentroids, values);
	else
	{
		
		return newCentroids;
	}
}
