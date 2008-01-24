/*
    servicetest.cpp -

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

#include "servicetest.h"
#include "service.h"
#include <QString>

ServiceTest::ServiceTest()
{
}

ServiceTest::~ServiceTest()
{
}

void ServiceTest::testService() 
{
	Service service;
	QVERIFY(service.serviceType().isNull());
	QVERIFY(service.serviceId().isNull());
	QVERIFY(service.controlURL().isNull());
	QVERIFY(service.eventSubURL().isNull());
	QVERIFY(service.xmlDocService().isNull());
	QVERIFY(service.actionList()->isEmpty());
}

void ServiceTest::testSetServiceType()
{
	Service service;
	service.setServiceType(QString("test_service_type"));
	QCOMPARE(service.serviceType(),QString("test_service_type"));
}

void ServiceTest::testSetServiceId()
{
	Service service;
	service.setServiceId(QString("test_service_id"));
	QCOMPARE(service.serviceId(),QString("test_service_id"));
}

void ServiceTest::testSetControlURL()
{
	Service service;
	service.setControlURL(QString("test_control_url"));
	QCOMPARE(service.controlURL(),QString("test_control_url"));
}

void ServiceTest::testSetEventSubURL()
{
	Service service;
	service.setEventSubURL(QString("test_event_suburl"));
	QCOMPARE(service.eventSubURL(),QString("test_event_suburl"));
}

void ServiceTest::testSetXmlDocService()
{
	Service service;
	service.setXmlDocService(QString("test_event_suburl"));
	QCOMPARE(service.xmlDocService(),QString("test_event_suburl"));
}

void ServiceTest::testExistAction()
{
	Service service;
	// Create actions
	Action action1, action2, action3, action4;
	// Create action1
	action1.setName(QString("SetConnectionType"));
	action1.addArgument(QString("NewConnectionType"),QString("in"),QString("ConnectionType"));
	// Create action2
	action2.setName(QString("GetConnectionTypeInfo"));
	action2.addArgument(QString("NewConnectionType"),QString("out"),QString("ConnectionType"));
	action2.addArgument(QString("NewPossibleConnectionTypes"),QString("out"),QString("PossibleConnectionType"));
	// Create action3
	action3.setName(QString("RequestConnection"));
	// Create action4
	action4.setName(QString("ForceTermination"));
	// Add action to the list
	service.actionList()->append(action1);
	service.actionList()->append(action2);
	service.actionList()->append(action3);
	service.actionList()->append(action4);
	QVERIFY(service.existAction(QString("SetConnectionType")));
	QVERIFY(service.existAction(QString("GetConnectionTypeInfo")));
	QVERIFY(service.existAction(QString("RequestConnection")));
	QVERIFY(service.existAction(QString("ForceTermination")));
	QVERIFY(service.existAction(QString("erreur1")) == false);
	QVERIFY(service.existAction(QString("erreur2")) == false);
}

QTEST_KDEMAIN_CORE(ServiceTest)
