/*
    kopeteaway.h  -  Kopete Away

	Copyright (c) 2002 by Hendrik vom Lehn <hvl@linux-4-ever.de> 

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEAWAY_HI
#define KOPETEAWAY_HI

#include <qstring.h>
#include <kconfig.h>
#include <qvaluelist.h>

/**
 * @author Hendrik vom Lehn <hvl@linux-4-ever.de>
 * @author Chris TenHarmsel <ctenha56@calvin.edu>
 */

 struct KopeteAwayMessage{
	QString title;
	QString message;
};

/* Forward Declarations */
class QStringList;


/**
 * @class KopeteAway kopeteaway.h 
 * 
 * KopeteAway is a singleton class that manages away messages for Kopete.  It stores a global
 * away message, as well as a list of user defined away messages.  The messages are primarily
 * configured through the ConfigModule PersonalPrefsConfig, which is a member of the Kopete class.
 * This class is used by KopeteAwayDialog, which gets it's list of user-defined away messages from
 * this.  Protocol plugins' individual away dialogs should also get away messages from this object.
 *
 */
class KopeteAway
{
friend class KopeteAwayDialog;
	
	public:
	
	/** 
	 * @brief Method to get the single instance of KopeteAway
	 * @return KopeteAway instance pointer
	 */
	static KopeteAway *getInstance();
	
	/**
	 * @brief Gets the current global away message
	 * @return The global away message
	 */
	static QString message();
	
	/**
	 * @brief Shows the global away dialog
	 */
	static void show();
	
	/**
	 * @brief Sets global away (all protocols)
	 */
	static void setGlobalAway(bool status);
	
	/**
	 * @brief Indicates global away status
	 * @return Bool indicating global away status
	 */
	static bool globalAway();
	
	/**
	 * @brief Saves the away messages to disk
	 * 
	 * This function will save the current list of away messages to the disk
	 * using KConfig
	 */
	void save();
	
	/**
	 * @brief Function to get the titles of user defined away messages
	 * @return List of away message titles
	 *
	 * This function can be used to retrieve a QStringList of the away message titles,
	 * these titles can be passed to getMessage(QString title) to retrieve the 
	 * corresponding message.
	 */
	QStringList getTitles();	
	
	/**
	 * @brief Function to get an away message
	 * @return The away message corresponding to the title
	 * @param title Title of the away message to retrieve
	 *
	 * This function retrieves the away message that corresponds to the title passed in.
	 * If the requested away message is not found an empty string ("") is returned.
	 */
	 QString getMessage(QString title);
	 
	 /**
	  * @brief Adds an away message
	  * @param title The away message title
	  * @param message The away message
	  * @return true if away message was successfully added
	  * @return false if away message was not added (already exists)
	  *
	  * This function will add an away message with given title to the list of user defined
	  * away messages.  If the title conflicts with an already defined away message, nothing
	  * will be changed and the return value will be false.  Otherwise, the away message will
	  * be added and true will be returned.  save() should be called when finished adding messages
	  * in order to write the new message configuration to disk.
	  */
	 bool addMessage(QString title, QString message);
	 
	 /**
	  * @brief Deletes an away message
	  * @param title The title of the away message to delete
	  * @return true if function succeeds in deleting message
	  * @return false if function fails to delete message (indicated message does not exist)
	  * 
	  * This function will delete an away message with the given title from the list of user
	  * defined away messages.  save() should be called after any changes to the current away
	  * message list in order to write the new away message list to disk.
	  */
	 bool deleteMessage(QString title);
	 
	 /**
	 * @brief Updates an existing away message
	 * @param title Title of away message to be updated
	 * @param message New text of away message
	 * @return true if message was successfully updated
	 * @return false if message was not successfully update (message does not exist)
	 *
	 * This function will update an existing message and replace it's text with the
	 * text specified.
	 */
	bool updateMessage(QString title, QString message);
	
	private:
	KopeteAway();
	KopeteAway( const KopeteAway &rhs );
	KopeteAway &operator=( const KopeteAway &rhs );
	static KopeteAway *instance;
	QString mAwayMessage;
	bool mGlobalAway;
	QValueList<KopeteAwayMessage> mAwayMessageList;
	KConfig *config;	
};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

