/***************************************************************************
                          jabbercontact.h  -  description
                             -------------------
    begin                : Thu Aug 15 2002
    copyright            : (C) 2002 by
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERRESOURCE_H
#define JABBERRESOURCE_H

/**
 * Container class for a contact's resource
 */

#include <qobject.h>
#include "jabberprotocol.h"

class QDateTime;

class JabberResource : public QObject
{
	Q_OBJECT

	public:
		JabberResource();
		JabberResource(const QString &, const int &, const QDateTime &, const JabberProtocol::Presence &, const QString &);
		~JabberResource();

        QString resource();
		
		int priority();

		QDateTime timestamp();
		
		JabberProtocol::Presence status();

		QString reason();

	private:
		QString mResource, mReason;
		int mPriority;
		JabberProtocol::Presence mStatus;
		QDateTime mTimestamp;
};

#endif
