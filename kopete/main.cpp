/*
	Kopete , The KDE Instant Messenger

	Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

	Viva Chile Mierda!
	Started at Wed Dec 26 03:12:10 CLST 2001, Santiago de Chile

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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
#include "kopete.h"

#include <dcopclient.h>
#include "kopeteiface.h"

static const char *description =
	I18N_NOOP("Kopete, the KDE Instant Messenger");

#define KOPETE_VERSION "0.5"
	
static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	KAboutData aboutData( "kopete", I18N_NOOP("Kopete"),
		KOPETE_VERSION, description, KAboutData::License_GPL,
		"(c) 2001,2002, Duncan Mac-Vicar Prett\n(c) 2002, The Kopete Development Team", "kopete-devel@kde.org", "http://kopete.kde.org");

	aboutData.addAuthor ( "Duncan Mac-Vicar Prett", I18N_NOOP("Original author, core developer"), "duncan@kde.org", "http://www.mac-vicar.com" );
	aboutData.addAuthor ( "Nick Betcher", I18N_NOOP("Core developer, fastest plugin developer on earth."), "nbetcher@kde.org", "http://www.kdedevelopers.net" );
	aboutData.addAuthor ( "Ryan Cumming", I18N_NOOP("Core developer"), "bodnar42@phalynx.dhs.org" );
	aboutData.addAuthor ( "Martijn Klingens", I18N_NOOP("Core developer"), "klingens@kde.org" );
	aboutData.addAuthor ( "Richard Stellingwerff", I18N_NOOP("Developer"), "remenic@linuxfromscratch.org");
	aboutData.addAuthor ( "Daniel Stone", I18N_NOOP("Core developer, Jabber plugin"), "dstone@kde.org", "http://raging.dropbear.id.au/daniel/");
	aboutData.addAuthor ( "Till Gerken", I18N_NOOP("Core developer, Jabber plugin"), "till@tantalo.net");
	aboutData.addAuthor ( "Olivier Goffart", I18N_NOOP("Core developer"), "ogoffart@tiscalinet.be");
	aboutData.addAuthor ( "Hendrik vom Lehn", I18N_NOOP("Developer"), "hennevl@hennevl.de", "http://www.hennevl.de");
	aboutData.addAuthor ( "Stefan Gehn", I18N_NOOP("Developer"), "sgehn@gmx.net", "http://metz81.mine.nu" );
	aboutData.addAuthor ( "Andres Krapf", I18N_NOOP("Developer"), "dae@chez.com" );
	aboutData.addAuthor ( "Gav Wood", I18N_NOOP("Winpopup plugin"), "gjw102@york.ac.uk" );
    aboutData.addAuthor ( "Zack Rusin", I18N_NOOP("Core developer, Gadu plugin"), "zack@kde.org" );
	aboutData.addAuthor ( "Carsten Pfeiffer", I18N_NOOP("Misc Bugfixes and Enhancelets"), "pfeiffer@kde.org" );

	aboutData.addCredit ( "Luciash d' Being", I18N_NOOP("Icon Author") );
	aboutData.addCredit ( "Vladimir Shutoff", I18N_NOOP("SIM icq library") );
    aboutData.addCredit ( "Herwin Jan Steehouwer", I18N_NOOP("KxEngine icq code") );
	aboutData.addCredit ( "Olaf Lueg", I18N_NOOP("Kmerlin MSN code") );
	aboutData.addCredit ( "Neil Stevens", I18N_NOOP("TAim engine AIM code") );
	aboutData.addCredit ( "Justin Karneges", I18N_NOOP("Psi Jabber code") );

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

