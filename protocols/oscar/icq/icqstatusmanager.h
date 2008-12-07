/*
    icqstatusmanager.h  -  ICQ Status Manager
    
    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2006,2007 by Roman Jarosz           <kedgedev@centrum.cz>
    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#ifndef ICQSTATUSMANAGER_H
#define ICQSTATUSMANAGER_H

#include "oscarstatusmanager.h"
#include "xtrazstatus.h"

namespace Kopete { class OnlineStatus; }

/**
 * @brief A manager for ICQ's online statuses
 * 
 * Looks after ICQ's numerous online statuses, and maps between them and Presence objects.
 * A single instance of this class is held by the ICQProtocol object.
 */
class ICQStatusManager : public OscarStatusManager
{
public:
	ICQStatusManager();
	~ICQStatusManager();

	virtual Kopete::OnlineStatus connectingStatus() const;
	virtual Kopete::OnlineStatus unknownStatus() const;
	virtual Kopete::OnlineStatus waitingForAuth() const;

	void loadXtrazStatuses();
	void saveXtrazStatuses();

	QList<Xtraz::Status> xtrazStatuses() const;
	void setXtrazStatuses( const QList<Xtraz::Status> &statusList );
	void appendXtrazStatus( const Xtraz::Status &status );

private:
	class Private;
	Private * const d;
};

#endif

