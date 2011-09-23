 /*

    Copyright (c) 2011      by Tobias Koenig  <tokoe at kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */
#ifndef DLGJABBERBOOKMARKEDITOR_H
#define DLGJABBERBOOKMARKEDITOR_H

#include <kdialog.h>
#include "ui_dlgjabberbookmarkeditor.h"

#include "jabberbookmarks.h"

class JabberBookmarkModel;

class DlgJabberBookmarkEditor : public KDialog
{
  Q_OBJECT

  public:
    DlgJabberBookmarkEditor( const JabberBookmark::List &bookmarks, QWidget *parent = 0 );
    ~DlgJabberBookmarkEditor();

    JabberBookmark::List bookmarks() const;

  private Q_SLOTS:
    void renameBookmark();
    void toggleAutoJoin();
    void removeBookmark();

  private:
	  Ui::DlgJabberBookmarkEditor m_ui;
    JabberBookmarkModel *m_model;
};

#endif
