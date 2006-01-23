/*
	detectornetworkstatus.cpp

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

#include <kdebug.h>

#include "kopeteuiglobal.h"
#include "connectionmanager.h"

#include "iconnector.h"
#include "detectornetworkstatus.h"

DetectorNetworkStatus::DetectorNetworkStatus(IConnector* connector) 
	: Detector(connector), m_connManager(NULL) {
	
	m_connManager = ConnectionManager::self();
	connect(m_connManager, SIGNAL(statusChanged(const QString&, NetworkStatus::EnumStatus)),
			this, SLOT(statusChanged(const QString&, NetworkStatus::EnumStatus)));
}

DetectorNetworkStatus::~DetectorNetworkStatus() {}

void DetectorNetworkStatus::checkStatus() const {
	// needs to do nothing
}

void DetectorNetworkStatus::statusChanged(const QString& host, NetworkStatus::EnumStatus status) {
	switch(status) {
		case NetworkStatus::Offline:
			kdDebug(14312) << k_funcinfo << host << ": NetworkStatus::Offline" << endl;
			break;
		case NetworkStatus::OfflineFailed:
			kdDebug(14312) << k_funcinfo << host << ": NetworkStatus::OfflineFailed" << endl;
			break;
		case NetworkStatus::OfflineDisconnected:
			kdDebug(14312) << k_funcinfo << host << ": NetworkStatus::OfflineDisconnected" << endl;
			break;
		case NetworkStatus::ShuttingDown:
			kdDebug(14312) << k_funcinfo << host << ": NetworkStatus::ShuttingDown" << endl;
			break;
		case NetworkStatus::Establishing:
			kdDebug(14312) << k_funcinfo << host << ": NetworkStatus::Establishing" << endl;
			break;
		case NetworkStatus::Online:
			kdDebug(14312) << k_funcinfo << host << ": NetworkStatus::Online" << endl;
			break;
		case NetworkStatus::NoNetworks:
			kdDebug(14312) << k_funcinfo << host << ": NetworkStatus::NoNetworks" << endl;
			break;
		case NetworkStatus::Unreachable:
			kdDebug(14312) << k_funcinfo << host << ": NetworkStatus::Unreachable" << endl;
			break;
	}
}

#include "detectornetworkstatus.moc"
