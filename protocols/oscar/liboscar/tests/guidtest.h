#ifndef GUIDTEST_H
#define GUIDTEST_H

#include <qobject.h>

class GuidTest : public QObject
{
Q_OBJECT
private slots:
	void testConstructors();
	void testSetData();
	void testIsVaild();
	void testCompare();
};

#endif
