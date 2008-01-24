/*
    devicetest.cpp -

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

#include	"device.h"
#include	"devicetest.h"

DeviceTest::DeviceTest(){
}
DeviceTest::~DeviceTest(){
}
// Constructor test
void DeviceTest::testDevice() {
	// Without parameters
	Device device;
	QCOMPARE(device.deviceType(), QString(""));
}
/*
*	Test string
*/
#define		 DEVICE_TEST_STRING	"Some string"

/*
*	Setters test
*/
// Device type
void DeviceTest::testSetDeviceType(){
	Device device;
	device.setDeviceType(DEVICE_TEST_STRING);
	QCOMPARE(device.deviceType(), QString(DEVICE_TEST_STRING));
}

// Friendly name
void DeviceTest::testSetFriendlyName(){
	Device device;
	device.setFriendlyName(DEVICE_TEST_STRING);
	QCOMPARE(device.friendlyName(), QString(DEVICE_TEST_STRING));
}

// Manufacturer
void DeviceTest::testSetManufacturer(){
	Device device;
	device.setManufacturer(DEVICE_TEST_STRING);
	QCOMPARE(device.manufacturer(), QString(DEVICE_TEST_STRING));
}

// Manufacturer URL
void DeviceTest::testSetManufacturerURL(){
	Device device;
	device.setManufacturerURL(DEVICE_TEST_STRING);
	QCOMPARE(device.manufacturerURL(), QString(DEVICE_TEST_STRING));
}

// Model name
void DeviceTest::testSetModelName(){
	Device device;
	device.setModelName(DEVICE_TEST_STRING);
	QCOMPARE(device.modelName(), QString(DEVICE_TEST_STRING));
}

// UDN
void DeviceTest::testSetUDN(){
	Device device;
	device.setUDN(DEVICE_TEST_STRING);
	QCOMPARE(device.UDN(), QString(DEVICE_TEST_STRING));
}

// Model description
void DeviceTest::testSetModelDescription(){
	Device device;
	device.setModelDescription(DEVICE_TEST_STRING);
	QCOMPARE(device.modelDescription(), QString(DEVICE_TEST_STRING));
}

// Model number
void DeviceTest::testSetModelNumber(){
	Device device;
	device.setModelNumber(DEVICE_TEST_STRING);
	QCOMPARE(device.modelNumber(), QString(DEVICE_TEST_STRING));
}

// Serial number
void DeviceTest::testSetSerialNumber(){
	Device device;
	device.setSerialNumber(DEVICE_TEST_STRING);
	QCOMPARE(device.serialNumber(), QString(DEVICE_TEST_STRING));
}

// Presentation URL
void DeviceTest::testSetPresentationURL(){
	Device device;
	device.setPresentationURL(DEVICE_TEST_STRING);
	QCOMPARE(device.presentationURL(), QString(DEVICE_TEST_STRING));
}

// UPC
void DeviceTest::testSetUPC(){
	Device device;
	device.setUPC(DEVICE_TEST_STRING);
	QCOMPARE(device.UPC(), QString(DEVICE_TEST_STRING));
}

// Doc URL
void DeviceTest::testSetDescDocURL(){
	Device device;
	device.setDescDocURL(DEVICE_TEST_STRING);
	QCOMPARE(device.descDocURL(), QString(DEVICE_TEST_STRING));
}


/*
*/
QTEST_KDEMAIN_CORE(DeviceTest)
