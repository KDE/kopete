/*
    detectorsmpppd.h
 
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

#ifndef DETECTORSMPPPD_H
#define DETECTORSMPPPD_H

#include <qstringlist.h>

#include "detectordcop.h"

namespace KNetwork {
class KStreamSocket;
};

class IConnector;

/**
	@author Heiko Sch&auml;fer <heiko@rangun.de>
*/
class DetectorSMPPPD : public DetectorDCOP {

    DetectorSMPPPD(const DetectorSMPPPD&);
    DetectorSMPPPD& operator=(const DetectorSMPPPD&);

public:
    DetectorSMPPPD(IConnector* connector);
    virtual ~DetectorSMPPPD();

    virtual void checkStatus() const;

};

#endif
