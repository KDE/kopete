/*
    kopeteidleplatform_dummy.cpp  -  Kopete Idle Platform

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

Kopete::IdlePlatform::IdlePlatform() : d(0)
{
}

Kopete::IdlePlatform::~IdlePlatform()
{

}

bool Kopete::IdlePlatform::init()
{
	return false;
}

int Kopete::IdlePlatform::secondsIdle()
{
	return 0;
}

