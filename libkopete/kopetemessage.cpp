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

KopeteMessage::KopeteMessage()
{
	mTimestamp = QDateTime::currentDateTime();
	mBody = "Body not set";
	mDirection = Outbound;
	mBg = QColor();
	mFg = QColor();
	mFont = QFont();
}


KopeteMessage::KopeteMessage(const KopeteContact *fromKC,
		KopeteContactPtrList toKC, QString body, MessageDirection direction) {
	init(QDateTime::currentDateTime(), fromKC, toKC, body, QString::null, direction);
}

KopeteMessage::KopeteMessage(const KopeteContact *fromKC,
		KopeteContactPtrList toKC, QString body, QString subject, MessageDirection direction) {
	init(QDateTime::currentDateTime(), fromKC, toKC, body, subject, direction);
}

KopeteMessage::KopeteMessage(QDateTime timeStamp,
		const KopeteContact *fromKC, KopeteContactPtrList toKC, QString body,
		MessageDirection direction) {
	init(timeStamp, fromKC, toKC, body, QString::null, direction);
}

KopeteMessage::KopeteMessage(QDateTime timeStamp,
		const KopeteContact *fromKC, KopeteContactPtrList toKC, QString body,
		QString subject, MessageDirection direction) {
	init(timeStamp, fromKC, toKC, body, subject, direction);
}

void KopeteMessage::setFg(QColor color) {
	mFg = color;
}

void KopeteMessage::setBg(QColor color) {
	mBg = color;
}

void KopeteMessage::setFont(QFont font) {
	mFont = font;
}

void KopeteMessage::init(QDateTime timeStamp, const KopeteContact * from,
		KopeteContactPtrList to, QString body, QString subject, MessageDirection direction) {
	mTimestamp = timeStamp;
	mFrom = from;
	mTo = to;
	mBody = body;
	mSubject = subject;
	mDirection = direction;
	mFg = QColor();
	mBg = QColor();
	mFont = QFont();

}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */

// vim: set noet ts=4 sts=4 sw=4:

