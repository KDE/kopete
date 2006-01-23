/*
    detectordcop.cpp
 
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

#include <kapplication.h>
#include <dcopclient.h>
#include <kdebug.h>

#include "kinternetiface_stub.h"

#include "detectordcop.h"
#include "iconnector.h"

QCString DetectorDCOP::m_kinternetApp = "";

DetectorDCOP::DetectorDCOP(IConnector * connector)
	: Detector(connector) {}

DetectorDCOP::~DetectorDCOP() {}

/*!
    \fn DetectorDCOP::getKInternetDCOP()
 */
QCString DetectorDCOP::getKInternetDCOP() const {
    DCOPClient * client = kapp->dcopClient();
    if(m_kinternetApp.isEmpty() && client && client->isAttached()) {
        // get all registered dcop apps and search for kinternet
        QCStringList apps = client->registeredApplications();
        QCStringList::iterator iter;
        for(iter = apps.begin(); iter != apps.end(); ++iter) {
            if((*iter).left(9) == "kinternet") {
                return *iter;
            }
        }
    }

    return m_kinternetApp;
}

/*!
    \fn DetectorDCOP::getConnectionStatusDCOP()
 */
DetectorDCOP::KInternetDCOPState DetectorDCOP::getConnectionStatusDCOP() const {
    kdDebug(14312) << k_funcinfo << "Start inquiring " << m_kinternetApp << " via DCOP" << endl;
	
	
	KInternetIface_stub stub = KInternetIface_stub(kapp->dcopClient(), m_kinternetApp, "KInternetIface");
	
	bool status = stub.isOnline();
	
	if(stub.ok()) {
		if(status) {
			kdDebug(14312) << k_funcinfo << "isOnline() returned true" << endl;
			return CONNECTED;
		} else {
			kdDebug(14312) << k_funcinfo << "isOnline() returned false" << endl;
			return DISCONNECTED;
		}
	} else {
		kdWarning(14312) << k_funcinfo << "DCOP call to " << m_kinternetApp << " failed!";
	}

	return ERROR;
}

