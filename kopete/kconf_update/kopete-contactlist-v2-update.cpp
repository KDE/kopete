/*
 * kconf_update app for migrating contact list version 1.0 contacts to
 * version 1.1. 
 * 
 * Copyright 2007 by Matt Rogers <mattr@kde.org>
 *
 * Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */
 
#include <QtXml>
 
#include <QFile>
#include <QUuid>
#include <QString>
#include <QtDebug>
#include <QtGlobal>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KStandardDirs>
#include <KLocalizedString>

/* function prototypes */
void convertContactList();

/* globals are nasty, but I don't care. this is just a quick hack */
bool debugModeOn = false;

/* text streams for debug output */
static QTextStream qcout( stdout, QIODevice::WriteOnly );

int main( int argc, char* argv[] )
{
    qcout.setCodec(QTextCodec::codecForName("UTF-8"));
    
    KAboutData aboutData("kopete-contactlist-v2-update", "",
                         ki18n("Kopete Contactlist Updater"),
                         "Kopete Contactlist Updater 0.1",
                         ki18n("Keeps the contact list up to date with trunk changes"),
                         KAboutData::License_GPL,
                         ki18n("2007"),
                         ki18n(""),
                         "http://kopete.kde.org",
                         "submit@bugs.kde.org");
                     
    KCmdLineArgs::init(argc, argv, &aboutData);
    
    KCmdLineOptions options;
	options.add("z", ki18n( "Debug mode. Write new XML to stdout instead of back to the contactlist.xml file" ));
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app(false /* no gui here */);
    
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("z"))
        debugModeOn = true;
    
    convertContactList();
    return 0;
}

void convertContactList()
{
    QString contactListFile = KStandardDirs::locateLocal("data", "kopete/contactlist.xml");
    if ( !QFile::exists( contactListFile )  )
    {
        qWarning() << "No contact list file to convert";
        return;
    }
        
    QFile xmlFile(contactListFile);
    QDomDocument contactList;
    if (!contactList.setContent(&xmlFile, true /* namespace processing */))
    {
        qWarning() << "Unable to parse contactlist.xml file";
        xmlFile.close();
        return;
    }
    xmlFile.close(); //we're done with this file. we're create a temporary and save

    QDomElement rootElement = contactList.documentElement();
    if ( rootElement.hasAttribute("version") )
    {
        QString clVersion = rootElement.attribute(QLatin1String("version"));
        if (clVersion == "1.1")
            return;
    }
    rootElement.setAttribute(QString("version"), "1.1");
    
    QDomNodeList metaContactElements = contactList.elementsByTagName(QLatin1String("meta-contact"));
    for (int i = 0; i < metaContactElements.count(); i++)
    {
        QDomNode node = metaContactElements.at(i);
        QDomElement element = node.toElement();
        if (element.hasAttribute(QLatin1String("contactId")))
        {
            QString kabcId;
            QString contactId = element.attribute("contactId");
            QUuid newContactId = QUuid::createUuid();

            if (!contactId.contains(':'))
                element.setAttribute(QLatin1String("kabcId"), contactId);
            
            element.setAttribute(QLatin1String("contactId"), newContactId.toString());
        }
    }
    
    if ( debugModeOn )
    {
        qcout << contactList.toString(4 /*indent*/);
    }
    else
    {
        QTemporaryFile newFile;
        newFile.open();
        newFile.write( contactList.toByteArray(4 /*indent*/) );
        newFile.close();
        xmlFile.remove();
        newFile.rename( contactListFile );
    }
}
