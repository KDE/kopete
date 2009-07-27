/*
    kopetecontacttaskbase.cpp - Base task for all contact tasks

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopetecontacttaskbase.h"

// Qt includes
#include <QtCore/QPointer>

// KDE includes
#include <kdebug.h>

// Kopete includes
#include <kopetecontact.h>
#include <kopeteprotocol.h>

namespace Kopete
{

class ContactTaskBase::Private
{
public:
	QPointer<Kopete::Contact> contact;
};

ContactTaskBase::ContactTaskBase(QObject *parent)
 : Kopete::Task(parent), d(new Private)
{
}

ContactTaskBase::~ContactTaskBase()
{
	delete d;
}

void ContactTaskBase::setContact(Kopete::Contact *contact)
{
	d->contact = contact;

	// Add the children tasks for DeleteContactTask from the rptocol
	KJob *subTask = d->contact->protocol()->createProtocolTask( taskType() );
	if( subTask )
	{
		kDebug(14010) << "Adding protocol subtask for " << taskType();
		addSubTask(subTask);
	}
}

Kopete::Contact *ContactTaskBase::contact()
{
	return d->contact;
}

} // end namespace Kopete

#include "kopetecontacttaskbase.moc"
