/*
    onlineinquiry.cpp
 
    Copyright (c) 2005      by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "detector.h"
#include "onlineinquiry.h"

OnlineInquiry::OnlineInquiry() 
 : m_detector(NULL), m_online(FALSE) {
	m_detector = new Detector(this);
}

OnlineInquiry::~OnlineInquiry() {
	delete m_detector;
}

bool OnlineInquiry::isOnline(bool useSMPPPD) { 
	if(useSMPPPD) {
		m_detector->smpppdCheckStatus();
	} else {
		m_detector->netstatCheckStatus();
	}

	return m_online; 
}

void OnlineInquiry::setConnectedStatus(bool newStatus) { 
	m_online = newStatus; 
}
