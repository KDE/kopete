
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



