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
#include <kio/job.h>
#include <qcstring.h>
#include <qmap.h>

/**
@author Roie Kerstein <sf_kersteinroie@bezeqint.net>
*/

class AddBookmarksPlugin : public KopetePlugin
{
Q_OBJECT
public:
    AddBookmarksPlugin(QObject *parent, const char *name, const QStringList &args);

    ~AddBookmarksPlugin();
    
private:
    typedef struct S_URLANDNAME{
    	KURL url;
	QString sender;
    } URLandName;
    typedef QMap<KIO::TransferJob*,URLandName> JobsToURLsMap;
    
    JobsToURLsMap m_map;
    AddBookmarksPrefsSettings m_settings;
    
    void AddKopeteBookmark(KURL url, QString sender );
    KURL::List* ExtractURLsFromString(QString text);
    KBookmarkGroup GetKopeteFolder();
    KBookmarkGroup GetFolder( KBookmarkGroup group, QString folder );
    bool isURLInGroup(KURL url, KBookmarkGroup group);
    QTextCodec* GetPageEncoding( QByteArray data );
public slots:
    void slotBookmarkURLsInMessage(KopeteMessage & msg);
    void slotReloadSettings();
    
private slots:
    void slotAddKopeteBookmark( KIO::Job *transfer, const QByteArray &data );
};

typedef KGenericFactory<AddBookmarksPlugin> AddBookmarksPluginFactory;

#endif
