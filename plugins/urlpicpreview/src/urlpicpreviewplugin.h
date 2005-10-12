/*
    urlpicpreviewplugin.h
 
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

#ifndef URLPICPREVIEWPLUGIN_H
#define URLPICPREVIEWPLUGIN_H

#include <kurl.h>

//Kopete
#include "kopeteplugin.h"

class QImage;

/**
@author Heiko Sch&auml;fer <heiko@rangun.de>
*/
class URLPicPreviewPlugin : public Kopete::Plugin {
    Q_OBJECT

    URLPicPreviewPlugin(const URLPicPreviewPlugin&);
    URLPicPreviewPlugin& operator=(const URLPicPreviewPlugin&);

signals:
    void abortAllOperations();

public:
    URLPicPreviewPlugin(QObject* parent, const char* name, const QStringList& args);
    virtual ~URLPicPreviewPlugin();

private:
    QString prepareBody(const QString& parsedBody, int previewCount = 0);
    QString createPreviewPicture(const KURL& url);

protected slots:
    void aboutToDisplay(Kopete::Message& message);
    void readyForUnload();

private:
    QStringList m_tmpFileRegistry;
    QImage *    m_pic;
    bool        m_abortMessageCheck;
};

#endif /* URLPICPREVIEWPLUGIN_H */
