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
#include <kapplication.h>

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
    kdDebug(14312) << k_funcinfo << "Checking for online status..." << endl;

#ifndef NOKINTERNETDCOP
    m_kinternetApp = getKInternetDCOP();
    if(kapp->dcopClient() && m_kinternetApp != "") {
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
#warning DCOP inquiry disabled
	kdDebug(14312) << k_funcinfo << "DCOP inquiry disabled" << endl;
#endif

    SMPPPD::Client c;

	unsigned int port = SMPPPDCSConfig::self()->port();
	QString    server = SMPPPDCSConfig::self()->server();

	c.setPassword(SMPPPDCSConfig::self()->password().utf8());

    if(c.connect(server, port)) {
        m_connector->setConnectedStatus(c.isOnline());
    } else {
        kdDebug(14312) << k_funcinfo << "not connected to smpppd => I'll try again later" << endl;
        m_connector->setConnectedStatus(false);
    }
}
