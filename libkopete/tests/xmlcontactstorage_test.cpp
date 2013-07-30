/*
    Unit test for Kopete::StatusMessage class.

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
#include "xmlcontactstorage_test.h"

// Qt includes
#include <QtCore/QStringList>

// KDE includes
#include <qtest_kde.h>

// Kopete includes
#include "kopetecontactliststorage.h"
#include "xmlcontactstorage.h"
#include "kopetegroup.h"
#include "kopetemetacontact.h"

QTEST_KDEMAIN( XmlContactStorage_Test, GUI )

QStringList expectedGroupList()
{
	QStringList groupList;
	
	groupList << QString("Kopete Developers");
	groupList << QString("Kontact Developers");
	groupList << QString("Users");
	groupList << QString("Friends & Family");
	groupList << QString("Top Level");

	return groupList;
}

QStringList expectedContactList()
{
	QStringList contactList;

	contactList << QString("Duncan Mac-Vicar Prett");
	contactList << QString("Olivier Goffart");
	contactList << QString("Matt Rogers");
	contactList << QString("Will Stephenson");
	// NOTE: We need ::fromUtf8 so those string will be really read as UTF8, otherwise test fail.
	contactList << QString::fromUtf8("Michaël Larouche");
	contactList << QString::fromUtf8("André Duffeck");

	return contactList;
}

void XmlContactStorage_Test::testLoad()
{
	// TODO: Check plugin data.
	// TODO: Check more things.
	QString xmlFilename = QString( SRCDIR ) + QString("xmlcontactstorage_test_list.xml");

	Kopete::ContactListStorage *storage = new Kopete::XmlContactStorage( xmlFilename );
	storage->load();

	// Verify that the loading went well.
	QVERIFY( storage->isValid() );
	// Check that we have the correct numbers of groups and contacts.
	QCOMPARE( storage->groups().size(), 5 );
	QCOMPARE( storage->contacts().size(), 6 );

	// Verify that we parsed the correct group names.
	QStringList groupNameList = expectedGroupList();
	QStringList::ConstIterator expectedIt, expectedItEnd = groupNameList.constEnd();

	uint groupId = 1;
	Kopete::Group::List loadGroupList = storage->groups();
	Kopete::Group::List::ConstIterator it, itEnd = loadGroupList.constEnd();
	for(it = loadGroupList.constBegin(), expectedIt = groupNameList.constBegin();
		it != itEnd, expectedIt != expectedItEnd;
		++it, ++expectedIt)
	{
		Kopete::Group *group = (*it);
		QString groupName = group->displayName();
		QString expectedGroupName = (*expectedIt);
		QCOMPARE( groupName, expectedGroupName );
		QCOMPARE( group->groupId(), groupId++ );
	}

	// Verify that we parsed the correct contacts.
	QStringList contactList = expectedContactList();
	QStringList::ConstIterator contactIt, contactItEnd = contactList.constEnd();
	
	Kopete::MetaContact::List loadContactList = storage->contacts();
	Kopete::MetaContact::List::ConstIterator loadContactIt, loadContactItEnd = loadContactList.constEnd();
	for(loadContactIt = loadContactList.constBegin(), contactIt = contactList.constBegin();
		loadContactIt != loadContactItEnd, contactIt != contactItEnd;
		++loadContactIt, ++contactIt)
	{
		Kopete::MetaContact *contact = (*loadContactIt);
		QString expectedContactName = (*contactIt);

		QCOMPARE( contact->displayName(), expectedContactName );
		QCOMPARE( (int)contact->displayNameSource(), (int)Kopete::MetaContact::SourceCustom );
	}
}

#include "xmlcontactstorage_test.moc"
