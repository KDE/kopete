//
// C++ Implementation: clientstream_test
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
// Licensed under the GNU General Public License

#ifndef RedirectTest_h
#define RedirectTest_h

#include <qglobal.h>
#include <qapplication.h>
#include <qtimer.h>

#include "transfer.h"
#include "oscartypes.h"
#include "serverredirecttask.h"
#include "task.h"

#define QT_FATAL_ASSERT 1

class RedirectTest : public QApplication
{
public:
	RedirectTest(int argc, char ** argv);
	~RedirectTest();
	
	bool testHandleRedirect();
	bool testInvalidService();
	bool testInvalidCookie();
	bool testCookieIsSet();

	void Setup();
	void Teardown();
	
	void CheckTest(bool TestPassed);
	
private:
	//Helper functions
	Buffer* SetupBuffer(WORD Service, QString Cookie);

	Task *m_root;
	SnacTransfer * m_transfer;
	ServerRedirectTask* m_task;
	
	bool connected;
};

#endif
