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
#ifndef ADDBOOKMARKSPREFERENCES_H
#define ADDBOOKMARKSPREFERENCES_H

#include <kcmodule.h>
#include "addbookmarksprefssettings.h"
#include "addbookmarksprefsui.h"

/**
@author Roie Kerstein <sf_kersteinroie@bezeqint.net>
*/
class BookmarksPreferences : public KCModule
{
Q_OBJECT
public:
    BookmarksPreferences(QWidget *parent = 0, const char *name = 0, const QStringList &args = QStringList());

    ~BookmarksPreferences();
    
    virtual void load();
    virtual void save();
    
signals:
    void PreferencesChanged();
    
private:
    BookmarksPrefsUI *p_dialog;
    BookmarksPrefsSettings m_settings;

private slots:
    void slotSetStatusChanged();
    void slotAddUntrustedChanged();
};

#endif
