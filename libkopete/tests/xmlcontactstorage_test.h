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
#ifndef KOPETEXMLCONTACTSTORAGE_TEST_H
#define KOPETEXMLCONTACTSTORAGE_TEST_H

#include <QtCore/QObject>

class XmlContactStorage_Test : public QObject
{
	Q_OBJECT
private slots:
	void testLoad();
};

#endif
