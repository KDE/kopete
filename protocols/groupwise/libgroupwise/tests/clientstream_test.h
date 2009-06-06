//
// C++ Implementation: clientstream_test
//
// Description: 
//
//
// Author: Kopete Developers <kopete-devel@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef clientstream_test_h
#define clientstream_test_h

#include <qglobal.h>
#include <qapplication.h>
#include <qtimer.h>

#include "gwclientstream.h"
#include "gwconnector.h"
#include <QtCrypto>
#include "qcatlshandler.h"
#include "requestfactory.h"
#include "request.h"
#include "usertransfer.h"

#include "coreprotocol.h"

#define QT_FATAL_ASSERT 1

class ClientStreamTest : public QApplication
{
Q_OBJECT
public:
	ClientStreamTest(int argc, char ** argv);
	
	~ClientStreamTest();

public slots:
	void slotDoTest();
	
	void slotConnected();
	
	void slotWarning(int warning);

	void slotsend(int layer);
	void slotTLSHandshaken();
	
private:
	KNetworkConnector *myConnector;
	QCA::TLS *myTLS;
	QCATLSHandler *myTLSHandler;
	ClientStream *myTestObject;
};

#endif
