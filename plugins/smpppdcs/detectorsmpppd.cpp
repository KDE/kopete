/*
    detectorsmpppd.cpp
 
    Copyright (c) 2004-2006 by Heiko Schaefer        <heiko@rangun.de>
 
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
#include <kglobal.h>
#include <kconfig.h>

#include "iconnector.h"
#include "detectorsmpppd.h"
#include "smpppdcsconfig.h"

#include "smpppdclient.h"

DetectorSMPPPD::DetectorSMPPPD(IConnector * connector)
        : DetectorDCOP(connector) {}

DetectorSMPPPD::~DetectorSMPPPD() {}

/*!
    \fn DetectorSMPPPD::checkStatus()
 */
void DetectorSMPPPD::checkStatus() const {
    kDebug(14312) << "Checking for online status...";

#ifndef NOKINTERNETDCOP
    m_kinternetApp = getKInternetDCOP();
    if(kapp->dcopClient() && !m_kinternetApp.isEmpty()) {
        switch(getConnectionStatusDCOP()) {
        case CONNECTED:
            m_connector->setConnectedStatus(true);
            return;
        case DISCONNECTED:
            m_connector->setConnectedStatus(false);
            return;
        default:
            break;
        }
    }
#else
#ifdef __GNUC__
#warning DCOP inquiry disabled
#endif
	kDebug(14312) << "DCOP inquiry disabled";
#endif

    SMPPPD::Client c;

	unsigned int port = SMPPPDCSConfig::self()->port();
	QString    server = SMPPPDCSConfig::self()->server();

	c.setPassword(SMPPPDCSConfig::self()->password().utf8());

    if(c.connect(server, port)) {
        m_connector->setConnectedStatus(c.isOnline());
    } else {
        kDebug(14312) << "not connected to smpppd => I'll try again later";
        m_connector->setConnectedStatus(false);
    }
}
