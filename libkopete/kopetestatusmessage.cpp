/*
    kopetestatusmessage.cpp - Describle a status message and it's metadata.

    Copyright (c) 2006  by Michaël Larouche          <larouche@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopetestatusmessage.h"

#include <QHash>

namespace Kopete
{

class StatusMessage::Private : public KShared
{
public:
	Private()
	{}

	QString statusTitle;
	QString statusMessage;
	QHash<QString, QVariant> metaData;
};	

StatusMessage::StatusMessage()
 : d(new Private)
{}

StatusMessage::StatusMessage(const QString &message)
 : d(new Private)
{
	d->statusMessage = message;
}

StatusMessage::StatusMessage(const QString &title, const QString &message)
	: d(new Private)
{
	d->statusTitle = title;
	d->statusMessage = message;
}

StatusMessage::~StatusMessage()
{}

StatusMessage::StatusMessage(const StatusMessage &copy)
 : d(copy.d)
{}

StatusMessage &StatusMessage::operator=(const StatusMessage &other)
{
	d = other.d;
	return *this;
}

bool StatusMessage::isEmpty() const
{
	return d->statusTitle.isEmpty() && d->statusMessage.isEmpty() && d->metaData.isEmpty();
}

void StatusMessage::setMessage(const QString &message)
{
	d->statusMessage = message;
}

QString StatusMessage::message() const
{
	return d->statusMessage;
}

void StatusMessage::addMetaData(const QString &key, const QVariant &value)
{
	d->metaData.insert(key, value);	
}

void StatusMessage::addMetaData(const QHash<QString, QVariant> &otherHash)
{
	QHash<QString, QVariant>::ConstIterator it, itEnd = otherHash.constEnd();
	for(it = otherHash.begin(); it != itEnd; ++it)
	{
		d->metaData.insert(it.key(), it.value());
	}
}

bool StatusMessage::hasMetaData(const QString &key) const
{
	return d->metaData.contains(key);
}

QVariant StatusMessage::metaData(const QString &key) const
{
	return d->metaData[key];
}

void StatusMessage::setTitle(const QString &title)
{
	d->statusTitle = title;
}

QString StatusMessage::title() const
{
	return d->statusTitle;
}

}
