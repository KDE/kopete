/*
    meanwhileplugin.h - provide helpers for meanwhile from an external plugin

    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef __MEANWHILE_PLUGIN_H__
#define __MEANWHILE_PLUGIN_H__

#include <kglobal.h>

class QLineEdit;
class QWidget;
class QString;
class KActionMenu;

class MeanwhilePlugin
{
public:
    virtual ~MeanwhilePlugin() {}
    /* do something when the find button on add contact is hit
     *  - like do lookups in company databases
     *   when done fill 'fillThis' with the id of the contact
     */
    virtual void getMeanwhileId(QWidget *parent, QLineEdit *fillThis);
    /* can this plugin provide the above functionality */
    virtual bool canProvideMeanwhileId();

    /* show user info has been selected - the meanwhile server doesn't
     *  seem to have any info...maybe you can find it somewhere else
     */
    virtual void showUserInfo(const QString &userid);

    /* if you want to provide more functions on the rightclick dropdown
     *   menu...implement this
     */
    virtual void addCustomMenus(KActionMenu *menu);
};

#endif
