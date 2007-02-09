/*
    Kopete Oscar Protocol
    icqlogintask.h - Handles logging into to the ICQ service

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

#ifndef _OSCAR_ICQLOGINTASK_H_
#define _OSCAR_ICQLOGINTASK_H_

#include <oscartypes.h>
#include <task.h>

class QString;
class Transfer;

using namespace Oscar;

class IcqLoginTask : public Task
{
Q_OBJECT
public:
	IcqLoginTask( Task* parent );
	~IcqLoginTask();
	bool take( Transfer* transfer );
	virtual void onGo();

protected:
	bool forMe( Transfer* transfer ) const;

private:
	QString encodePassword( const QString& pw );

};

#endif
