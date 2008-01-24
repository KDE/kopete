/*
    argumenttest.h -

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

#ifndef ARGUMENT_TEST_H
#define ARGUMENT_TEST_H

#include <QObject>
#include <qtest_kde.h>


class ArgumentTest : public QObject 
{
	Q_OBJECT
	public:
		ArgumentTest();
		~ArgumentTest();
	private slots:
		void testArgument();
		void testOperatorEquals();
		void testSetName();
		void testSetDirection();
		void testSetRelatedStateVariable();
};

#endif


