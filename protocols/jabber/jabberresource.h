 /*
  * jabberresource.h
  * 
  * Copyright (c) 2002 by Daniel Stone <dstone@kde.org>
  * 
  * Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>
  * 
  * *************************************************************************
  * *                                                                       *
  * * This program is free software; you can redistribute it and/or modify  *
  * * it under the terms of the GNU General Public License as published by  *
  * * the Free Software Foundation; either version 2 of the License, or     *
  * * (at your option) any later version.                                   *
  * *                                                                       *
  * *************************************************************************
  */

#ifndef JABBERRESOURCE_H
#define JABBERRESOURCE_H

/**
 * Container class for a contact's resource
 */

#include <qobject.h>
#include "jabberprotocol.h"

class QDateTime;

class JabberResource:public QObject {
  Q_OBJECT public:
    JabberResource();
    JabberResource(const QString &, const int &, const QDateTime &,
		   const KopeteOnlineStatus & status, const QString &);

    QString resource();

    int priority();

    QDateTime timestamp();

    KopeteOnlineStatus status();

    QString reason();

  private:
    QString mResource, mReason;
    int mPriority;
    KopeteOnlineStatus mStatus;
    QDateTime mTimestamp;
};

#endif

// vim: set noet ts=4 sts=4 tw=4:
