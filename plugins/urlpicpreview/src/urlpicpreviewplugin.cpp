/*
    urlpicpreviewplugin.cpp
 
    Copyright (c) 2005      by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

// Qt
#include <qimage.h>
#include <qregexp.h>

// KDE
#include <kurl.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kimageio.h>
#include <kapplication.h>
#include <kgenericfactory.h>

// KIO
#include <kio/netaccess.h>

// Kopete
#include "kopeteuiglobal.h"
#include "urlpicpreviewplugin.h"
#include "urlpicpreviewglobals.h"
#include "kopetechatsessionmanager.h"

typedef KGenericFactory<URLPicPreviewPlugin> URLPicPreviewPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kopete_urlpicpreview, URLPicPreviewPluginFactory("kopete_urlpicpreview"))

URLPicPreviewPlugin::URLPicPreviewPlugin(QObject* parent, const char* name, const QStringList& /* args */)
        : Kopete::Plugin(URLPicPreviewPluginFactory::instance(), parent, name), m_pic(NULL) {

    kdDebug(0) << k_funcinfo << endl;

    Kopete::ChatSessionManager * chatSessionManager = Kopete::ChatSessionManager::self();
    connect(chatSessionManager, SIGNAL(aboutToDisplay(Kopete::Message&)),
            this, SLOT(aboutToDisplay(Kopete::Message&)));

    // register file formats
    KImageIO::registerFormats();

    m_pic = new QImage;
}

URLPicPreviewPlugin::~URLPicPreviewPlugin() {

    kdDebug(0) << k_funcinfo << "Removing temporary files..." << endl;
    for(uint i = 0; i < m_tmpFileRegistry.count(); i++) {
        KIO::NetAccess::removeTempFile(m_tmpFileRegistry[i]);
    }

    disconnect(this, SLOT(aboutToDisplay(Kopete::Message&)));

    delete m_pic;

    kdDebug(0) << k_funcinfo << endl;
}

/*!
    \fn URLPicPreviewPlugin::aboutToDiplay(Kopete::Message& message)
 */
void URLPicPreviewPlugin::aboutToDisplay(Kopete::Message& message) {
    if(message.direction() == Kopete::Message::Inbound) {
        // prepare parsed message body
        message.setBody(prepareBody(message.parsedBody()), Kopete::Message::ParsedHTML);
    }
}

/**
 * @brief Recursively searches the message, downloads and replaces all found imgages
 *
 * @param parsedBody the parsed body of the message
 *
 * @return a new message body with the images as preview
 */
QString URLPicPreviewPlugin::prepareBody(const QString& parsedBody, int previewCount) {

    kdDebug(0) << k_funcinfo << "Searching for URLs to pictures" << endl;

    KConfig * config = kapp->config();
    config->setGroup(CONFIG_GROUP);

    static const QString rex = "(<a href=\")([^\"]*)(\" )?([^<]*)(</a>)(.*)$";
    //             Caps:          1           2        3     4      5    6

    QRegExp ex(rex);
    QString myParsedBody = parsedBody;

    kdDebug(0) << k_funcinfo << "Analyzing message: \"" << myParsedBody << "\"" << endl;

    if(ex.search(myParsedBody) == -1 || (previewCount >= config->readNumEntry("PreviewAmount", 2))) {
        kdDebug(0) << k_funcinfo << "No more URLs found in message." << endl;
        return myParsedBody;
    }

    QString foundURL = ex.cap(2);
    KURL url(foundURL);
    QString tmpFile;

    kdDebug(0) << k_funcinfo << "Found an URL: " << foundURL << endl;

    if(url.isValid()) {
        kdDebug(0) << k_funcinfo << "URL \"" << foundURL << "\" is valid." << endl;

        if(KIO::NetAccess::mimetype(url, Kopete::UI::Global::mainWidget()).startsWith("image/")) {
            if(KIO::NetAccess::download(url, tmpFile, Kopete::UI::Global::mainWidget())) {
                if(config->readBoolEntry("Scaling", true)) {
                    int width = config->readNumEntry("PreviewScaleWidth", 256);
                    kdDebug(0) << k_funcinfo << "Try to scale the image to width: " << width << endl;
                    if(m_pic->load(tmpFile)) {
                        // resize but keep aspect ratio
                        if(m_pic->width() > width) {
                            if(!(m_pic->scaleWidth(width)).save(tmpFile, "PNG")) {
                                kdWarning(0) << k_funcinfo << "Couldn't save scaled image (Format: " << QImage::imageFormat(tmpFile) << ") " << tmpFile << endl;
                            }
                        }
                    } else {
                        kdWarning(0) << k_funcinfo << "Couldn't load image " << tmpFile << endl;
                    }
                }

                myParsedBody.replace(QRegExp(rex), QString("<a href=\"%1\" title=\"%2\">%3</a><br /><img align=\"center\" src=\"%4\" title=\"" + i18n("Preview of:") + " %5\" /><br />").arg(foundURL).arg(foundURL).arg(foundURL).arg(tmpFile).arg(foundURL));

                if(config->readBoolEntry("PreviewRestriction", true)) {
                    previewCount++;
                    kdDebug(0) << k_funcinfo << "Updating previewCount: " << previewCount << endl;
                }

                kdDebug(0) << k_funcinfo << "Registering temporary file for deletion." << endl;
                m_tmpFileRegistry.append(tmpFile);
                return myParsedBody + prepareBody(ex.cap(6), previewCount);
            }
        } else {
            kdWarning(0) << k_funcinfo << foundURL << " is not an image file. Ignoring." << endl;
        }
    } else {
        kdWarning(0) << k_funcinfo << "URL \"" << foundURL << "\" is invalid. Ignoring." << endl;
    }

    return myParsedBody.replace(QRegExp(rex), ex.cap(1) + ex.cap(2) + ex.cap(3) + ex.cap(4) + ex.cap(5)) + prepareBody(ex.cap(6), previewCount);
}

#include "urlpicpreviewplugin.moc"
