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

#include "detectordcop.h"
#include "iconnector.h"
//Added by qt3to4:
#include <Q3CString>

Q3CString DetectorDCOP::m_kinternetApp = "";

DetectorDCOP::DetectorDCOP(IConnector * connector)
        : Detector(connector) {}

DetectorDCOP::~DetectorDCOP() {}

/*!
    \fn DetectorDCOP::getKInternetDCOP()
 */
Q3CString DetectorDCOP::getKInternetDCOP() {
    m_client = kapp->dcopClient();
    if(m_kinternetApp.isEmpty() && m_client && m_client->isAttached()) {
        // get all registered dcop apps and search for kinternet
        DCOPCStringList apps = m_client->registeredApplications();
        DCOPCStringList::iterator iter;
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
DetectorDCOP::KInternetDCOPState DetectorDCOP::getConnectionStatusDCOP() {
    QByteArray data, replyData;
    DCOPCString replyType;
    QDataStream arg(&data, QIODevice::WriteOnly);

    kDebug(14312) << k_funcinfo << "Start inquiring " << m_kinternetApp << " via DCOP" << endl;

    if(!m_client->call(m_kinternetApp, DCOPCString("KInternetIface"), DCOPCString("isOnline()"), data, replyType, replyData)) {
        kDebug(14312) << k_funcinfo << "there was some error using DCOP." << endl;
    } else {
        QDataStream reply(&replyData, QIODevice::ReadOnly);
        if(replyType == "bool") {
            bool result;
            reply >> result;
            kDebug(14312) << k_funcinfo << "isOnline() returned " << result << endl;
            return result ? CONNECTED : DISCONNECTED;
        } else {
            kDebug(14312) << k_funcinfo << "isOnline() returned an unexpected type of reply!" << endl;
        }
    }

    return ERROR;
}

