/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Wed Dec 26 03:12:10 CLST 2001
    copyright            : (C) 2001 by Duncan Mac-Vicar Prett
    email                : duncan@puc.cl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include "kopete.h"

static const char *description =
	I18N_NOOP("Kopete, the KDE Messenger");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
static const char *version="0.0.1";	
	
static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

  KAboutData aboutData( "kopete", I18N_NOOP("Kopete"),
    version, description, KAboutData::License_GPL,
    "(c) 2001, Duncan Mac-Vicar Prett", 0, 0, "duncan@puc.cl");
  aboutData.addAuthor("Duncan Mac-Vicar Prett","Author, core developer", "duncan@puc.cl","http://www.mac-vicar.com");
  aboutData.addAuthor ("Nick Betcher", "core developer","nbetcher@usinternet.com", "http://www.kdedevelopers.net" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	
  Kopete kopete;
//kopete->show();
  kopete.exec();
}
