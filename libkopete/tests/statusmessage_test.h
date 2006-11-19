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
#ifndef KOPETESTATUSMESSAGE_TEST_H
#define KOPETESTATUSMESSAGE_TEST_H

#include <QObject>

class StatusMessage_Test : public QObject
{
	Q_OBJECT
private slots:
	void testNormalStatus();
	void testMusicMetaData();
	void testAppendHash();
};
#endif
