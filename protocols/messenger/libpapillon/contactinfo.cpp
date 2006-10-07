/*
   contactinfo.cpp - Information for a Windows Live Messenger contact.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "contactinfo.h"

// Qt includes
#include <QtCore/QSharedData>
#include <QtCore/QStringList>
#include <QtXml/QDomElement>

namespace Papillon
{

class ContactInfo::Private : public QSharedData
{
public:
	Private()
	 : clientFeatures(0), lists(0)
	{}

	QString contactId;
	QString passportId;
	
	QStringList groups;
	Papillon::ClientInfo::Features clientFeatures;
	Papillon::ContactList::ContactListFlags lists;
	
	QString displayName;
};

ContactInfo::ContactInfo()
{}

ContactInfo::~ContactInfo()
{
}

ContactInfo::ContactInfo(const ContactInfo &copy)
 : d(copy.d)
{
}

ContactInfo &ContactInfo::operator=(const ContactInfo &other)
{
	d = other.d;
	return *this;
}


bool ContactInfo::isValid() const
{
	return !d->passportId.isEmpty();
}

QString ContactInfo::contactId() const
{
	return d->contactId;
}

void ContactInfo::setContactId(const QString &contactId)
{
	d->contactId = contactId;
}

QString ContactInfo::passportId() const
{
	return d->passportId;
}

void ContactInfo::setPassportId(const QString &passportId)
{
	d->passportId = passportId;
}

ClientInfo::Features ContactInfo::clientFeatures() const
{
	return d->clientFeatures;
}

void ContactInfo::setClientFeatures(const ClientInfo::Features &features)
{
	d->clientFeatures = features;
}

ContactList::ContactListFlags ContactInfo::lists() const
{
	return d->lists;
}

void ContactInfo::setContactListFlags(const ContactList::ContactListFlags &flags)
{
	d->lists = flags;
}

void ContactInfo::addContactListFlags(const ContactList::ContactListFlags &flags)
{
	d->lists |= flags;
}

void ContactInfo::removeContactListFlags(const ContactList::ContactListFlags &flags)
{
	d->lists |= ~flags;
}

QStringList ContactInfo::groups() const
{
	return d->groups;
}

void ContactInfo::setGroups(const QStringList &groups)
{
	d->groups = groups;
}

QString ContactInfo::displayName() const
{
	return d->displayName;
}

void ContactInfo::setDisplayName(const QString &displayName)
{
	d->displayName = displayName;
}


QDomElement ContactInfo::toXml() const
{
	return QDomElement();	
}

void ContactInfo::fromXml(const QDomElement &xml)
{
	Q_UNUSED(xml)	
}


}
