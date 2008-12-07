/*
    nscainfoevent.h  -  Non Server Contacts Add Info Event

    Copyright (c) 2008 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef NSCAINFOEVENT_H
#define NSCAINFOEVENT_H

#include <kopeteinfoevent.h>
#include <QSet>

class ContactManager;
class OContact;

class NonServerContactsAddInfoEvent : public Kopete::InfoEvent
{
	Q_OBJECT
public:
	NonServerContactsAddInfoEvent( ContactManager* listManager, bool icq, QObject* parent );

	void updateText();

	void addContact( const QString& contact );

public Q_SLOTS:
	void ssiContactAdded( const OContact& item );

private:
	QSet<QString> mRemainingContacts;
	int mContactCount;
};

#endif
