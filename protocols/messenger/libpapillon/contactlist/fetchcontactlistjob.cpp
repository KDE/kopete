/*
   fetchcontactlistjob.cpp - Job to fetch contact list from MSN server using SOAP

   Copyright (c) 2007 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#include "fetchcontactlistjob.h"

// Qt includes
#include <QtCore/QPointer>
#include <QtCore/QTimer>
#include <QtCore/QLatin1String>
#include <QtDebug>

// Papillon includes
#include "Papillon/Contact"
#include "Papillon/ContactList"
#include "Papillon/Client"
#include "Papillon/UserContact"
#include "Papillon/Http/Connection"
#include "Papillon/Enums"

// Papillon private includes
#include "sharingservicebinding.h"

using namespace Papillon::Internal;

namespace Papillon
{

class FetchContactListJob::Private
{
public:
	Private()
	 : connection(0)
	{}

	QPointer<ContactList> contactList;
	HttpConnection *connection;
};

FetchContactListJob::FetchContactListJob(ContactList *contactList)
 : QObject(contactList), d(new Private)
{
	d->contactList = contactList;

	d->connection = new HttpConnection( d->contactList->client()->createSecureStream(), this );

	QString cookie = QString("MSPAuth=%1").arg( d->contactList->client()->userContact()->loginCookie() );
	d->connection->setCookie(cookie);
}

FetchContactListJob::~FetchContactListJob()
{
	delete d;
}

void FetchContactListJob::execute()
{
	Papillon::Internal::SharingServiceBinding *binding = new Papillon::Internal::SharingServiceBinding(d->connection, this);
	connect(binding, SIGNAL(findMembershipResult(Papillon::Internal::FindMembershipResult *)), this, SLOT(bindingFindMembershipResult(Papillon::Internal::FindMembershipResult *)));

	QTimer::singleShot(0, binding, SLOT(findMembership()));
}

void FetchContactListJob::bindingFindMembershipResult(Papillon::Internal::FindMembershipResult *result)
{
	if( result )
	{
		QList<Service*> services = result->services();
		foreach(Service *service, services)
		{
			foreach(Membership *membership, service->memberships())
			{
				ContactListEnums::ListFlags currentFlag;
	
				QString role = membership->memberRole();
				if( role == QLatin1String("Allow") )
					currentFlag = ContactListEnums::AllowList;
				else if( role == QLatin1String("Block") )
					currentFlag = ContactListEnums::BlockList;
				else if( role == QLatin1String("Reverse") )
					currentFlag = ContactListEnums::ReverseList;
				else if( role == QLatin1String("Pending") )
					currentFlag = ContactListEnums::PendingList;

				foreach(Member *member, membership->members())
				{
					QString contactId = member->passportName();
					Contact *contact = d->contactList->createContact(contactId);
					contact->setPassportId(contactId);
					contact->addToList(currentFlag);
				}
			}
		}

		delete result;
	}

	emit finished(this);
	deleteLater();
}

}

#include "fetchcontactlistjob.moc"
