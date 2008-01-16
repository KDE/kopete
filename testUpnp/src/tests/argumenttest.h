
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


