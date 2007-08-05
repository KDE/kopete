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
			kDebug(14312) << k_funcinfo << host << ": NetworkStatus::Offline";
			break;
		case NetworkStatus::OfflineFailed:
			kDebug(14312) << k_funcinfo << host << ": NetworkStatus::OfflineFailed";
			break;
		case NetworkStatus::OfflineDisconnected:
			kDebug(14312) << k_funcinfo << host << ": NetworkStatus::OfflineDisconnected";
			break;
		case NetworkStatus::ShuttingDown:
			kDebug(14312) << k_funcinfo << host << ": NetworkStatus::ShuttingDown";
			break;
		case NetworkStatus::Establishing:
			kDebug(14312) << k_funcinfo << host << ": NetworkStatus::Establishing";
			break;
		case NetworkStatus::Online:
			kDebug(14312) << k_funcinfo << host << ": NetworkStatus::Online";
			break;
		case NetworkStatus::NoNetworks:
			kDebug(14312) << k_funcinfo << host << ": NetworkStatus::NoNetworks";
			break;
		case NetworkStatus::Unreachable:
			kDebug(14312) << k_funcinfo << host << ": NetworkStatus::Unreachable";
			break;
	}
}

#include "detectornetworkstatus.moc"
