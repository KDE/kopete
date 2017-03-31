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


#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include <KDBusService>

#include "kopeteapplication.h"
#include "kopeteversion.h"

static const char description[]
    = I18N_NOOP("Kopete, the KDE Instant Messenger");

int main(int argc, char *argv[])
{
    //QApplication app(argc, argv);
    KopeteApplication kopete(argc, argv);

    KLocalizedString::setApplicationDomain("kopete");
    KAboutData aboutData("kopete", i18n("Kopete"),
                         QStringLiteral(KOPETE_VERSION_STRING), i18n(description), KAboutLicense::GPL,
                         i18n("(c) 2001-2004, Duncan Mac-Vicar Prett\n(c) 2002-2017, Kopete Development Team"),
                         i18n("kopete-devel@kde.org"), "http://kopete.kde.org");

    aboutData.addAuthor(i18n("Pali Rohár"), i18n("Developer and maintainer"), "pali.rohar@gmail.com");
    aboutData.addAuthor(i18n("Duncan Mac-Vicar Prett"), i18n("Developer and Project founder"), "duncan@kde.org", "http://www.mac-vicar.org/~duncan");
    aboutData.addAuthor(i18n("Andre Duffeck"), i18n("Developer, Yahoo plugin maintainer"), "duffeck@kde.org");
    aboutData.addAuthor(i18n("Andy Goossens"), i18n("Developer"), "andygoossens@telenet.be");
    aboutData.addAuthor(i18n("Chris Howells"), i18n("Developer, Connection status plugin author"), "howells@kde.org", "http://chrishowells.co.uk");
    aboutData.addAuthor(i18n("Cláudio da Silveira Pinheiro"), i18n("Developer, Video device support"), "taupter@gmail.com", "http://taupter.homelinux.org");
    aboutData.addAuthor(i18n("Gregg Edghill"), i18n("Developer, MSN"), "gregg.edghill@gmail.com");
    aboutData.addAuthor(i18n("Grzegorz Jaskiewicz"), i18n("Developer, Gadu plugin maintainer"), "gj@pointblue.com.pl");
    aboutData.addAuthor(i18n("Gustavo Pichorim Boiko"), i18n("Developer"), "gustavo.boiko@kdemail.net");
    aboutData.addAuthor(i18n("Jason Keirstead"), i18n("Developer"), "jason@keirstead.org", "http://www.keirstead.org");
    aboutData.addAuthor(i18n("Matt Rogers"), i18n("Lead Developer, AIM and ICQ plugin maintainer"), "mattr@kde.org");
    aboutData.addAuthor(i18n("Michel Hermier"), i18n("IRC plugin maintainer"), "michel.hermier@wanadoo.fr");
    aboutData.addAuthor(i18n("Michaël Larouche"), i18n("Lead Developer, Telepathy and Messenger plugin maintainer"), "larouche@kde.org", "http://www.tehbisnatch.org/");
    aboutData.addAuthor(i18n("Olivier Goffart"), i18n("Lead Developer, MSN plugin maintainer"), "ogoffart@kde.org");
    aboutData.addAuthor(i18n("Ollivier Lapeyre Johann"), i18n("Artist / Developer, Artwork maintainer"), "johann.ollivierlapeyre@gmail.com");
    aboutData.addAuthor(i18n("Richard Smith"), i18n("Developer, UI maintainer"), "kde@metafoo.co.uk");
    aboutData.addAuthor(i18n("Tiago Salem Herrmann"), i18n("Developer, WLM plugin maintainer"), "tiagosh@gmail.com");
    aboutData.addAuthor(i18n("Till Gerken"), i18n("Developer, Jabber plugin maintainer"), "till@tantalo.net");
    aboutData.addAuthor(i18n("Will Stephenson"), i18n("Lead Developer, GroupWise maintainer"), "wstephenson@kde.org");
    aboutData.addAuthor(i18n("Rafael Fernández López"), i18n("Developer"), "ereslibre@kde.org");
    aboutData.addAuthor(i18n("Roman Jarosz"), i18n("Developer, AIM and ICQ"), "kedgedev@centrum.cz");
    aboutData.addAuthor(i18n("Charles Connell"), i18n("Developer"), "charles@connells.org");
    aboutData.addAuthor(i18n("Tejas Dinkar"), i18n("Developer, Bonjour Plugin Maintainer"), "tejas@gja.in", "http://www.gja.in");

    aboutData.addCredit(i18n("Vally8"), i18n("Konki style author"), "vally8@gmail.com", "http://vally8.free.fr/");
    aboutData.addCredit(i18n("Tm_T"), i18n("Hacker style author"), "jussi.kekkonen@gmail.com");
    aboutData.addCredit(i18n("Luciash d' Being"), i18n("Kopete's icon author"));
    aboutData.addCredit(i18n("Steve Cable"), i18n("Sounds"));
    aboutData.addCredit(i18n("Jessica Hall"), i18n("Kopete Docugoddess, Bug and Patch Testing."));
    aboutData.addCredit(i18n("Justin Karneges"), i18n("Iris Jabber Backend Library"));
    aboutData.addCredit(i18n("Tom Linsky"), i18n("OscarSocket author"), "twl6@po.cwru.edu");
    aboutData.addCredit(i18n("Olaf Lueg"), i18n("Kmerlin MSN code"));
    aboutData.addCredit(i18n("Chetan Reddy"), i18n("Former developer"), "chetan13@gmail.com");
    aboutData.addCredit(i18n("Nick Betcher"), i18n("Former developer, project co-founder"), "nbetcher@kde.org");
    aboutData.addCredit(i18n("Ryan Cumming"), i18n("Former developer"), "ryan@kde.org");
    aboutData.addCredit(i18n("Stefan Gehn"), i18n("Former developer"), "metz@gehn.net", "http://metz.gehn.net");
    aboutData.addCredit(i18n("Martijn Klingens"), i18n("Former developer"), "klingens@kde.org");
    aboutData.addCredit(i18n("Andres Krapf"), i18n("Former developer"), "dae@chez.com");
    aboutData.addCredit(i18n("Carsten Pfeiffer"), i18n("Misc bugfixes and enhancements"), "pfeiffer@kde.org");
    aboutData.addCredit(i18n("Zack Rusin"), i18n("Former developer, original Gadu plugin author"), "zack@kde.org");
    aboutData.addCredit(i18n("Richard Stellingwerff"), i18n("Former developer"), "remenic@linuxfromscratch.org");
    aboutData.addCredit(i18n("Daniel Stone"), i18n("Former developer, Jabber plugin author"), "daniel@fooishbar.org", "http://fooishbar.org");
    aboutData.addCredit(i18n("Chris TenHarmsel"), i18n("Former developer, Oscar plugin"), "chris@tenharmsel.com");
    aboutData.addCredit(i18n("Hendrik vom Lehn"), i18n("Former developer"), "hennevl@hennevl.de", "http://www.hennevl.de");
    aboutData.addCredit(i18n("Gav Wood"), i18n("Former developer and WinPopup maintainer"), "gav@indigoarchive.net");

    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"),
                            i18nc("EMAIL OF TRANSLATORS", "Your emails"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("noplugins"), i18n("Do not load plugins. This option overrides all other options.")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("noconnect"), i18n("Disable auto-connection")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("!+[URL]"), i18n("URLs to pass to kopete / emoticon themes to install")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("autoconnect"), i18n("Auto-connect the specified accounts. Use a comma-separated list\n"
                                                                                            "to auto-connect multiple accounts.")));

    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("disable"), i18n("Do not load the specified plugin. Use a comma-separated list\n"
                                                                                        "to disable multiple plugins.")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("load-plugins"), i18n("Load only the specified plugins. Use a comma-separated list\n"
                                                                                             "to load multiple plugins. This option has no effect when\n"
                                                                                             "--noplugins is set and overrides all other plugin related\n"
                                                                                             "command line options.")));
    aboutData.setupCommandLine(&parser);
    parser.process(kopete);
    aboutData.processCommandLine(&parser);


    KDBusService service(KDBusService::Unique);

    return kopete.exec();
}

// vim: set noet ts=4 sts=4 sw=4:
