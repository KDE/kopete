/*
    linkpreview.h
 
    Copyright (c) 2005      by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>
 
    ************************************************************************ *
    *                                                                        *
    * This program is free software; you can redistribute it and/or modify   *
    * it under the terms of the GNU General Public License as published by   *
    * the Free Software Foundation; version 2, or (at your option) version 3 *
    * of the License.                                                        *
    *                                                                        *
    **************************************************************************
*/

#ifndef LINKPREVIEW_H
#define LINKPREVIEW_H

// Qt
#include <qpixmap.h>

// KDE
#include <kurl.h>

class KHTMLPart;
class URLPicPreviewPlugin;

class LinkPreview : public QObject {
    Q_OBJECT

    LinkPreview(const LinkPreview&);
    LinkPreview& operator=(const LinkPreview&);

public:
    virtual ~LinkPreview();
    static LinkPreview * self(const URLPicPreviewPlugin * plugin);
    QPixmap getPreviewPic(const KUrl& url);

private:
    LinkPreview();

protected slots:
    void completed();

private:
    QPixmap m_pic;
    KHTMLPart * m_khtml;
    static LinkPreview * m_self;
    bool m_URLLoading;
};

#endif /* LINKPREVIEW_H */
