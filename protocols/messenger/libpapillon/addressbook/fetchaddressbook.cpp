/*
	fetchaddressbook.cpp: Job to fetch Address Book from Windows Live 
						  Messenger server using SOAP

    Copyright (c) 2007		by Zhang Panyong	        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers	<kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
namespace Papillon
{
class FetchAddressBookJob::Private
{
public:
	Private()
	 : connection(0)
	{}

	HttpConnection *connection;
	QPointer<AddressBook> addressBook;
};

FetchAddressBookJob::FetchAddressBookJob(AddressBook *addressBook)
 : QObject(contactList), d(new Private)
{
	d->addressBook = addressBook;
	d->connection = new HttpConnection( d->addressBook->client()->createSecureStream(), this );

	QString cookie = QString("MSPAuth=%1").arg( d->addressBook->client()->userContact()->loginCookie() );
	d->connection->setCookie(cookie);
}

FetchAddressBookJob::~FetchAddressBookJob()
{
	delete d;
}

void FetchAddressBookJob::execute()
{
	Papillon::Internal::ABServiceBinding *binding = new Papillon::Internal::ABServiceBinding(d->connection, this);
	connect(binding, SIGNAL(findABResult(Papillon::Internal::FindABResult *)), this, SLOT(bindingFindABResult(Papillon::Internal::FindABResult *)));

	QTimer::singleShot(0, binding, SLOT(findMembership()));

}

void FetchAddressBookJob::bindingFindAddressBookResult(Papillon::Internal::FindABResult *result)
{

	emit finished(this);
	deleteLater();
}

}
#include "fetchaddressbook.moc"
