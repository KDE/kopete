//
// C++ Implementation: clientstream_test
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
// Licensed under the GNU General Public License

#ifndef clientstream_test_h
#define clientstream_test_h

#include <qglobal.h>
#include <qapplication.h>
#include <qtimer.h>

#include "yahooclientstream.h"
#include "yahooconnector.h"

#include "coreprotocol.h"

#define QT_FATAL_ASSERT 1

class ClientStreamTest : public QApplication
{
Q_OBJECT
public:
	ClientStreamTest(int argc, char ** argv);
	
	~ClientStreamTest();
	
	bool isConnected();

public slots:
	void slotDoTest();
	
	void slotConnected();
	
	//void slotWarning(int warning);

	//void slotsend(int layer);

private:
	KNetworkConnector *myConnector;
	ClientStream *myTestObject;
	bool connected;
};

#endif
