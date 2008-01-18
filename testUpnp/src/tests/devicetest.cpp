#include	"device.h"
#include	"devicetest.h"

DeviceTest::DeviceTest(){
}
DeviceTest::~DeviceTest(){
}

/*
*	Setters test
*/
void DeviceTest::testSetDeviceType(){
	Device device;
	device.setDeviceType("Some type");
	QCOMPARE(device.deviceType(), QString("Some type"));
}
/*
void DeviceTest::testSetFriendlyName(){
}

void DeviceTest::testSetManufacturer(){
}

void DeviceTest::testSetManufacturerURL(){
}

void DeviceTest::testSetModelName(){
}

void DeviceTest::testSetUDN(){
}

void DeviceTest::testSetModelDescription(){
}

void DeviceTest::testSetModelNumber(){
}

void DeviceTest::testSetSerialNumber(){
}

void DeviceTest::testSetPresentationURL(){
}

void DeviceTest::testSetUPC(){
}

void DeviceTest::testSetDescDocURL(){
}
*/
QTEST_KDEMAIN_CORE(DeviceTest)
