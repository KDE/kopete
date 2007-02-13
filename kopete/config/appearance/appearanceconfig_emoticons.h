/*
    appearanceconfig_emoticons.h

    Copyright (c) 2006      by Thorben Kröger         <thorbenk@gmx.net>

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

#ifndef __APPEARANCECONFIG_EMOTICONS_H
#define __APPEARANCECONFIG_EMOTICONS_H

#include "ui_appearanceconfig_emoticons.h"

class AppearanceConfig_Emoticons : public QWidget, public Ui::AppearanceConfig_Emoticons
{
	Q_OBJECT

public:
	AppearanceConfig_Emoticons(QWidget *parent = 0);
};
#endif
