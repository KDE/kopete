/*
    linkpreview.cpp
 
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

//Qt
#include <qpainter.h>

//KDE
#include <kapplication.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kdebug.h>

// Kopete
#include "urlpicpreviewplugin.h"
#include "linkpreview.h"

LinkPreview * LinkPreview::m_self = NULL;

LinkPreview::LinkPreview() : m_pic(1280, 1024), m_khtml(NULL), m_URLLoading(false) {

    kdDebug(0) << k_funcinfo << endl;

    m_khtml = new KHTMLPart();

    m_khtml->setJScriptEnabled(false);
    m_khtml->setJavaEnabled(false);
    m_khtml->setMetaRefreshEnabled(false);
    m_khtml->setPluginsEnabled(false);

    m_khtml->setStatusMessagesEnabled(false);
    m_khtml->setProgressInfoEnabled(true);
}

LinkPreview::~LinkPreview() {
    delete m_khtml;

    kdDebug(0) << k_funcinfo << endl;
}

LinkPreview * LinkPreview::self(const URLPicPreviewPlugin * plugin) {
    if(!m_self) {
        m_self = new LinkPreview();
        connect(plugin, SIGNAL(abortAllOperations()), m_self, SLOT(completed()));
    }

    return m_self;
}

/*!
    \fn LinkPreview::getPreviewPic(const KURL& url)
 */
QPixmap LinkPreview::getPreviewPic(const KURL& url) {
    QPainter painter(&m_pic);

    m_pic.fill();
    m_URLLoading = true;

    connect(m_khtml, SIGNAL(completed()), this, SLOT(completed()));
    connect(m_khtml, SIGNAL(canceled(const QString &)), this, SLOT(completed()));

    if(m_khtml->openURL(url)) {
        kdDebug(0) << k_funcinfo << "Creating preview of " << url << endl;
        m_khtml->view()->resize(1280, 1024);
        m_khtml->stopAnimations();
        m_khtml->paint(&painter, QRect(0, 0, 1280, 1024));

        while(m_URLLoading) {
            kapp->processEvents();
        }
    }

    disconnect(this, SLOT(completed()));

    return m_pic;
}

/*!
    \fn LinkPreview::completed()
 */
void LinkPreview::completed() {
    m_URLLoading = false;
}

#include "linkpreview.moc"
