/*
	detectornetworkstatus.h

	Copyright (c) 2006      by Heiko Schaefer        <heiko@rangun.de>

	Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; version 2 of the License.               *
	*                                                                       *
	*************************************************************************
*/

#ifndef DETECTORNETWORKSTATUS_H
#define DETECTORNETWORKSTATUS_H

#include <qobject.h>

#include "detector.h"

class IConnector;
class ConnectionManager;

/**
	@author Heiko Sch&auml;fer <heiko@rangun.de>
*/
class DetectorNetworkStatus : protected QObject, public Detector
{
	Q_OBJECT
	
	DetectorNetworkStatus(const DetectorNetworkStatus&);
	DetectorNetworkStatus& operator=(const DetectorNetworkStatus&);

public:
    DetectorNetworkStatus(IConnector* connector);
    virtual ~DetectorNetworkStatus();
	
	virtual void checkStatus() const;

protected slots:
	void statusChanged(const QString& host, NetworkStatus::EnumStatus status);
	
private:
	ConnectionManager * m_connManager;
};

#endif
