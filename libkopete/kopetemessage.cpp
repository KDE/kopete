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

KopeteMessage::KopeteMessage()
{
	mTimestamp = QDateTime::currentDateTime();
	mFrom = "Unknown";
	mTo = "Unknown";
	mBody = "Body not set";
	mDirection = Outbound;
	mBg = QColor();
	mFg = QColor();
	mFont = QFont();
}


KopeteMessage::KopeteMessage(QString from, QString to, QString body, MessageDirection direction, QColor fg, QColor bg, QFont fnt)
{
	mTimestamp = QDateTime::currentDateTime();
	mFrom = from;
	mTo = to;
	mBody = body;
	mDirection = direction;
	mBg = bg;
	mFg = fg;
	mFont = fnt;
}

KopeteMessage::KopeteMessage(QDateTime timestamp, QString from, QString to, QString body, MessageDirection direction, QColor fg, QColor bg, QFont fnt)
{
	mTimestamp = timestamp;
	mFrom = from;
	mTo = to;
	mBody = body;
	mDirection = direction;
	mBg = bg;
	mFg = fg;
	mFont = fnt;
}

// vim: set noet ts=4 sts=4 sw=4:

