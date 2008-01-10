
#include "actiontest.h"
#include "action.h"
#include <QString>
#include <QList>

ActionTest::ActionTest()
{
}

ActionTest::~ActionTest()
{
}

void ActionTest::testAction() 
{
	Action action;
	QVERIFY(action.name().isNull());
	QVERIFY(action.listArgument().isEmpty());
}

void ActionTest::testAction_2()
{
	Action action(QString("action_name"));
	QCOMPARE(action.name(),QString("action_name"));
	QVERIFY(action.listArgument().isEmpty());
}

void ActionTest::testAddArgument()
{
}

void ActionTest::testSetName()
{
	Action action;
	action.setName(QString("name_action"));
	QCOMPARE(action.name(),QString("name_action"));
}

QTEST_KDEMAIN_CORE(ActionTest)


