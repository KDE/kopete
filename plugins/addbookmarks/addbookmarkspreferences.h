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

class QButtonGroup;
class QStringListModel;

namespace Ui { class BookmarksPrefsUI; }

/**
@author Roie Kerstein <sf_kersteinroie@bezeqint.net>
*/
class BookmarksPreferences : public KCModule
{
Q_OBJECT
public:
    explicit BookmarksPreferences(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

    ~BookmarksPreferences();
    
    void load() Q_DECL_OVERRIDE;
    void save() Q_DECL_OVERRIDE;
    
signals:
    void PreferencesChanged();
    
private:
    Ui::BookmarksPrefsUI *p_dialog;
    QButtonGroup* p_buttonGroup;
    QStringListModel *p_contactsListModel;
    BookmarksPrefsSettings m_settings;

private slots:
    void slotSetStatusChanged();
           
};

#endif
