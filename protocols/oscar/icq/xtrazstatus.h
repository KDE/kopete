/*
    xtrazstatus.h  -  Xtraz Status

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

#ifndef XTRAZSTATUS_H
#define XTRAZSTATUS_H

#include <QString>

namespace Xtraz
{

class Status
{
public:
	Status();
	~Status();

	void setStatus( int status );
	int status() const { return mStatus; }

	void setDescription( const QString& description );
	QString description() const { return mDescription; }

	void setMessage( const QString& message );
	QString message() const { return mMessage; }

private:
	int mStatus;
	QString mDescription;
	QString mMessage;

};

}

#endif
