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
#include <qdatetime.h>
#include <qstring.h>

class KopeteMessage : public QObject
{
	Q_OBJECT
public:
	/**
		Direction of a message. Inbound is from the chat partner, Outbound is
		from the user.
	*/
	enum MessageDirection { Inbound, Outbound };

	/*
		Constructs a new message
		Please note that body -must- be valid HTML, so all HTML control
		characters must be escaped.
	*/
	KopeteMessage(QString from, QString to, QString body, MessageDirection direction);
	KopeteMessage(QDateTime timestamp, QString from, QString to, QString body, MessageDirection direction);

	QDateTime timestamp() const { return mTimestamp; }

	QString from() const { return mFrom; }
	QString to() const { return mTo; }

	QString body() const { return mBody; }

	MessageDirection direction() const { return mDirection; }

protected:
	QDateTime mTimestamp;

	QString mFrom;
	QString mTo;
	QString mBody;

	MessageDirection mDirection;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

