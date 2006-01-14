//
// C++ Implementation: clientstream_test
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
// Licensed under the GNU General Public License

#ifndef ssitest_h
#define ssitest_h

#include <qglobal.h>
#include <qapplication.h>
#include <qtimer.h>

#include "ssimanager.h"

#define QT_FATAL_ASSERT 1

class SSITest : public QApplication
{
Q_OBJECT
public:
	SSITest(int argc, char ** argv);
	
	~SSITest();
	
	void testIt();
private:
	SSIManager *m_manager;
};

#endif
