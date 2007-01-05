/*
    slpmessage.cpp - Peer To Peer Session Layer Protocol Message class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include "slpmessage.h"
#include <qregexp.h>

namespace PeerToPeer
{

SlpMessage::SlpMessage(const QString& version) : Message(version)
{
}

SlpMessage::SlpMessage(const SlpMessage& other) : Message(other)
{
}

SlpMessage::~SlpMessage()
{
}

const QString SlpMessage::from() const
{
	QString s;
	QRegExp regex("<msnmsgr:([^>]*)>");
	if (regex.search(Message::from(), false) != -1)
	{
		s = regex.cap(1);
	}

	return s;
}

const QString SlpMessage::to() const
{
	QString s;
	QRegExp regex("<msnmsgr:([^>]*)>");
	if (regex.search(Message::to(), false) != -1)
	{
		s = regex.cap(1);
	}

	return s;
}

}
