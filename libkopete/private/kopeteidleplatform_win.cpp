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

#include <QtCore/QLibrary>
#include <windows.h>

class Kopete::IdlePlatform::Private
{
public:
	Private()
	{
		GetLastInputInfo = 0;
		IdleUIGetLastInputTime = 0;
		lib = 0;
	}
	
	BOOL (__stdcall * GetLastInputInfo)(PLASTINPUTINFO);
	DWORD (__stdcall * IdleUIGetLastInputTime)(void);
	QLibrary *lib;
};

Kopete::IdlePlatform::IdlePlatform()
{
	d = new Private;
}

Kopete::IdlePlatform::~IdlePlatform()
{
	delete d->lib;
	delete d;
}

bool Kopete::IdlePlatform::init()
{
	if ( d->lib )
		return true;

	void *p;

	// try to find the built-in Windows 2000 function
	d->lib = new QLibrary( "user32" );
	if ( d->lib->load() && (p = d->lib->resolve( "GetLastInputInfo" )) )
	{
		d->GetLastInputInfo = (BOOL (__stdcall *)(PLASTINPUTINFO))p;
		return true;
	}
	else
	{
		delete d->lib;
		d->lib = 0;
	}

	// fall back on idleui
	d->lib = new QLibrary( "idleui" );
	if ( d->lib->load() && (p = d->lib->resolve("IdleUIGetLastInputTime")) )
	{
		d->IdleUIGetLastInputTime = (DWORD (__stdcall *)(void))p;
		return true;
	}
	else
	{
		delete d->lib;
		d->lib = 0;
	}

	return false;
}

int Kopete::IdlePlatform::secondsIdle()
{
	int i;
	if ( d->GetLastInputInfo )
	{
		LASTINPUTINFO li;
		li.cbSize = sizeof(LASTINPUTINFO);
		bool ok = d->GetLastInputInfo( &li );
		if ( !ok )
			return 0;

		i = li.dwTime;
	}
	else if ( d->IdleUIGetLastInputTime )
	{
		i = d->IdleUIGetLastInputTime();
	}
	else
		return 0;
	
	return (GetTickCount() - i) / 1000;
}
