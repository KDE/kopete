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

#include "kopetepassword.h"

#include <qtextstream.h>
#include <qpixmap.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

static QTextStream _out( stdout, IO_WriteOnly );

static KCmdLineOptions opts[] =
{
 { "id <id>", I18N_NOOP("Config group to store password in"), "TestAccount" },
 { "set <new>", I18N_NOOP("Set password to new"), "" },
 { "error", I18N_NOOP("Claim password was erroneous"), "" },
 { "prompt <prompt>", I18N_NOOP("Password prompt"), "Enter a password" },
 { "image <filename>", I18N_NOOP("Image to display in password dialog"), "" },
 KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "kopetepasswordtest", "kopetepasswordtest", "version" );
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( opts );
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	KApplication app( "kopetepasswordtest" );
	
	bool setPassword = args->isSet("set");
	QString newPwd = args->getOption("set");
	QString passwordId = args->getOption("id");
	bool error = args->isSet("error");
	QString prompt = args->getOption("prompt");
	QPixmap image = QString(args->getOption("image"));

	_out << (image.isNull() ? "image is null" : "image is valid") << endl;

	KopetePassword pwd( passwordId );
	QString pass = pwd.retrieve( image, prompt, error );

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
		pass = pwd.retrieve( image, i18n("Hopefully this popped up because you set the password to the empty string.") );
		if( pass == newPwd )
			_out << "Password successfully set." << endl;
		else
			_out << "Failed: password ended up as " << pass << endl;
	}

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

