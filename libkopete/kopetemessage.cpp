/*
	kopetemessage.cpp  -  Base class for Kopete messages

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

#include "kopetemessage.h"

#include "kopetemessage.moc"

KopeteMessage::KopeteMessage(QString from, QString to, QString body, MessageDirection direction) {
	mTimestamp = QDateTime::currentDateTime();
	mFrom = from;
	mTo = to;
	mBody = body;
	mDirection = direction;
}

KopeteMessage::KopeteMessage(QDateTime timestamp, QString from, QString to, QString body, MessageDirection direction) {
	mTimestamp = timestamp;
	mFrom = from;
	mTo = to;
	mBody = body;
	mDirection = direction;
}

// vim: set noet ts=4 sts=4 sw=4:

