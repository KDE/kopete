//
// C++ Implementation: clientstream_test
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
// Licensed under the GNU General Public License

#ifndef logintest_h
#define logintest_h

#include <qglobal.h>
#include <qapplication.h>
#include <qtimer.h>

#include "client.h"
#include "coreprotocol.h"

#define QT_FATAL_ASSERT 1

class LoginTest : public QApplication
{
Q_OBJECT
public:
	LoginTest(int argc, char ** argv);
	
	~LoginTest();
	
	bool isConnected() { return connected; }

public slots:
	void slotDoTest();
	
	void slotConnected();
	
	//void slotWarning(int warning);

	//void slotsend(int layer);

private:
	Client* myClient;
	
	bool connected;
};

#endif
