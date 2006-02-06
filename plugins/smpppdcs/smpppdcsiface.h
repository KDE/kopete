/*
    smpppdcsiface.h
 
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

#ifndef SMPPPDCSIFACE_H
#define SMPPPDCSIFACE_H

#include <dcopobject.h>

/**
 * @author Heiko Sch&auml;fer <heiko@rangun.de>
 */

class SMPPPDCSIFace : virtual public DCOPObject
{
  K_DCOP

  k_dcop:

    virtual QString detectionMethod() const = 0;
    virtual bool isOnline() const = 0;
};

#endif
