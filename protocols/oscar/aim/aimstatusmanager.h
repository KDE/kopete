/*
    aimstatusmanager.h  -  AIM Status Manager

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


#ifndef AIMSTATUSMANAGER_H
#define AIMSTATUSMANAGER_H

#include "oscarstatusmanager.h"

namespace Kopete { class OnlineStatus; }

class AIMStatusManager : public OscarStatusManager
{
public:
	AIMStatusManager();
	~AIMStatusManager();

	virtual Kopete::OnlineStatus connectingStatus() const;
	virtual Kopete::OnlineStatus unknownStatus() const;
	virtual Kopete::OnlineStatus waitingForAuth() const;

private:
	class Private;
	Private * const d;
};

#endif
