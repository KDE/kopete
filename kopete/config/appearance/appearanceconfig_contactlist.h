/*
    appearanceconfig_contactlist.h

    Copyright (c) 2006       by Thorben Kröger         <thorbenk@gmx.net>

    Kopete    (c) 2002-2007  by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __APPEARANCECONFIG_CONTACTLIST_H
#define __APPEARANCECONFIG_CONTACTLIST_H

#include "ui_appearanceconfig_contactlist.h"

class AppearanceConfig_ContactList : public QWidget, public Ui::AppearanceConfig_ContactList
{
	Q_OBJECT

public:
	AppearanceConfig_ContactList(QWidget *parent = 0);
};
#endif
