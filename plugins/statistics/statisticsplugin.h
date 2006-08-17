/*
    statisticsplugin.h

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@gmail.com>


    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef STATISTICSPLUGIN_H
#define STATISTICSPLUGIN_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

#include <dcopobject.h>

#include "kopeteplugin.h"

#include "kopetemessage.h"
#include "kopetemessagehandler.h"
#include "kopeteonlinestatus.h"

#include "statisticsdcopiface.h"

class QString;

class StatisticsDB;
class StatisticsContact;
class StatisticsDCOPIface;

class KopeteView;
class KActionCollection;

/** \section Kopete Statistics Plugin
 *
 * \subsection intro_sec Introduction
 *
 * This plugin aims at giving detailed statistics on metacontacts, for instance, how long was
 * the metacontact online, how long was it busy etc. 
 * In the future, it will maybe make prediction on when the contact should be available for chat.
 *
 * \subsection install_sec How it works ...
 * Each Metacontact is bound to a StatisticsContact which has access to the SQLITE database.
 * This StatisticsContact stores the last status of the metacontact; the member function onlineStatusChanged is called when the 
 * metacontact status changed (this is managed in the slot slotOnlineStatusChanged of StatisticsPlugin) and then the DB is 
 * updated for the contact.
 * 
 * More exactly the DB is updated only if the oldstatus was not Offline

 * To have an idea how it works, here is a table :
 * 
 * <table>
  * <tr>
 * <td>Event</td><td>Changes to database</td><td>oldStatus</td>
 * </tr>
  * <tr>
 * <td>John 17:44 Away <i>(connexion)</i></td><td> - <i>(oldstatus was offline)</i></td><td>oldstatus = away </td>
 * </tr>
 * <tr>
 * <td>John 18:01 Online</td><td>(+) Away 17:44 18:01</td><td>oldstatus = online</td>
 * </tr>
 * <tr>
 * <td>John 18:30 Offline <i>(disconnect)</i></td><td>(+) Online 18:01 18:30</td><td>oldstatus = offline</td>
 * </tr>
 * <tr>
 * <td>John 18:45 Online <i>(connexion)</i></td><td> - <i>(oldstatus was offline)</i></td><td>oldstatus = online</td>
 * </tr>
 * <tr>
 * <td>John 20:30 Offline <i>(disconnect)</i></td><td>(+) Online 18:45 20:30</td><td>oldstatus = offline</td>
 * </tr>
 * </table>
 *  
 * etc.
 * 
 * \subsection install_sec Some little stats
 * This plugin is able to record some other stats, not based on events. Theyre saved in the commonstat table in which we store stats
 * like this :
 * 
 * <code>statname, statvalue1, statvalue2</code>
 * 
 * Generally, we store the value, and its ponderation. If an average on one hundred messages says that the contact X takes about 
 * 3 seconds between two messages, we store "timebetweentwomessages", "3", "100"
 *
 *
 *
 * StatisticsPlugin is the main Statistics plugin class.
 * Contains mainly slots.
 */
class StatisticsPlugin : public Kopete::Plugin, virtual public StatisticsDCOPIface
{
	Q_OBJECT
public:
	/// Standard plugin constructors
	StatisticsPlugin(QObject *parent, const char *name, const QStringList &args);
	~StatisticsPlugin();
	
	/// Method to access m_db member
	StatisticsDB *db() { return m_db; }
private slots:
	// Do the initializations
	void slotInitialize();

public slots:
	
	/** \brief This slot is called when the status of a contact changed.
	 *
	 *   Then it searches for the contact bind to the metacontact who triggered the signal and calls 
	 *   the specific StatisticsContact::onlineStatusChanged of the StatisticsContact to update the StatisticsContact status,
	 *   and maybe, update the database.
 	 */
	void slotOnlineStatusChanged(Kopete::MetaContact *contact, Kopete::OnlineStatus::StatusType status );

	/** 
	 * Builds and show the StatisticsDialog for a contact
	 */	   
	void slotViewStatistics();
		
	/**
	 *  
	 * Extract the metaContactId from the message, and calls the 
	 * StatisticsContact::newMessageReceived(Kopete::Message& m) function
	 * for the corresponding contact
	 */
	void slotAboutToReceive(Kopete::Message& m);
	
	/*
	 * Managing views
	 */
	
	/**
	 * \brief Only connects the Kopete::ChatSession::closing() signal to our slotViewClosed().
	 */
	void slotViewCreated(Kopete::ChatSession* session);
			  
	/**
	 * One aim of this slot is to be able to stop recording time between two messages 
	 * for the contact in the chatsession. But, we only 
	 * want to do this if the contact is not in an other chatsession.
	 */
	void slotViewClosed(Kopete::ChatSession* session);

	/**
	 * Slot called when a new metacontact is added to make some slots connections and to create a new
	 * StatisticsContact object.
	 *
	 * In the constructor, we connect the metacontacts already existing to some slots, but we need to do this
	 * when new metacontacts are added.
	 * This function is also called when we loop over the contact list in the constructor.
	*/
	void slotMetaContactAdded(Kopete::MetaContact *mc);
	
	/**
	 * Slot called when a metacontact is removed to delete statistic data from db and to remove StatisticsContact object.
	*/
	void slotMetaContactRemoved(Kopete::MetaContact *mc);
	
	/**
	 * Slot called when a contact is added to metacontact.
	 */
	void slotContactAdded(Kopete::Contact *c);
	
	/**
	 * Slot called when a contact is removed from metacontact.
	 */
	void slotContactRemoved(Kopete::Contact *c);


	/*
	 * DCOP functions 
	 * See statisticsdcopiface.h for the documentation
	 */
	void dcopStatisticsDialog(QString id);
	
	bool dcopWasOnline(QString id, int timeStamp);
	bool dcopWasOnline(QString id, QString dt);
	
	bool dcopWasAway(QString id, int timeStamp);
	bool dcopWasAway(QString id, QString dt);
	
	bool dcopWasOffline(QString id, int timeStamp);
	bool dcopWasOffline(QString id, QString dt);
	
	bool dcopWasStatus(QString id, QDateTime dateTime, Kopete::OnlineStatus::StatusType status);
	
	QString dcopStatus(QString id, QString dateTime);
	QString dcopStatus(QString id, int timeStamp);
	
	QString dcopMainStatus(QString id, int timeStamp);

private:	
	StatisticsDB *m_db;
	/** Associate a Kopete::Contact id to a StatisticsContact to retrieve
	* the StatisticsContact corresponding to the Kopete::Contact
	*/
	QMap<QString, StatisticsContact*> statisticsContactMap;
	/** Associate a Kopete::MetaContact to a StatisticsContact to retrieve
	* the StatisticsContact corresponding to the MetaContact
	*/
	QMap<Kopete::MetaContact*, StatisticsContact*> statisticsMetaContactMap;
};


#endif
