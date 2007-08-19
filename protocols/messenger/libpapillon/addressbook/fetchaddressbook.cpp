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

	QString	abLastChange;
	QString	gleamsLastChange;

	HttpConnection *connection;
	QPointer<AddressBook> addressBook;
};

FetchAddressBookJob::FetchAddressBookJob(AddressBook *addressBook)
 : QObject(contactList), d(new Private)
{
	d->addressBook = addressBook;
	d->connection = new HttpConnection( d->addressBook->client()->createSecureStream(), this );
	d->abLastChange = QString("0001-01-01T00:00:00.0000000-08:00");
	d->gleamsLastChange= QString("0001-01-01T00:00:00.0000000-08:00");

	QString cookie = QString("MSPAuth=%1").arg( d->addressBook->client()->userContact()->loginCookie() );
	d->connection->setCookie(cookie);
}

FetchAddressBookJob::~FetchAddressBookJob()
{
	delete d;
}

FetchAddressBookJob::setABLastChange(QString &abLastChange)
{
	d->abLastChange = abLastChange;
}

FetchAddressBookJob::setGleamLastChange(QString &gleamsLastChange)
{
	d->gleamsLastChange = gleamsLastChange;
}

FetchAddressBookJob::setTicketToken(QString &TicketToken)
{
	d->TicketToken = TicketToken;
}

void FetchAddressBookJob::execute()
{
	Papillon::Internal::ABServiceBinding *binding = new Papillon::Internal::ABServiceBinding(d->connection, this);
	binding->setLastChange(d->abLastChange);
	binding->setGleamLastChange(d->gleamsLastChange);
	binding->setTicketToken(d->TicketToken);
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
