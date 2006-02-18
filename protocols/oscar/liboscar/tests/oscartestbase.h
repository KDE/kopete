/*
    oscartestbase.h - OSCAR Testlib base

    Copyright (c) 2006 by Brian Smith <linuxfood@linuxfood.net>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef OSCARTESTBASE_H
#define OSCARTESTBASE_H

#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include "buffer.h"

class OscarTestBase : public QObject
{
Q_OBJECT
public:
	OscarTestBase(QString &path, QObject *parent = 0);
	bool loadFile(QString &file);
protected:
	Buffer m_data;
	QString m_dataDir;
};

#define OSCAR_TEST_MAIN(TestCase) \
int main(int argc, char **argv) \
{ \
	if(argv[1]) { \
		TestCase tc(argv[1]); \
		return QTest::qExec( &tc, argc, argv); \
	} \
}

#endif
