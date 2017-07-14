//
// C++ Interface: %{MODULE}
//
// Description:
//
//
// Author: Roie Kerstein <sf_kersteinroie@bezeqint.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ADDBOOKMARKSPLUGIN_H
#define ADDBOOKMARKSPLUGIN_H

#include "addbookmarksprefssettings.h"

#include <kopeteplugin.h>
#include <kopetemessage.h>

#include <kpluginfactory.h>
#include <kbookmarkmanager.h>
#include <kio/job.h>
#include <kurl.h>

#include <qmap.h>

/**
@author Roie Kerstein <sf_kersteinroie@bezeqint.net>
*/

class BookmarksPlugin : public Kopete::Plugin
{
    Q_OBJECT
public:
    BookmarksPlugin(QObject *parent, const QVariantList &args);

    ~BookmarksPlugin();

private:
    typedef struct S_URLANDNAME {
        QUrl url;
        QString sender;
    } URLandName;
    typedef QMap<KIO::TransferJob *, URLandName> JobsToURLsMap;

    JobsToURLsMap m_map;
    BookmarksPrefsSettings m_settings;

    void addKopeteBookmark(const QUrl &url, const QString &sender);
    QList<QUrl> *extractURLsFromString(const QString &text);
    KBookmarkGroup getKopeteFolder();
    KBookmarkGroup getFolder(KBookmarkGroup group, QString folder);
    bool isURLInGroup(const QUrl &url, KBookmarkGroup group);
    QTextCodec *getPageEncoding(const QByteArray &data);
public Q_SLOTS:
    void slotBookmarkURLsInMessage(Kopete::Message &msg);
    void slotReloadSettings();

private Q_SLOTS:
    void slotAddKopeteBookmark(KIO::Job *transfer, const QByteArray &data);
};

#endif
