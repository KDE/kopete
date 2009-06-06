/*
    behaviourconfig_away.h

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

#ifndef BEHAVIORCONFIG_AWAY_H
#define BEHAVIORCONFIG_AWAY_H

#include "ui_behaviorconfig_away.h"

class BehaviorConfig_Away : public QWidget, public Ui::BehaviorConfig_Away
{
	Q_OBJECT

public:
	BehaviorConfig_Away(QWidget *parent = 0);
};

#endif // BEHAVIORCONFIG_AWAY_H
