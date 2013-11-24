/*
    meanwhileplugin.cpp - a plugin to provide more functions

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
#include "meanwhileplugin.h"
#include <qwidget.h>
#include <qlineedit.h>

void MeanwhilePlugin::getMeanwhileId(QWidget * /*parent*/,
            QLineEdit * /*fillThis*/)
{
}

bool MeanwhilePlugin::canProvideMeanwhileId()
{
    return false;
}

void MeanwhilePlugin::showUserInfo(const QString &/*userid*/)
{
}

void MeanwhilePlugin::addCustomMenus(KActionMenu *menu)
{
}
