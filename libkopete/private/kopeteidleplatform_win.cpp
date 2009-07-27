/*
    kopeteidleplatform_win.cpp  -  Kopete Idle Platform

    Copyright (C) 2003      by Justin Karneges       <justin@affinix.com>       (from KVIrc source code)
    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteidleplatform_p.h"

#define _WIN32_WINNT 0x0501
#include <windows.h>

Kopete::IdlePlatform::IdlePlatform()
 : d(0)
{
}

Kopete::IdlePlatform::~IdlePlatform()
{
    delete d;
}

bool Kopete::IdlePlatform::init()
{
    return true;
}

int Kopete::IdlePlatform::secondsIdle()
{
    LASTINPUTINFO li;
    li.cbSize = sizeof(LASTINPUTINFO);
    BOOL ok = GetLastInputInfo( &li );
    if ( !ok )
        return 0;

	return (GetTickCount() - li.dwTime) / 1000;
}
