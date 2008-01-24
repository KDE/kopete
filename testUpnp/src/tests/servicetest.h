/*
    servicetest.h -

    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef SERVICE_TEST_H
#define SERVICE_TEST_H

#include <QObject>
#include <qtest_kde.h>


class ServiceTest : public QObject 
{
	Q_OBJECT
	public:
		ServiceTest();
		~ServiceTest();
	private slots:
		// Test of constructor without argument
		void testService();
		// Setters tests
		void testSetServiceType();
		void testSetServiceId();
		void testSetControlURL();
		void testSetEventSubURL();
		void testSetXmlDocService();
		void testExistAction();
};

#endif



