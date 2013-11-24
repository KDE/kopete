/*
    statisticscontact.h

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

#ifndef STATISTICSCONTACT_H
#define STATISTICSCONTACT_H

#include "kopeteonlinestatus.h"
#include "kopetemessage.h"
#include "kopetemetacontact.h"
#include <QtCore/QList>
#include <QtCore/QTime>
#include <QtCore/QPointer>

class StatisticsDB;
class QDateTime;

class StatisticsContact 
{
	
public:
	StatisticsContact(Kopete::MetaContact *mc, StatisticsDB *db);
	
	/**
	 * We save all stats to the database (commonstats)
	 */
	~StatisticsContact();

	
	/*
	 * Access method
	 */
	
	/** \brief Access method
	 *  \return m_db
	 */
	 StatisticsDB *db() { return m_db; }	

	/** \brief Access method
	 *  \return m_metaContact
	 */
	Kopete::MetaContact *metaContact() { return m_metaContact; }
	
	/** \brief Access method
	 *  \return m_metaContactId
	 */
	QString metaContactId() { return m_metaContactId; }
	
	/** \brief Access method
	 *  \return m_oldStatus
	 */
	Kopete::OnlineStatus::StatusType oldStatus() { return m_oldStatus; }
	
	/** \brief Access method
	 *  \return m_oldStatusDateTime
	 */
	QDateTime oldStatusDateTime() const { return m_oldStatusDateTime; }
	
	/** \brief Access method
	 *  \return m_messageLength
	 */
	int messageLength() const { return m_messageLength; }
	/** \brief Access method
	 *  \return m_timeBetweenTwoMessages
	 */
	int timeBetweenTwoMessages() const { return m_timeBetweenTwoMessages; }
	/**
	 * \brief Access method
	 * \return m_lastTalk
	 */
	QDateTime lastTalk() const { return m_lastTalk; }
	/**
	 * \brief Access method
	 * \return m_lastPresent
 	 */
	QDateTime lastPresent() const { return m_lastPresent; }
	/**
	 * \brief sets \p m_isChatWindowOpen to true
	 */
	void setIsChatWindowOpen(bool c) { m_isChatWindowOpen = c; }
	
	
	
	/*
	 * Method performing some useful actions
	 */
	/**
	 * \brief update the events database with the new statuss
	 */
	void onlineStatusChanged(Kopete::OnlineStatus::StatusType status);
	
	/**
	 * \brief update the average time between to messages for this contact
	 * Should be called when a new message is received by this contact
	 */
	void newMessageReceived(Kopete::Message& m);

	/**
	 * \returns true if contact was status at dt, false else.
	 */
	bool wasStatus(QDateTime dt, Kopete::OnlineStatus::StatusType status);
	
	/**
	 * \returns the status of the contact at dt. Return false if dt is invalid.
	 */
	QString statusAt(QDateTime dt);

	/**
	 * \returns the main (most used) status of the contact at date (not time) dt. return false if dt is invalid.
	 */
	QString mainStatusDate(const QDate& date);
	/*
	 * Prevision methods
	 */
	/**
// 	 * \brief Give information on when the next event will occur
// 	 *
// 	 * \param status the status to be checked.
// 	 * \retval nextEventDateTime the next event average prevision datetime.
// 	 */
// 	QDateTime nextEvent(const Kopete::OnlineStatus::StatusType& status);
// 	
// 	/**
// 	 * \brief Convenience method for nextEvent with Offline status
// 	 */
// 	QDateTime nextOfflineEvent();
// 	
// 	/**
// 	 * \brief Convenience method for nextEvent with Online status
// 	 */
// 	QDateTime nextOnlineEvent();
	
	
	/**
	 * \brief computes the main "status" events for the contact
	 */
	QList<QTime> mainEvents(const Kopete::OnlineStatus::StatusType& status);
	/// \brief used by mainEvents()
	QList<int> computeCentroids(const QList<int>& centroids, const QList<int>& values);

	
private:	
	/**
	 * \brief Checks if the value name exists in "commonstats" table, if not, add the row.
	 *
	 * \param name the name of the entry to be checked
	 * \param statVar1 retrieve this var from the database. If it doesn't exists, get it from \p defaultValue1
	 * \param statVar2 retrieve this var from the database. If it doesn't exists, get it from \p defaultValue2
	 * \param defaultValue1 defaultValue for \p statVar1
	 * \param defaultValue2 defaultValue for \p statVar2
	 * \retval statvar1 
	 * \retval statvar2
	 */
	void commonStatsCheck(const QString &name, QString& statVar1, QString& statVar2, const QString &defaultValue1 = "", const QString &defaultValue2 = "");
	
	/**
	 * @brief Same as commonStatsCheck for integers.
	 * Provided for convenience
	 */
	void commonStatsCheck(const QString &name, int& statVar1, int& statVar2, const int defaultValue1 = 0, const int defaultValue2 = -1);
	
	/**
	 * @brief Save a value in the "commonstats" table 
	 * \param name the name of the entry to be saved
	 * \param statVar1 what we are going to store in the first column for this entry
	 * \param statVar2 the second stat we can save in this table for this entry
	 * \param statVarChanged if this param is true, we save. Else, we don't. Spare some disk usage.
	 */
	void commonStatsSave(const QString &name, const QString &statVar1, const QString &statVar2, const bool statVarChanged);
	
	/**
	 * Kopete::MetaContact linked to this StatisticsContact
	 * Each StatisticsContact object _has_ to be linked to a metaContact
	 */ 
	QPointer <Kopete::MetaContact> m_metaContact;
	
	/**
	 * Id of Kopete::MetaContact
	 */
	QString m_metaContactId;
	
	/**
	 * Required to be able to write to the database
	 */
	StatisticsDB *m_db;
	
	/** 
	 * The interest of statistics contact is to manage the changes of status
	 * in order to correctly update the database. That's why here we keep the oldStatus
	 */
	Kopete::OnlineStatus::StatusType m_oldStatus;
	/// We keep the old status datetime here
	QDateTime m_oldStatusDateTime;
		
	/**
	 * Average time this user takes between two of his messages
	 * It may be used to compute a "speed" or "availability" for this contact
	 */
	int m_timeBetweenTwoMessages;
	bool m_timeBetweenTwoMessagesChanged;
	/// Date at which the last message was received. 
	/// Used to compute m_timeBetweenTwoMessages
	QDateTime m_lastMessageReceived;
	/// This is the ponderation corresponding to m_timeBetweenTwoMessagesOn
	int m_timeBetweenTwoMessagesOn;
	/// We don't count time if a chatwindow isn't open
	bool m_isChatWindowOpen;
	
	/**
	 * Average length of contact's messages
	 */
	int m_messageLength;
	bool m_messageLengthChanged;
	/// This is the ponderation corresponding to m_messageLength
	int m_messageLengthOn;
	
	/**
	 * Last time user talked with this contact
	 */
	QDateTime m_lastTalk;
	bool m_lastTalkChanged;
	
	/**
	 * Last time user was present (=online or away)
	 */
	QDateTime m_lastPresent;
	bool m_lastPresentChanged;
};


#endif
