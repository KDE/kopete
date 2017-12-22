/*
    nlamarok.h

    Kopete Now Listening To plugin


    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>

    Purpose:
    This class abstracts the interface to amaroK by
    implementing NLMediaPlayer

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef NLAMAROK_H
#define NLAMAROK_H

#include <QtDBus>
#include "nlmediaplayer.h"

class QDBusInterface;

class NLamaroK : public NLMediaPlayer
{
public:
    NLamaroK();
    virtual ~NLamaroK();
    virtual void update();
private:
    QDBusInterface *m_client;
};

#endif
