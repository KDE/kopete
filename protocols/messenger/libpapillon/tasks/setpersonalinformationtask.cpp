/*
   setpersonalinformationtask.h - Set personal information for myself contact.

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
#include "Papillon/Tasks/SetPersonalInformationTask"

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QLatin1String>
#include <QtCore/QUrl>
#include <QtDebug>

// Papillon includes
#include "Papillon/Transfer"
#include "Papillon/Connection"

namespace Papillon
{

class SetPersonalInformationTask::Private
{
public:
	Private()
	 : type(Papillon::ClientInfo::PersonalInfoNone)
	{}

	QString currentTransactionId;
	Papillon::ClientInfo::PersonalInformation type;
	QString value;

	/**
	 * @internal
	 * Return the string representation of the PersonalInformation enum.
	 * @param type the value of PersonalInformation.
	 */
	QString infoTypeToString(Papillon::ClientInfo::PersonalInformation type);
};

SetPersonalInformationTask::SetPersonalInformationTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{}

SetPersonalInformationTask::~SetPersonalInformationTask()
{
	delete d;	
}

bool SetPersonalInformationTask::take(Transfer *transfer)
{
	if( transfer->transactionId() == d->currentTransactionId )
	{
		setSuccess();
		return true;
	}

	return false;
}

void SetPersonalInformationTask::setPersonalInformation(Papillon::ClientInfo::PersonalInformation type, const QString &value)
{
	d->type = type;
	d->value = value;
}

void SetPersonalInformationTask::onGo()
{
	Q_ASSERT( d->type != Papillon::ClientInfo::PersonalInfoNone );

	d->currentTransactionId = QString::number( connection()->transactionId() );

	Transfer *setInfoTransfer = new Transfer(Transfer::TransactionTransfer);
	setInfoTransfer->setCommand( QLatin1String("PRP") );
	setInfoTransfer->setTransactionId( d->currentTransactionId );
	
	QStringList args;
	// Type of information that we are setting.
	args << d->infoTypeToString(d->type);
	// The new value for the information.
	args << QString( QUrl::toPercentEncoding(d->value) );

	setInfoTransfer->setArguments( args );

	qDebug() << PAPILLON_FUNCINFO << "Setting" << d->infoTypeToString(d->type) << "with value:" << d->value;
	send(setInfoTransfer);
}

QString SetPersonalInformationTask::Private::infoTypeToString(Papillon::ClientInfo::PersonalInformation type)
{
	QString result;

	switch(type)
	{
		case Papillon::ClientInfo::Nickname:
			result = QLatin1String("MFN");
			break;
		case Papillon::ClientInfo::PhoneHome:
			result = QLatin1String("PHH");
			break;
		case Papillon::ClientInfo::PhoneWork:
			result = QLatin1String("PHW");
			break;
		case Papillon::ClientInfo::PhoneMobile:
			result = QLatin1String("PHM");
			break;
		case Papillon::ClientInfo::MobileAuthorization:
			result = QLatin1String("MOB");
			break;
		case Papillon::ClientInfo::MobileDeviceEnabled:
			result = QLatin1String("MBE");
			break;
		default:
			break;
	}

	return result;
}

}

#include "setpersonalinformationtask.moc"
