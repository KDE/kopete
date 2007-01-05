/*
   fetchcontactlistjob.h - Job to fetch contact list from MSN server using SOAP

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
#ifndef FETCHCONTACTLISTJOB_H
#define FETCHCONTACTLISTJOB_H

#include <QtCore/QObject>

class QStringList;

namespace Papillon
{

class ContactList;

namespace Internal
{
	class FindMembershipResult;
}

class FetchContactListJob : public QObject
{
	Q_OBJECT
public:
	FetchContactListJob(ContactList *contactList);
	~FetchContactListJob();

public slots:
	void execute();

signals:
	void finished(Papillon::FetchContactListJob *job);

private slots:
	void bindingFindMembershipResult(Papillon::Internal::FindMembershipResult *result);

private:
	class Private;
	Private *d;
};

}

#endif
