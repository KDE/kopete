/*
    behaviourconfig_general.h

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

#ifndef BEHAVIORCONFIG_GENERAL_H
#define BEHAVIORCONFIG_GENERAL_H

#include "ui_behaviorconfig_general.h"

class BehaviorConfig_General : public QWidget, public Ui::BehaviorConfig_General
{
	Q_OBJECT

public:
	BehaviorConfig_General(QWidget *parent = 0);

private slots:
	void queueToggled(bool checked);
};

#endif // BEHAVIORCONFIG_GENERAL_H
