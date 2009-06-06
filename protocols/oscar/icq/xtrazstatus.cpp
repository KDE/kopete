/*
    xtrazstatus.cpp  -  Xtraz Status

    Copyright (c) 2007 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "xtrazstatus.h"

namespace Xtraz {

Status::Status()
{
	mStatus = 0;
}


Status::~Status()
{
}

void Status::setStatus( int status )
{
	mStatus = status;
}

void Status::setDescription( const QString& description )
{
	mDescription = description;
}

void Status::setMessage( const QString& message )
{
	mMessage = message;
}

}
