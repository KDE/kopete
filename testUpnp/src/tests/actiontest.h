
#ifndef ACTION_TEST_H
#define ACTION_TEST_H

#include <QObject>
#include <qtest_kde.h>


class ActionTest : public QObject 
{
	Q_OBJECT
	public:
		ActionTest();
		~ActionTest();
	private slots:
		// Test of constructor without parameter
		void testAction();
		// Test of constructor with parameter
		void testAction_2();
		void testAddArgument();
		void testSetName();
};

#endif


