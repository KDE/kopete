/*
    switchboard.cpp - Messenger Switchboard handle class

    Copyright (c) 2007		by Zhang Panyong        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

namespace Papillon
{

class SwitchBoard::Private
{
public:
}

SwitchBoard::SwitchBoard()
{

}

SwitchBoard::~SwitchBoard()
{

}

SwitchBoard::connect()
{
	if(!d->switchboardConnection){
		d->switchboardConnection = createConnection();
		connect(d->switchboardConnection, SIGNAL(connected()), this, SLOT(switchboardConnected()));
	}
}

}

