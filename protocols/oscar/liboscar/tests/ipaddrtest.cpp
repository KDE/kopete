//Licensed under the GNU General Public License

#include <iostream>
#include "ipaddrtest.h"
#include <qstring.h>

using namespace std;
IPAddrTest::IPAddrTest(int argc, char ** argv)
: QApplication( argc, argv )
{
}

IPAddrTest::~IPAddrTest()
{
}

bool IPAddrTest::testDottedDecimal()
{
    DWORD address = 1096652712;
    return ( Oscar::getDottedDecimal( address ) == "65.93.151.168" );
}

bool IPAddrTest::testAllZeroDotted()
{
    DWORD address = 0;
    return ( Oscar::getDottedDecimal( address ) == "0.0.0.0" );
}

bool IPAddrTest::testNumericalIP()
{
    QString address = "65.93.151.168";
    return ( Oscar::getNumericalIP( address ) == 1096652712 );
}

bool IPAddrTest::testAllZeroNumerical()
{
    QString address = "0.0.0.0";
    return ( Oscar::getNumericalIP( address ) == 0 );
}

void IPAddrTest::CheckTest(bool TestPassed)
{
	if ( TestPassed )
		cout << "passed" << endl;
	else
		cout << "failed" << endl;
}

int main(int argc, char ** argv)
{
	IPAddrTest a( argc, argv );

	a.CheckTest(a.testDottedDecimal());
	a.CheckTest(a.testNumericalIP());
        a.CheckTest(a.testAllZeroDotted() );
        a.CheckTest( a.testAllZeroNumerical() );
}

