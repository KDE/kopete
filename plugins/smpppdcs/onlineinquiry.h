/*
    onlineinquiry.h
 
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

#ifndef ONLINEINQUIRY_H
#define ONLINEINQUIRY_H

#include "iconnector.h"

class Detector;

/**
 * @author Heiko Sch&auml;fer <heiko@rangun.de>
 */

class OnlineInquiry : public IConnector {
    OnlineInquiry(const OnlineInquiry&);
    OnlineInquiry& operator=(const OnlineInquiry&);

public:
    OnlineInquiry();
    virtual ~OnlineInquiry();

    bool isOnline(bool useSMPPPD);

    virtual void setConnectedStatus(bool newStatus);

private:
    Detector * m_detector;
    bool       m_online;
};

#endif
