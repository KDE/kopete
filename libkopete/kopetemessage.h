/*
	kopetemessage.h  -  Base class for Kopete messages

	copyright   : (c) 2002 by Martijn Klingens
	email       : klingens@kde.org

	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; either version 2 of the License, or     *
	* (at your option) any later version.                                   *
	*                                                                       *
	*************************************************************************
*/

#ifndef _KOPETE_MESSAGE_H
#define _KOPETE_MESSAGE_H

#include <qobject.h>

class KopeteMessageLogger : public QObject
{
	Q_OBJECT
public:
	/**
		Direction of a message. Inbound is from the chat partner, Outbound is
		from the user.
	*/
	enum MessageDirection { Inbound, Outbound };
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

