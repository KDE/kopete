
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
	Action action(QString("action_name"));
	QVERIFY(action.listArgument().isEmpty());
	action.addArgument(QString("argument_name"),QString("argument_direction"),QString("argument_relatedStateVariable"));
	QVERIFY(action.listArgument().isEmpty() == false);
	QVERIFY(action.listArgument().size() == 1);
	Argument argument = action.listArgument().at(0);
	QCOMPARE(argument.name(),QString("argument_name"));
	QCOMPARE(argument.direction(),QString("argument_direction"));
	QCOMPARE(argument.relatedStateVariable(),QString("argument_relatedStateVariable"));
}

void ActionTest::testSetName()
{
	Action action;
	action.setName(QString("name_action"));
	QCOMPARE(action.name(),QString("name_action"));
}

QTEST_KDEMAIN_CORE(ActionTest)


