
#include "argumenttest.h"
#include "argument.h"
#include <QString>

ArgumentTest::ArgumentTest()
{
}

ArgumentTest::~ArgumentTest()
{
}

void ArgumentTest::testArgument() 
{
	Argument arg;
	QVERIFY(arg.name().isNull());
	QVERIFY(arg.direction().isNull());
	QVERIFY(arg.relatedStateVariable().isNull());
}

void ArgumentTest::testSetName()
{
	Argument arg;
	arg.setName(QString("name_arg"));
	QCOMPARE(arg.name(),QString("name_arg"));
}

void ArgumentTest::testSetDirection()
{
	Argument arg;
	arg.setDirection(QString("my_direction"));
	QCOMPARE(arg.direction(),QString("my_direction"));
}

void ArgumentTest::testSetRelatedStateVariable()
{
	Argument arg;
	arg.setRelatedStateVariable(QString("state_variable"));
	QCOMPARE(arg.relatedStateVariable(),QString("state_variable"));
}

QTEST_KDEMAIN_CORE(ArgumentTest)

