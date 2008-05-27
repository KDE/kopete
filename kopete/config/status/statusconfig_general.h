/*
    statusconfig_general.h

    Copyright (c) 2008       by Roman Jarosz           <kedgedev@centrum.cz>
    Kopete    (c) 2008       by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef STATUSCONFIG_GENERAL_H
#define STATUSCONFIG_GENERAL_H

#include "ui_statusconfig_general.h"

class StatusConfig_General : public QWidget, public Ui::StatusConfig_General
{
	Q_OBJECT

public:
	StatusConfig_General(QWidget *parent = 0);
};

#endif // STATUSCONFIG_GENERAL_H
