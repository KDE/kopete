/*
    devicetest.h -

    Copyright (c) 2007-2008 by Kevin Kin-foo     <>

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

#ifndef DEVICE_TEST_H
#define DEVICE_TEST_H

#include <QObject>
#include <qtest_kde.h>


class DeviceTest : public QObject 
{
	Q_OBJECT
	public:
		DeviceTest();
		~DeviceTest();
	private slots:
	// Contructor test
	void testDevice();
	/*
	*	Setters
	*/
	void testSetDeviceType();
	void testSetFriendlyName();
	void testSetManufacturer();
	void testSetManufacturerURL();
	void testSetModelName();
	void testSetUDN();
	void testSetModelDescription();
	void testSetModelNumber();
	void testSetSerialNumber();
	void testSetPresentationURL();
	void testSetUPC();
	void testSetDescDocURL();
};

#endif
