/*
    Tests for the KopetePassword class

    Copyright (c) 2003      by Richard Smith          <kde@metafoo.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qtextstream.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "kopetepassword.h"

static QTextStream _out( stdout, IO_WriteOnly );

static KCmdLineOptions opts[] =
{
 { "id <passwordID>", I18N_NOOP("Unique id of password"), "PasswordTest" },
 { "group <group>", I18N_NOOP("Config group to store settings in"), "TestGroup" },
 { "set <new>", I18N_NOOP("Set password to new"), "" },
 { "error", I18N_NOOP("Claim password was erroneous"), "" },
};

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "kopetepasswordtest", "kopetepasswordtest", "version" );
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( opts );
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	KApplication app( "kopetepasswordtest" );
	
	QString id = args->getOption("id");
	bool setPassword = args->isSet("set");
	QString newPwd = args->getOption("set");
	QString group = args->getOption("group");
	bool error = args->isSet("error");

	KopetePassword pwd( group, id, id );
	QString pass = pwd.retrieve( error );

	if ( !pass.isNull() )
		_out << "Read password: " << pass << endl;
	else
		_out << "Could not read a password" << endl;

	if ( setPassword )
	{
		if ( newPwd == "" )
		{
			_out << "Clearing password" << endl;
			newPwd = QString::null;
		}
		else
		{
			_out << "Setting password to " << newPwd << endl;
		}
		pwd.set( newPwd );
		pass = pwd.retrieve();
		if( pass == newPwd )
			_out << "Password successfully set." << endl;
		else
			_out << "Failed: password ended up as " << pass << endl;
	}

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

