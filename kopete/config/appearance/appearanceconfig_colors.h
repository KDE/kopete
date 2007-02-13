/*
    appearanceconfig_colors.h

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

#ifndef __APPEARANCECONFIG_COLORS_H
#define __APPEARANCECONFIG_COLORS_H

#include "ui_appearanceconfig_colors.h"

class AppearanceConfig_Colors : public QWidget, public Ui::AppearanceConfig_Colors
{
	Q_OBJECT

public:
	AppearanceConfig_Colors(QWidget *parent = 0);
};
#endif
