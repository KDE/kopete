/*
    urlpicpreviewpreferences.h
 
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

#ifndef URLPICPREVIEWPREFERENCES_H
#define URLPICPREVIEWPREFERENCES_H

#include <kcmodule.h>

class QLayout;
class KConfig;
class URLPicPreviewPrefsBase;

class URLPicPreviewPreferences : public KCModule {
    Q_OBJECT

    URLPicPreviewPreferences(const URLPicPreviewPreferences&);
    URLPicPreviewPreferences& operator=(const URLPicPreviewPreferences&);

public:
    URLPicPreviewPreferences(QWidget* parent, const char* name, const QStringList& args);

    virtual ~URLPicPreviewPreferences();
    virtual void load();
    virtual void save();

private:
    QLayout * m_layout;
    URLPicPreviewPrefsBase * m_ui;
    KConfig * m_config;
};

#endif /* URLPICPREVIEWPREFERENCES_H */

