/*
    Kopete Oscar Protocol
    locationrightstask.h - Set up the service limitations

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LOCATIONRIGHTSTASK_H
#define LOCATIONRIGHTSTASK_H

#include <task.h>

class Transfer;

using namespace Oscar;

/**
This task handles location rights.
This task implements the following SNACS:
 \li 0x02, 0x02
 \li 0x02, 0x03
 
@author Kopete Developers
*/
class LocationRightsTask : public Task
{
public:
	LocationRightsTask( Task* parent );
	~LocationRightsTask();
	bool take( Transfer* transfer );

protected:
	bool forMe( const Transfer* transfer ) const;
	void onGo();

private:
	//! Send the location rights request ( SNAC 0x02, 0x02 )
	void sendLocationRightsRequest();

	//! Handle the location rights reply ( SNAC 0x02, 0x03 )
	void handleLocationRightsResponse();
};

#endif

//kate: tab-width 4; indent-mode csands;
