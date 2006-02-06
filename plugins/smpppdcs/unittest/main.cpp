/*
	main.cpp
 
	Copyright (c) 2006      by Heiko Schaefer        <heiko@rangun.de>
 
	Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; version 2 of the License.               *
	*                                                                       *
	*************************************************************************
*/

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kunittest/runnergui.h>

#include "clienttest.h"

static const char description[] = I18N_NOOP("SMPPPDClientTests");
static const char version[] = "0.1";
static KCmdLineOptions options[] = { KCmdLineLastOption };

int main( int argc, char** argv ) {
    KAboutData about("SMPPPDClientTests", I18N_NOOP("SMPPPDClientTests"), version, description,
                     KAboutData::License_BSD, "(C) 2006 Heiko Schäfer", 0, 0, "heiko@rangun.de");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    KUnitTest::Runner::registerTester("ClientTest", new ClientTest);

    KUnitTest::RunnerGUI runner(0);
    runner.show();
    app.setMainWidget(&runner);

    return app.exec();
}
