//
// C++ Implementation: clientstream_test
//
// Description:
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
// Licensed under the GNU General Public License

#ifndef IPADDRTEST_H
#define IPADDRTEST_H

#include <qglobal.h>
#include <qapplication.h>
#include <qtimer.h>

#include "oscarutils.h"

#define QT_FATAL_ASSERT 1

class IPAddrTest : public QApplication
{
public:
	IPAddrTest(int argc, char ** argv);
	~IPAddrTest();

	bool testDottedDecimal();
	bool testNumericalIP();
	bool testAllZeroDotted();
	bool testAllZeroNumerical();

	void CheckTest(bool TestPassed);
};

#endif
