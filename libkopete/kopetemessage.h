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

#include <qdatetime.h>
#include <qstring.h>
#include <qfont.h>
#include <qcolor.h>
#include "kopetecontact.h"

typedef QPtrList<KopeteContact> KopeteContactList;

class KopeteMessage
{
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
	KopeteMessage();
	KopeteMessage(const KopeteContact *, KopeteContactList, QString, MessageDirection );
	KopeteMessage(const KopeteContact*, KopeteContactList, QString, QString, MessageDirection );

	KopeteMessage(QDateTime, const KopeteContact *, KopeteContactList, QString, MessageDirection);
	KopeteMessage(QDateTime, const KopeteContact *, KopeteContactList, QString, QString, MessageDirection);

	// Accessors
	QDateTime timestamp() const { return mTimestamp; }

	const KopeteContact *from() const { return mFrom; }
	KopeteContactList to() const { return mTo; }
	QColor fg() const { return mFg; }
	QColor bg() const { return mBg; }
	QFont font() const { return mFont; }
	QString body() const { return mBody; }
	QString subject() const { return mSubject; }

	MessageDirection direction() const { return mDirection; }

	// Mutators
	void setFg(QColor color);
	void setBg(QColor color);
	void setFont(QFont font);

protected:
	// Helper for constructors
	void init(QDateTime timeStamp, const KopeteContact * from, KopeteContactList to, 
			  QString body, QString subject, MessageDirection direction);
	
	QDateTime mTimestamp;
	const KopeteContact *mFrom;
	QPtrList<KopeteContact> mTo;
	QString mBody, mSubject;
	QFont mFont;
	QColor mFg, mBg;

	MessageDirection mDirection;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

