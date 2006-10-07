/*
    Kopete , The KDE Instant Messenger
    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Viva Chile Mierda!
    Started at Wed Dec 26 03:12:10 CLST 2001, Santiago de Chile

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

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
#include "kopeteapplication.h"

#include <dcopclient.h>
#include "kopeteiface.h"
#include "kimifaceimpl.h"
#include "kopeteversion.h"

static const char description[] =
	I18N_NOOP( "Kopete, the KDE Instant Messenger" );

static KCmdLineOptions options[] =
{
	{ "noplugins",              I18N_NOOP( "Do not load plugins. This option overrides all other options." ), 0 },
	{ "noconnect",              I18N_NOOP( "Disable auto-connection" ), 0 },
	{ "autoconnect <accounts>", I18N_NOOP( "Auto-connect the specified accounts. Use a comma-separated list\n"
		"to auto-connect multiple accounts." ), 0 },
	{ "disable <plugins>",      I18N_NOOP( "Do not load the specified plugin. Use a comma-separated list\n"
		"to disable multiple plugins." ), 0 },
	{ "load-plugins <plugins>", I18N_NOOP( "Load only the specified plugins. Use a comma-separated list\n"
		"to load multiple plugins. This option has no effect when\n"
		"--noplugins is set and overrides all other plugin related\n"
		"command line options." ), 0 },
//	{ "url <url>",              I18N_NOOP( "Load the given Kopete URL" ), 0 },
//	{ "!+[plugin]",            I18N_NOOP( "Load specified plugins" ), 0 },
	{ "!+[URL]",                 I18N_NOOP("URLs to pass to kopete / emoticon themes to install"), 0},
	KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "kopete", I18N_NOOP("Kopete"),
		KOPETE_VERSION_STRING, description, KAboutData::License_GPL,
		I18N_NOOP("(c) 2001-2004, Duncan Mac-Vicar Prett\n(c) 2002-2005, Kopete Development Team"), "kopete-devel@kde.org", "http://kopete.kde.org");

	aboutData.addAuthor ( "Duncan Mac-Vicar Prett", I18N_NOOP("Developer and Project founder"), "duncan@kde.org", "http://www.mac-vicar.org/~duncan" );
	aboutData.addAuthor ( "Andre Duffeck", I18N_NOOP("Developer, Yahoo plugin maintainer"), "andre@duffeck.de" );
	aboutData.addAuthor ( "Andy Goossens", I18N_NOOP("Developer"), "andygoossens@telenet.be" );
	aboutData.addAuthor ( "Chetan Reddy", I18N_NOOP("Developer, Yahoo"), "chetan13@gmail.com" );
	aboutData.addAuthor ( "Chris Howells", I18N_NOOP("Developer, Connection status plugin author"), "howells@kde.org", "http://chrishowells.co.uk");
	aboutData.addAuthor ( "Cláudio da Silveira Pinheiro", I18N_NOOP("Developer, Video device support"), "taupter@gmail.com", "http://taupter.homelinux.org" );
	aboutData.addAuthor ( "Gregg Edghill", I18N_NOOP("Developer, MSN"), "gregg.edghill@gmail.com");
	aboutData.addAuthor ( "Grzegorz Jaskiewicz", I18N_NOOP("Developer, Gadu plugin maintainer"), "gj@pointblue.com.pl" );
	aboutData.addAuthor ( "Jason Keirstead", I18N_NOOP("Developer"), "jason@keirstead.org", "http://www.keirstead.org");
	aboutData.addAuthor ( "Matt Rogers", I18N_NOOP("Lead Developer, AIM and ICQ plugin maintainer"), "mattr@kde.org" );
	aboutData.addAuthor ( "Michel Hermier", I18N_NOOP("IRC plugin maintainer"), "michel.hermier@wanadoo.fr" );
	aboutData.addAuthor ( "Michaël Larouche", I18N_NOOP("Lead Developer"), "larouche@kde.org", "http://www.tehbisnatch.org/" );
	aboutData.addAuthor ( "Olivier Goffart", I18N_NOOP("Lead Developer, MSN plugin maintainer"), "ogoffart @ kde.org");
	aboutData.addAuthor ( "Ollivier Lapeyre Johann", I18N_NOOP("Artist / Developer, Artwork maintainer"), "johann.ollivierlapeyre@gmail.com" );
	aboutData.addAuthor ( "Richard Smith", I18N_NOOP("Developer, UI maintainer"), "kde@metafoo.co.uk" );
	aboutData.addAuthor ( "Till Gerken", I18N_NOOP("Developer, Jabber plugin maintainer"), "till@tantalo.net");
	aboutData.addAuthor ( "Will Stephenson", I18N_NOOP("Lead Developer, GroupWise maintainer"), "lists@stevello.free-online.co.uk" );
	
	aboutData.addCredit ( "Vally8", I18N_NOOP("Konki style author"), "vally8@gmail.com", "http://vally8.free.fr/" );
	aboutData.addCredit ( "Tm_T", I18N_NOOP("Hacker style author"), "jussi.kekkonen@gmail.com");
	aboutData.addCredit ( "Luciash d' Being", I18N_NOOP("Kopete's icon author") );
	aboutData.addCredit ( "Steve Cable", I18N_NOOP("Sounds") );
	aboutData.addCredit ( "Jessica Hall", I18N_NOOP("Kopete Docugoddess, Bug and Patch Testing.") );
	aboutData.addCredit ( "Justin Karneges", I18N_NOOP("Iris Jabber Backend Library") );
	aboutData.addCredit ( "Tom Linsky", I18N_NOOP("OscarSocket author"), "twl6@po.cwru.edu" );
	aboutData.addCredit ( "Olaf Lueg", I18N_NOOP("Kmerlin MSN code") );
	aboutData.addCredit ( "Nick Betcher", I18N_NOOP("Former developer, project co-founder"), "nbetcher@kde.org");
	aboutData.addCredit ( "Ryan Cumming", I18N_NOOP("Former developer"), "ryan@kde.org" );
	aboutData.addCredit ( "Stefan Gehn", I18N_NOOP("Former developer"), "metz@gehn.net", "http://metz.gehn.net" );
	aboutData.addCredit ( "Martijn Klingens", I18N_NOOP("Former developer"), "klingens@kde.org" );
	aboutData.addCredit ( "Andres Krapf", I18N_NOOP("Former developer"), "dae@chez.com" );
	aboutData.addCredit ( "Carsten Pfeiffer", I18N_NOOP("Misc bugfixes and enhancements"), "pfeiffer@kde.org" );
	aboutData.addCredit ( "Zack Rusin", I18N_NOOP("Former developer, original Gadu plugin author"), "zack@kde.org" );
	aboutData.addCredit ( "Richard Stellingwerff", I18N_NOOP("Former developer"), "remenic@linuxfromscratch.org");
	aboutData.addCredit ( "Daniel Stone", I18N_NOOP("Former developer, Jabber plugin author"), "daniel@fooishbar.org", "http://fooishbar.org");
	aboutData.addCredit ( "Chris TenHarmsel", I18N_NOOP("Former developer, Oscar plugin"), "tenharmsel@users.sourceforge.net");
	aboutData.addCredit ( "Hendrik vom Lehn", I18N_NOOP("Former developer"), "hennevl@hennevl.de", "http://www.hennevl.de");
	aboutData.addCredit ( "Gav Wood", I18N_NOOP("Former developer and WinPopup maintainer"), "gav@indigoarchive.net" );

	aboutData.setTranslator( I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"),
		I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails") );

	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
	KUniqueApplication::addCmdLineOptions();

	KopeteApplication kopete;
	new KIMIfaceImpl();
	kapp->dcopClient()->registerAs( "kopete", false );
	kapp->dcopClient()->setDefaultObject( (new KopeteIface())->objId() ); // Has to be called before exec

	kopete.exec();
}
// vim: set noet ts=4 sts=4 sw=4:
