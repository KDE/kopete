/*
  kopeteaway.cpp  -  Kopete Away

  Copyright (c) 2002      by Hendrik vom Lehn       <hvl@linux-4-ever.de>

  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "kopeteaway.h"

#include <qstring.h>
#include <kconfig.h>
#include <qmultilineedit.h>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

KopeteAway *KopeteAway::instance = 0L;

KopeteAway::KopeteAway()
{
    // Set up the away messages
    mAwayMessage = "";
    mGlobalAway = false;

    // Empty the list
    mAwayMessageList.clear();

    // Set up the config object
    config = KGlobal::config();

    /* Load the saved away messages */
    config->setGroup("Away Messages");
    /* If Kopete has been run before, this will be true.
     * It's only false the first time Kopete is run
     */
    if(config->hasKey("Titles")){
        QStringList titles = config->readListEntry("Titles");  // Get the titles
        KopeteAwayMessage temp; // Temporary away message....
        for(QStringList::iterator i = titles.begin(); i != titles.end(); i++){
            temp.title = (*i); // Grab the title from the list of messages
            temp.message = config->readEntry(temp.title); // And the message (from disk)
            mAwayMessageList.append(temp); // And add it to the list
        }
    } else {
        /* There are no away messages, so we'll create a default one */
        /* Create an away message */
        KopeteAwayMessage temp;
        temp.title = i18n("Busy");
        temp.message = i18n("Sorry, I'm busy right now");

        /* Add it to the vector */
        mAwayMessageList.append(temp);

        temp.title = i18n("Gone");
        temp.message = i18n("I'm gone right now, but I'll be back later");
        mAwayMessageList.append(temp);

        /* Save this list to disk */
        save();
    }
}

KopeteAway::~KopeteAway()
{
}

QString KopeteAway::message()
{
    return getInstance()->mAwayMessage;
}

void KopeteAway::setGlobalAwayMessage(QString message)
{
    if( !message.isEmpty() )
    {
        kdDebug( 14013 ) << "[KOPETE AWAY] Setting global away message: " << message << endl;
        mAwayMessage = message;
    }
}

KopeteAway *KopeteAway::getInstance()
{
    if (instance == 0L)
    {
        instance = new KopeteAway;
    }
    return instance;
}

bool KopeteAway::globalAway()
{
    return getInstance()->mGlobalAway;
}

void KopeteAway::setGlobalAway(bool status)
{
    getInstance()->mGlobalAway = status;
}

void KopeteAway::save(){
    /* Set the away message settings in the Away Messages config group */
    config->setGroup("Away Messages");
    QStringList titles;
    /* For each message, keep track of the title, and write out the message */
    for(QValueList<KopeteAwayMessage>::iterator i = mAwayMessageList.begin(); i != mAwayMessageList.end(); i++){
        titles.append((*i).title); // Append the title to list of titles
        config->writeEntry((*i).title, (*i).message); // Append Title->message pair to the config
    }

    /* Write out the titles */
    config->writeEntry("Titles", titles);
}

QStringList KopeteAway::getTitles()
{
    QStringList titles;
    for(QValueList<KopeteAwayMessage>::iterator i = mAwayMessageList.begin(); i != mAwayMessageList.end(); i++){
        titles.append((*i).title);
    }
    return titles;
}

QString KopeteAway::getMessage(QString title)
{
    for(QValueList<KopeteAwayMessage>::iterator i = mAwayMessageList.begin(); i != mAwayMessageList.end(); i++){
        if((*i).title == title)
            return (*i).message;
    }

    /* Return an empty string if none was found */
    return QString::null;
}

bool KopeteAway::addMessage(QString title, QString message)
{
    bool found = false;
    /* Check to see if it exists already */
    for(QValueList<KopeteAwayMessage>::iterator i = mAwayMessageList.begin(); i != mAwayMessageList.end(); i++){
        if((*i).title == title){
            found = true;
            break;
        }
    }

    /* If not, add it */
    if(!found){
        KopeteAwayMessage temp;
        temp.title = title;
        temp.message = message;
        mAwayMessageList.append(temp);
        return true;
    } else {
        return false;
    }
}

bool KopeteAway::deleteMessage(QString title)
{
    /* Search for the message */
    QValueList<KopeteAwayMessage>::iterator itemToDelete = mAwayMessageList.begin();
    while( (itemToDelete != mAwayMessageList.end()) && ((*itemToDelete).title != title) )
    {
        itemToDelete++;
    }

    /* If it was found, delete it */
    if(itemToDelete != mAwayMessageList.end())
    {
        /* Remove it from the config entry, if it's there */
        if(config->hasKey((*itemToDelete).title)){
            config->deleteEntry((*itemToDelete).title);
        }
        /* Remove it from the list */
        mAwayMessageList.remove(itemToDelete);

        return true;
    }
    else
    {
        return false;
    }
}

bool KopeteAway::updateMessage(QString title, QString message)
{
    /* Search for the message */
    QValueList<KopeteAwayMessage>::iterator itemToUpdate = mAwayMessageList.begin();
    while( (itemToUpdate != mAwayMessageList.end()) && ((*itemToUpdate).title != title) )
    {
        itemToUpdate++;
    }

    /* If it was found, update it */
    if(itemToUpdate != mAwayMessageList.end()){
        (*itemToUpdate).message = message;
        return true;
    }
    else
    {
        return false;
    }
}

// vim: set et ts=4 sts=4 sw=4:

