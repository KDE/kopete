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


class Buffer;

/**
 * @brief Base testcase class
 */
class OscarTestBase : public QObject
{
Q_OBJECT
public:
	OscarTestBase();

	/** Default destructor */
	~OscarTestBase();

	/**
	 * Set the directory to use to load test files
	 */
	void setPath( const QString& path );

	/**
	 * Takes a file @p file and attempts to load the file, prepending
	 * the path specified in the constructor as test data.
	 * @returns true on success.
	 */
	bool loadFile(const QString& file);
protected:
	Buffer* m_data;
	QString m_dataDir;
};

#define OSCAR_TEST_MAIN(TestObject) \
int main(int argc, char **argv) \
{ \
	QCoreApplication app(argc, argv); \
	TestObject tc; \
	if(argv[1]) \
		tc.setPath( argv[1] ); \
	else \
		tc.setPath( KDESRCDIR ); \
	return QTest::qExec( &tc, 0, 0 ); \
}

#endif
