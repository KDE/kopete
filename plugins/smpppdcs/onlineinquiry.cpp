/*
    onlineinquiry.cpp
 
    Copyright (c) 2005-2006 by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "detectornetstat.h"
#include "detectorsmpppd.h"
#include "onlineinquiry.h"

OnlineInquiry::OnlineInquiry()
        : m_detector(NULL), m_online(FALSE) {}

OnlineInquiry::~OnlineInquiry() {
    delete m_detector;
}

bool OnlineInquiry::isOnline(bool useSMPPPD) {
	
	delete m_detector;
	
    if(useSMPPPD) {
		m_detector = new DetectorSMPPPD(this);
    } else {
		m_detector = new DetectorNetstat(this);
    }
	
	m_detector->checkStatus();

    return m_online;
}

void OnlineInquiry::setConnectedStatus(bool newStatus) {
    m_online = newStatus;
}
