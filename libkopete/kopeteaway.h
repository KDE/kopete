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

struct  KopeteAwayPrivate;

/* Forward Declarations */
class QStringList;
class KopeteGlobalAwayDialog;

/**
 * @class KopeteAway kopeteaway.h
 *
 * KopeteAway is a singleton class that manages away messages
 * for Kopete. It stores a global away message, as well as
 * a list of user defined away messages.
 * This class is used by KopeteAwayDialog, which gets it's
 * list of user-defined away messages from this.  Protocol
 * plugins' individual away dialogs should also get away
 * messages from this object.
 *
 * It also handle global Idle Time, and all auto away stuff
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
	 * This method sets the global away message,
	 * it does not set you away, just sets the message.
	 * @brief Sets the global away message
	 * @param message The message you want to set
	 */
	void setGlobalAwayMessage(const QString &message);

	/**
	 * @brief Sets global away for all protocols
	 */
	static void setGlobalAway(bool status);

	/**
	 * @brief Indicates global away status
	 * @return Bool indicating global away status
	 */
	static bool globalAway();

	/**
	 * @brief Function to get the titles of user defined away messages
	 * @return List of away message titles
	 *
	 * This function can be used to retrieve a QStringList of the away message titles,
	 * these titles can be passed to getMessage(QString title) to retrieve the
	 * corresponding message.
	 */
	QStringList getMessages();

	/**
	 * @brief Function to get an away message
	 * @return The away message corresponding to the title
	 * @param messageNumber Number of the away message to retrieve
	 *
	 * This function retrieves the away message that corresponds to the ringbuffer index
	 * passed in.
	 */
	 QString getMessage( uint messageNumber );

	 /**
	  * @brief Adds an away message to the ringbuffer
	  * @param message The away message
	  *
	  * This function will add an away message to the ringbuffer of user defined
	  * away messages.
	  */
	 void addMessage(const QString &message);

	/**
	 * time in seconds the user has been idle
	 */
	long int idleTime();

private:
	KopeteAway();
	~KopeteAway();

	/**
	 * @brief Saves the away messages to disk
	 *
	 * This function will save the current list of away messages to the disk
	 * using KConfig. It is called automatically.
	 */
	void save();

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
	 * this signal is emit when activity has been discover after being autoAway.
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

