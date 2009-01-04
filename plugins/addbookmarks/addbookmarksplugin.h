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
#include <kgenericfactory.h>
#include <kopeteplugin.h>
#include <kbookmarkmanager.h>
#include <kopetemessage.h>
#include <kio/job.h>
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
    typedef struct S_URLANDNAME{
		KUrl url;
		QString sender;
    } URLandName;
    typedef QMap<KIO::TransferJob*,URLandName> JobsToURLsMap;
    
    JobsToURLsMap m_map;
    BookmarksPrefsSettings m_settings;
    
    void addKopeteBookmark( const KUrl &url, const QString &sender );
    KUrl::List* extractURLsFromString( const QString &text );
    KBookmarkGroup getKopeteFolder();
    KBookmarkGroup getFolder( KBookmarkGroup group, QString folder );
    bool isURLInGroup( const KUrl &url, KBookmarkGroup group );
    QTextCodec* getPageEncoding( const QByteArray &data );
public slots:
    void slotBookmarkURLsInMessage(Kopete::Message & msg);
    void slotReloadSettings();
    
private slots:
    void slotAddKopeteBookmark( KIO::Job *transfer, const QByteArray &data );
};

#endif
