/*
    xtrazstatusaction.h  -  Xtraz Status Action

    Copyright (c) 2007 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef XTRAZSTATUSACTION_H
#define XTRAZSTATUSACTION_H

#include <QAction>

#include "xtrazstatus.h"

namespace Xtraz
{

class StatusAction: public QAction
{
	Q_OBJECT
public:
	StatusAction( const Xtraz::Status &status, QObject *parent );

public Q_SLOTS:
	void triggered();

Q_SIGNALS:
	void triggered( const Xtraz::Status &status );

private:
	Xtraz::Status mStatus;
};

}

#endif
