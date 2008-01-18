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
	/*
	*	Setters
	*/
	void testSetDeviceType();/*
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
	void testSetDescDocURL();*/
};

#endif
