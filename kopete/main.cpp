/*
    Kopete , The KDE Instant Messenger
    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Viva Chile Mierda!
    Started at Wed Dec 26 03:12:10 CLST 2001, Santiago de Chile

    Kopete    (c) 2002-2008 by the Kopete developers  <kopete-devel@kde.org>

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
#include <klocale.h>

#include "kopeteapplication.h"
#include "kopeteversion.h"

static const char description[] =
	I18N_NOOP( "Kopete, the KDE Instant Messenger" );

int main( int argc, char *argv[] )
{
	KAboutData aboutData( "kopete", 0, ki18n("Kopete"),
		KOPETE_VERSION_STRING, ki18n(description), KAboutData::License_GPL,
		ki18n("(c) 2001-2004, Duncan Mac-Vicar Prett\n(c) 2002-2015, Kopete Development Team"), ki18n("kopete-devel@kde.org"), "http://kopete.kde.org");

	aboutData.addAuthor ( ki18n("Pali Rohár"), ki18n("Developer and maintainer"), "pali.rohar@gmail.com" );
	aboutData.addAuthor ( ki18n("Duncan Mac-Vicar Prett"), ki18n("Developer and Project founder"), "duncan@kde.org", "http://www.mac-vicar.org/~duncan" );
	aboutData.addAuthor ( ki18n("Andre Duffeck"), ki18n("Developer, Yahoo plugin maintainer"), "duffeck@kde.org" );
	aboutData.addAuthor ( ki18n("Andy Goossens"), ki18n("Developer"), "andygoossens@telenet.be" );
	aboutData.addAuthor ( ki18n("Chris Howells"), ki18n("Developer, Connection status plugin author"), "howells@kde.org", "http://chrishowells.co.uk");
	aboutData.addAuthor ( ki18n("Cláudio da Silveira Pinheiro"), ki18n("Developer, Video device support"), "taupter@gmail.com", "http://taupter.homelinux.org" );
	aboutData.addAuthor ( ki18n("Gregg Edghill"), ki18n("Developer, MSN"), "gregg.edghill@gmail.com");
	aboutData.addAuthor ( ki18n("Grzegorz Jaskiewicz"), ki18n("Developer, Gadu plugin maintainer"), "gj@pointblue.com.pl" );
	aboutData.addAuthor ( ki18n("Gustavo Pichorim Boiko"), ki18n("Developer"), "gustavo.boiko@kdemail.net" );
	aboutData.addAuthor ( ki18n("Jason Keirstead"), ki18n("Developer"), "jason@keirstead.org", "http://www.keirstead.org");
	aboutData.addAuthor ( ki18n("Matt Rogers"), ki18n("Lead Developer, AIM and ICQ plugin maintainer"), "mattr@kde.org" );
	aboutData.addAuthor ( ki18n("Michel Hermier"), ki18n("IRC plugin maintainer"), "michel.hermier@wanadoo.fr" );
	aboutData.addAuthor ( ki18n("Michaël Larouche"), ki18n("Lead Developer, Telepathy and Messenger plugin maintainer"), "larouche@kde.org", "http://www.tehbisnatch.org/" );
	aboutData.addAuthor ( ki18n("Olivier Goffart"), ki18n("Lead Developer, MSN plugin maintainer"), "ogoffart@kde.org");
	aboutData.addAuthor ( ki18n("Ollivier Lapeyre Johann"), ki18n("Artist / Developer, Artwork maintainer"), "johann.ollivierlapeyre@gmail.com" );
	aboutData.addAuthor ( ki18n("Richard Smith"), ki18n("Developer, UI maintainer"), "kde@metafoo.co.uk" );
	aboutData.addAuthor ( ki18n("Tiago Salem Herrmann"), ki18n("Developer, WLM plugin maintainer"), "tiagosh@gmail.com");
	aboutData.addAuthor ( ki18n("Till Gerken"), ki18n("Developer, Jabber plugin maintainer"), "till@tantalo.net");
	aboutData.addAuthor ( ki18n("Will Stephenson"), ki18n("Lead Developer, GroupWise maintainer"), "wstephenson@kde.org" );
	aboutData.addAuthor ( ki18n("Rafael Fernández López"), ki18n("Developer"), "ereslibre@kde.org" );
	aboutData.addAuthor ( ki18n("Roman Jarosz"), ki18n("Developer, AIM and ICQ"), "kedgedev@centrum.cz" );
	aboutData.addAuthor ( ki18n("Charles Connell"), ki18n("Developer"), "charles@connells.org" );
	aboutData.addAuthor ( ki18n("Tejas Dinkar"), ki18n("Developer, Bonjour Plugin Maintainer"), "tejas@gja.in", "http://www.gja.in" );

	aboutData.addCredit ( ki18n("Vally8"), ki18n("Konki style author"), "vally8@gmail.com", "http://vally8.free.fr/" );
	aboutData.addCredit ( ki18n("Tm_T"), ki18n("Hacker style author"), "jussi.kekkonen@gmail.com");
	aboutData.addCredit ( ki18n("Luciash d' Being"), ki18n("Kopete's icon author") );
	aboutData.addCredit ( ki18n("Steve Cable"), ki18n("Sounds") );
	aboutData.addCredit ( ki18n("Jessica Hall"), ki18n("Kopete Docugoddess, Bug and Patch Testing.") );
	aboutData.addCredit ( ki18n("Justin Karneges"), ki18n("Iris Jabber Backend Library") );
	aboutData.addCredit ( ki18n("Tom Linsky"), ki18n("OscarSocket author"), "twl6@po.cwru.edu" );
	aboutData.addCredit ( ki18n("Olaf Lueg"), ki18n("Kmerlin MSN code") );
	aboutData.addCredit ( ki18n("Chetan Reddy"), ki18n("Former developer"), "chetan13@gmail.com" );
	aboutData.addCredit ( ki18n("Nick Betcher"), ki18n("Former developer, project co-founder"), "nbetcher@kde.org");
	aboutData.addCredit ( ki18n("Ryan Cumming"), ki18n("Former developer"), "ryan@kde.org" );
	aboutData.addCredit ( ki18n("Stefan Gehn"), ki18n("Former developer"), "metz@gehn.net", "http://metz.gehn.net" );
	aboutData.addCredit ( ki18n("Martijn Klingens"), ki18n("Former developer"), "klingens@kde.org" );
	aboutData.addCredit ( ki18n("Andres Krapf"), ki18n("Former developer"), "dae@chez.com" );
	aboutData.addCredit ( ki18n("Carsten Pfeiffer"), ki18n("Misc bugfixes and enhancements"), "pfeiffer@kde.org" );
	aboutData.addCredit ( ki18n("Zack Rusin"), ki18n("Former developer, original Gadu plugin author"), "zack@kde.org" );
	aboutData.addCredit ( ki18n("Richard Stellingwerff"), ki18n("Former developer"), "remenic@linuxfromscratch.org");
	aboutData.addCredit ( ki18n("Daniel Stone"), ki18n("Former developer, Jabber plugin author"), "daniel@fooishbar.org", "http://fooishbar.org");
	aboutData.addCredit ( ki18n("Chris TenHarmsel"), ki18n("Former developer, Oscar plugin"), "chris@tenharmsel.com");
	aboutData.addCredit ( ki18n("Hendrik vom Lehn"), ki18n("Former developer"), "hennevl@hennevl.de", "http://www.hennevl.de");
	aboutData.addCredit ( ki18n("Gav Wood"), ki18n("Former developer and WinPopup maintainer"), "gav@indigoarchive.net" );

	aboutData.setTranslator( ki18nc("NAME OF TRANSLATORS","Your names"),
		ki18nc("EMAIL OF TRANSLATORS","Your emails") );

	KCmdLineArgs::init( argc, argv, &aboutData );

	KCmdLineOptions options;
	options.add("noplugins", ki18n( "Do not load plugins. This option overrides all other options." ));
	options.add("noconnect", ki18n( "Disable auto-connection" ));
	options.add("autoconnect <accounts>", ki18n( "Auto-connect the specified accounts. Use a comma-separated list\n"
		"to auto-connect multiple accounts." ));
	options.add("disable <plugins>", ki18n( "Do not load the specified plugin. Use a comma-separated list\n"
		"to disable multiple plugins." ));
	options.add("load-plugins <plugins>", ki18n( "Load only the specified plugins. Use a comma-separated list\n"
		"to load multiple plugins. This option has no effect when\n"
		"--noplugins is set and overrides all other plugin related\n"
		"command line options." ));
	//	{ "url <url>",              I18N_NOOP( "Load the given Kopete URL" ), 0 },
//	{ "!+[plugin]",            I18N_NOOP( "Load specified plugins" ), 0 },
	options.add("!+[URL]", ki18n("URLs to pass to kopete / emoticon themes to install"));
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
	KUniqueApplication::addCmdLineOptions();

	KopeteApplication kopete;
	return kopete.exec();
}
// vim: set noet ts=4 sts=4 sw=4:
