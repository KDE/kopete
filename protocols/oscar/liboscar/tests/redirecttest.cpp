//Licensed under the GNU General Public License

#include <iostream>
#include "redirecttest.h"
#include <qcstring.h>

using namespace std;
RedirectTest::RedirectTest(int argc, char ** argv)
: QApplication( argc, argv ),
	m_transfer(0),
	m_task(0)
{
	m_root = new Task(0, true);
}

RedirectTest::~RedirectTest()
{
	delete m_root;
}

void RedirectTest::Setup()
{
	m_transfer = new SnacTransfer;
	m_task = new ServerRedirectTask( m_root );
}

void RedirectTest::Teardown()
{
	delete m_task;
	m_task = 0;
	m_transfer = 0;
}

bool RedirectTest::testHandleRedirect()
{
	Buffer* b = SetupBuffer(0x0010, "REDF$");
	m_transfer->setBuffer(b);

	m_task->setService(0x0010);
	m_task->setTransfer(m_transfer);
	return m_task->handleRedirect();
}

bool RedirectTest::testInvalidService()
{
	Buffer* b = SetupBuffer(0x4321, "REDF$");
	m_transfer->setBuffer(b);

	m_task->setService(0x0010);
	m_task->setTransfer(m_transfer);
	return !m_task->handleRedirect();
}

bool RedirectTest::testInvalidCookie()
{
	Buffer* b = SetupBuffer(0x0010, "");
	m_transfer->setBuffer(b);

	m_task->setService(0x0010);
	m_task->setTransfer(m_transfer);
	return !m_task->handleRedirect();
}

bool RedirectTest::testCookieIsSet()
{
	Buffer* b = SetupBuffer(0x0010, "grouch");
	m_transfer->setBuffer(b);

	m_task->setService(0x0010);
	m_task->setTransfer(m_transfer);
	m_task->handleRedirect();
	
	return !m_task->cookie().isEmpty();
}

Buffer* RedirectTest::SetupBuffer(WORD Service, QString Cookie)
{
	Buffer* b = new Buffer;
	b->addTLV16(0x000D, Service);
	b->addWord(0x0005);
	b->addWord(0x0010);
	b->addString("65.86.43.45:5190", 16);
	b->addWord(0x0006);
	b->addWord(Cookie.length());
	b->addString(Cookie.latin1(), Cookie.length());
	return b;
}

void RedirectTest::CheckTest(bool TestPassed)
{
	if ( TestPassed )
		cout << "passed" << endl;
	else
		cout << "failed" << endl;
}

int main(int argc, char ** argv)
{
	RedirectTest a( argc, argv );
	
	a.Setup();
	a.CheckTest(a.testHandleRedirect());
	a.Teardown();

	a.Setup();
	a.CheckTest(a.testInvalidService());
	a.Teardown();
		
	a.Setup();
	a.CheckTest(a.testInvalidCookie());
	a.Teardown();
	
	a.Setup();
	a.CheckTest(a.testCookieIsSet());
	a.Teardown();
}

