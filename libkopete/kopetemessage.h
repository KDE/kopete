/*
	kopetemessage.h  -  Base class for Kopete messages

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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
#include "qptrlist.h"

typedef QPtrList<KopeteContact> KopeteContactPtrList;

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 */

class KopeteMessage
{
public:
	/**
		Direction of a message. Inbound is from the chat partner, Outbound is
		from the user.
	*/
	enum MessageDirection { Inbound, Outbound, Internal};

	/**
		Format of the body
		-PlainText: Just a simple text
		-RichText: Text already HTML escaped and which can contains some tags
		-ParsedHTML: only used by the chatwindow, this text is parsed and ready to
			show into the chatwindow
	*/
	enum MessageFormat  { PlainText, RichText, ParsedHTML };

	/*
		Constructs a new message
		Please note that body -must- be valid HTML, so all HTML control
		characters must be escaped.
	*/
	KopeteMessage();
	KopeteMessage(const KopeteContact *, KopeteContactPtrList, QString, MessageDirection, MessageFormat f=PlainText );
	KopeteMessage(const KopeteContact*, KopeteContactPtrList, QString, QString, MessageDirection, MessageFormat f=PlainText );

	KopeteMessage(QDateTime, const KopeteContact *, KopeteContactPtrList, QString, MessageDirection, MessageFormat f=PlainText);
	KopeteMessage(QDateTime, const KopeteContact *, KopeteContactPtrList, QString, QString, MessageDirection, MessageFormat f=PlainText);

	// Accessors
	QDateTime timestamp() const { return mTimestamp; }

	const KopeteContact *from() const { return mFrom; }
	KopeteContactPtrList to() const { return mTo; }
	QColor fg() const { return mFg; }
	QColor bg() const { return mBg; }
	QFont font() const { return mFont; }
	QString body() const { return mBody; }
	QString subject() const { return mSubject; }
	MessageFormat format() const { return mFormat; }

	MessageDirection direction() const { return mDirection; }

	// Mutators
	void setFg(QColor color);
	void setBg(QColor color);
	void setFont(QFont font);
	void setBody( const QString& body , MessageFormat f=PlainText );

	/*
		Access to body whith specifiedFormat
	*/
	//plain text
	QString plainBody() const ;
	//HTML escaped
	QString escapedBody() const ;
	//Parsed (HTML and Emoticon, ready to use in the chatwindow)
	QString parsedBody() const ;
	
protected:
	// Helper for constructors
	void init(QDateTime timeStamp, const KopeteContact * from, KopeteContactPtrList to,
			  QString body, QString subject, MessageDirection direction, MessageFormat f);

	QDateTime mTimestamp;
	const KopeteContact *mFrom;
	QPtrList<KopeteContact> mTo;
	QString mBody, mSubject;
	QFont mFont;
	QColor mFg, mBg;

	MessageDirection mDirection;
	MessageFormat mFormat;

public:
	/**
	 * Use it to parse HTML in text.
	 * You dont need to use this for chat windows,
	 * There is a special class that abstract a chat view
	 * and uses HTML parser.
	 **/
	static QString parseHTML( QString message, bool parseURLs = true );


};

#endif





/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

