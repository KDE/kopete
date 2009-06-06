/*
	Kopete Oscar Protocol
	icbmparamstask.h - Get the ICBM parameters
	
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
#ifndef ICBMPARAMSTASK_H
#define ICBMPARAMSTASK_H

#include <task.h>

/**
Get the parameters we need to follow for instant messages
 
@author Matt Rogers
*/
class ICBMParamsTask : public Task
{
public:
	ICBMParamsTask( Task* parent );
	~ICBMParamsTask();

	bool forMe( const Transfer* transfer ) const;
	bool take( Transfer* transfer );

	/**
	 * Handle the ICBM Parameters we get back from SNAC 0x04, 0x05
	 */
	void handleICBMParameters();
	
	/**
	 * Send the message parameters we want back to the server. Only
	 * appears to occur during login
	 * @param channel the channel to set up
	 */
	void sendMessageParams( int channel );

protected:
	void onGo();

};

#endif

//kate: auto-insert-doxygen on; tab-width 4; indent-mode csands;
