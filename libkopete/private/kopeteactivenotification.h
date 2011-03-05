/*
    kopeteactivenotification.h - View Manager

    Kopete    (c) 2009 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEACTIVENOTIFICATION_H
#define KOPETEACTIVENOTIFICATION_H

#include <QHash>

class KNotification;

namespace Kopete 
{

class ActiveNotification;

typedef QHash<QString, ActiveNotification *> ActiveNotifications;

class ActiveNotification : public QObject 
{
    Q_OBJECT

  public:
    /**
     * Construct an active notification and add to the specified hash.
     * notification becomes the parent object
     */
    ActiveNotification( KNotification *notification, const QString& id_, ActiveNotifications& notifications_, const QString& title_, const QString& body_ );

    /**
     * Remove active notification from queue
     */
    ~ActiveNotification();

    /**
     * Show this notification
     */
    void showNotification();

    /**
     * received another message from a sender with a notification
     */
    void incrementMessages();

  private:
    QString              id;
    ActiveNotifications& notifications;
    int                  nEventsSinceNotified;

    /**
     * This is text of title (like "Incoming message from ...")
     */
    QString              title;

    /**
     * This is the text of the body minus the <qt> and </qt> around it
     * and without the "n more messages" part
     */
    QString              body;
};

}

#endif
