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
class AddBookmarksPreferences : public KCModule
{
Q_OBJECT
public:
    AddBookmarksPreferences(QWidget *parent = 0, const char *name = 0, const QStringList &args = QStringList());

    ~AddBookmarksPreferences();
    
    virtual void load();
    virtual void save();
    
signals:
    void PreferencesChanged();
    
private:
    AddBookmarksPrefsUI *p_dialog;
    AddBookmarksPrefsSettings m_settings;

private slots:
    void slotSetStatusChanged();
           
};

#endif
