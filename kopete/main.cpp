/*
    Kopete , The KDE Instant Messenger
    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Viva Chile Mierda!
    Started at Wed Dec 26 03:12:10 CLST 2001, Santiago de Chile

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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include "kopete.h"

#include <dcopclient.h>
#include "kopeteiface.h"

#define KOPETE_VERSION "0.6.90cvs >= 20030511"

static const char *description =
	I18N_NOOP("Kopete, the KDE Instant Messenger");

static KCmdLineOptions options[] =
{
	{ "noplugins", I18N_NOOP("Do not load plugins"), 0 },
	{ "noconnect" , I18N_NOOP("Disable auto-connection") , 0 },
//	{ "connect <account>" , I18N_NOOP("auto-connect specified account") , 0 }, //TODO
	{ "disable <plugin>", I18N_NOOP("Do not load specified plugin"), 0 },
	{ "!+[plugin]", I18N_NOOP("Load specified plugins"), 0 },
	KCmdLineLastOption
};

int main(int argc, char *argv[])
{
	KAboutData aboutData( "kopete", I18N_NOOP("Kopete"),
		KOPETE_VERSION, description, KAboutData::License_GPL,
		I18N_NOOP("(c) 2001,2003, Duncan Mac-Vicar Prett\n(c) 2002,2003, The Kopete Development Team"), "kopete-devel@kde.org", "http://kopete.kde.org");

	aboutData.addAuthor ( "Duncan Mac-Vicar Prett", I18N_NOOP("Original author, core developer"), "duncan@kde.org", "http://www.mac-vicar.org/~duncan" );
	aboutData.addAuthor ( "Nick Betcher", I18N_NOOP("Core developer, fastest plugin developer on earth."), "nbetcher@kde.org");
	aboutData.addAuthor ( "Martijn Klingens", I18N_NOOP("Core developer"), "klingens@kde.org" );
	aboutData.addAuthor ( "Daniel Stone", I18N_NOOP("Core developer, Jabber plugin"), "dstone@kde.org", "http://raging.dropbear.id.au/daniel/");
	aboutData.addAuthor ( "Till Gerken", I18N_NOOP("Core developer, Jabber plugin"), "till@tantalo.net");
	aboutData.addAuthor ( "Olivier Goffart", I18N_NOOP("Core developer, MSN Plugin"), "ogoffart@tiscalinet.be");
	aboutData.addAuthor ( "Stefan Gehn", I18N_NOOP("Developer"), "metz@gehn.net", "http://metz.gehn.net" );
	aboutData.addAuthor ( "Gav Wood", I18N_NOOP("Winpopup plugin"), "gjw102@york.ac.uk" );
	aboutData.addAuthor ( "Zack Rusin", I18N_NOOP("Core developer, Gadu plugin"), "zack@kde.org" );
	aboutData.addAuthor ( "Chris TenHarmsel", I18N_NOOP("Developer"), "tenharmsel@users.sourceforge.net", "http://bemis.kicks-ass.net");
	aboutData.addAuthor ( "Chris Howells", I18N_NOOP("Connection status plugin author"), "howells@kde.org", "http://chrishowells.co.uk");
	aboutData.addAuthor ( "Jason Keirstead", I18N_NOOP("Core developer"), "jason@keirstead.org", "http://www.keirstead.org");
	aboutData.addAuthor ( "Andy Goossens", I18N_NOOP("Developer"), "andygoossens@pandora.be" );
	aboutData.addAuthor ( "Will Stephenson", I18N_NOOP("Developer, Icons, Plugins"), "lists@stevello.free-online.co.uk" );

	aboutData.addCredit ( "Luciash d' Being", I18N_NOOP("Icon Author") );
	aboutData.addCredit ( "Vladimir Shutoff", I18N_NOOP("SIM icq library") );
	aboutData.addCredit ( "Herwin Jan Steehouwer", I18N_NOOP("KxEngine icq code") );
	aboutData.addCredit ( "Olaf Lueg", I18N_NOOP("Kmerlin MSN code") );
	//aboutData.addCredit ( "Neil Stevens", I18N_NOOP("TAim engine AIM code") );
	aboutData.addCredit ( "Justin Karneges", I18N_NOOP("Psi Jabber code") );
	aboutData.addCredit ( "Steve Cable", I18N_NOOP("Sounds") );

	aboutData.addCredit ( "Ryan Cumming", I18N_NOOP("Old developer"), "ryan@kde.org" );
	aboutData.addCredit ( "Richard Stellingwerff", I18N_NOOP("Old Developer"), "remenic@linuxfromscratch.org");
	aboutData.addCredit ( "Hendrik vom Lehn", I18N_NOOP("Old Developer"), "hennevl@hennevl.de", "http://www.hennevl.de");
	aboutData.addCredit ( "Andres Krapf", I18N_NOOP("Old Developer"), "dae@chez.com" );
	aboutData.addCredit ( "Carsten Pfeiffer", I18N_NOOP("Misc Bugfixes and Enhancelets"), "pfeiffer@kde.org" );

	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
	KUniqueApplication::addCmdLineOptions();

	Kopete kopete;
	kapp->dcopClient()->setDefaultObject( (new KopeteIface())->objId() ); // Has to be called before exec
	kopete.exec();
}
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

