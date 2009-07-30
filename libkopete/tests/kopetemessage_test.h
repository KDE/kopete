/*
    Tests for Kopete::Message

    Copyright (c) 2005      by Duncan Mac-Vicar       <duncan@kde.org>
    Copyright (c) 2005      by Tommi Rantala  <tommi.rantala@cs.helsinki.fi>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEMESSAGE_TEST_H
#define KOPETEMESSAGE_TEST_H

#include <QObject>

#define private public
#include "kopetemessage.h"
#undef private

namespace Kopete
{
class Protocol;
class Account;
class MetaContact;
class Contact;
}

// change to SlotTester when it works
class KopeteMessage_Test : public QObject
{
	Q_OBJECT
public:
	KopeteMessage_Test();

private slots:
	void testPrimitives();
	void testLinkParser();

private:
	Kopete::Protocol *m_protocol;
	Kopete::Account *m_account;
	Kopete::MetaContact *m_metaContactMyself;
	Kopete::MetaContact *m_metaContactOther;
	Kopete::Contact *m_contactFrom;
	Kopete::Contact *m_contactTo;
};

#endif

