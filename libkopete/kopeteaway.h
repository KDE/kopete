/*
    kopeteaway.h  -  Kopete Away

    Copyright (c) 2002 by Hendrik vom Lehn <hvl@linux-4-ever.de>
    Copyright (c) 2003 Olivier Goffart     <ogoffart@tiscalinet.be>

    Kopete (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEAWAY_HI
#define KOPETEAWAY_HI

#include <qstring.h>
#include <qobject.h>
#include <qvaluelist.h>

struct KopeteAwayPrivate;

/* Forward Declarations */
class QStringList;
class KopeteGlobalAwayDialog;

/**
 * @class KopeteAway kopeteaway.h
 *
 * KopeteAway is a singleton class that manages away messages
 * for Kopete. It stores a global away message, as well as
 * a list of user defined away messages.
 * This class is used by KopeteAwayDialog, which gets its
 * list of user-defined away messages from here.  Protocol
 * plugins' individual away dialogs should also get away
 * messages from this object.
 *
 * It also handles global idle time and all the auto away stuff
 *
 * @author Hendrik vom Lehn <hvl@linux-4-ever.de>
 * @author Chris TenHarmsel <tenharmsel@users.sourceforge.net>
 * @author Olivier Goffart <ogoffart@tiscalinet.be>

 */
class KopeteAway : public QObject
{
Q_OBJECT

friend class KopeteAwayDialog;

public:

	/**
	 * @brief Gets the single instance of KopeteAway
	 * @return A KopeteAway instance pointer
	 */
	static KopeteAway *getInstance();

	/**
	 * @brief Gets the current global away message
	 * @return The global away message
	 */
	static QString message();

	/**
	 * @brief Sets the global away message.
	 *
	 * It does not set you away, just sets the message.
	 * @param message The message you want to set
	 */
	void setGlobalAwayMessage(const QString &message);

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
	 * @brief Get the titles of the user defined away messages
	 * @return List of away message titles
	 *
	 * This function can be used to retrieve a QStringList of the away message titles,
	 * these titles can be passed to getMessage(QString title) to retrieve the
	 * corresponding message.
	 */
	QStringList getTitles();

	/**
	 * @brief Get an away message
	 * @return The away message corresponding to the title
	 * @param title Title of the away message to retrieve
	 *
	 * This function retrieves the away message that corresponds to the title passed in.
	 * If the requested away message is not found an empty string ("") is returned.
	 */
	 QString getMessage(const QString &title);

	 /**
	  * @brief Adds an away message
	  * @param title The away message title
	  * @param message The away message
	  * @return true if the away message was successfully added
	  * @return false if the away message was not added
	  *
	  * This function will add an away message with the given title to the list of user defined
	  * away messages.  If the title conflicts with an already defined away message, nothing
	  * will be changed and the return value will be false.  Otherwise, the away message will
	  * be added and true will be returned.  save() should be called when finished adding messages
	  * in order to write the new message configuration to disk.
	  */
	 bool addMessage(const QString &title, const QString &message);

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
	 bool deleteMessage(const QString &title);

	 /**
	 * @brief Updates an existing away message
	 * @param title Title of away message to be updated
	 * @param message New text of away message
	 * @return true if message was successfully updated
	 * @return false if message was not successfully update (message does not exist)
	 *
	 * This function will update an existing message and replace its text with the
	 * text specified.
	 */
	bool updateMessage(const QString &title, const QString &message);

	/**
	 * time in seconds the user has been idle
	 */
	long int idleTime();

private:
	KopeteAway();
	~KopeteAway();
	//KopeteAway( const KopeteAway &rhs );
	//KopeteAway &operator=( const KopeteAway &rhs );
	static KopeteAway *instance;
	KopeteAwayPrivate *d;

private slots:
	void slotTimerTimeout();
	void load();

public slots:
	/**
	 * @brief Set the activity
	 *
	 * Plugins can set the activity if they discover activity by another way than the mouse or the keyboard
	 * (example, the motion auto away plugin)
	 * this will reset the @ref idleTime to 0, and set all protocols to available (online) if the state was
	 * set automatically to away because of idleness, and if they was previously online
	 */
	void setActivity();

	/**
	 * @brief Go in to auto away mode
	 *
	 * Use this method if you want to go in the autoaway mode.
	 * This will go autoaway even if the idle time is not yet reached. (and even if the user
	 * did not selected to go autoaway automaticaly)
	 * But that will go unaway again when activity will be detected
	 */
	void setAutoAway();

signals:
	/**
	 * @brief Activity was detected
	 *
	 * This signal is emitted when activity has been discovered after being set away automatically.
	 */
	void activity();

	/**
	 * @brief Default messages were changed
	 */
	void messagesChanged();
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

