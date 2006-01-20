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

QCString DetectorDCOP::m_kinternetApp = "";

DetectorDCOP::DetectorDCOP(IConnector * connector)
        : Detector(connector) {}

DetectorDCOP::~DetectorDCOP() {}

/*!
    \fn DetectorDCOP::getKInternetDCOP()
 */
QCString DetectorDCOP::getKInternetDCOP() {
    m_client = kapp->dcopClient();
    if(m_kinternetApp.isEmpty() && m_client && m_client->isAttached()) {
        // get all registered dcop apps and search for kinternet
        QCStringList apps = m_client->registeredApplications();
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
DetectorDCOP::KInternetDCOPState DetectorDCOP::getConnectionStatusDCOP() {
    QByteArray data, replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly);

    kdDebug(14312) << k_funcinfo << "Start inquiring " << m_kinternetApp << " via DCOP" << endl;

    if(!m_client->call(m_kinternetApp, "KInternetIface", "isOnline()", data, replyType, replyData)) {
        kdDebug(14312) << k_funcinfo << "there was some error using DCOP." << endl;
    } else {
        QDataStream reply(replyData, IO_ReadOnly);
        if(replyType == "bool") {
            bool result;
            reply >> result;
            kdDebug(14312) << k_funcinfo << "isOnline() returned " << result << endl;
            return result ? CONNECTED : DISCONNECTED;
        } else {
            kdDebug(14312) << k_funcinfo << "isOnline() returned an unexpected type of reply!" << endl;
        }
    }

    return ERROR;
}

