/*
    behaviorconfig.h  -  Kopete Look Feel Config

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __BEHAVIOR_H
#define __BEHAVIOR_H

#include "configmodule.h"

class QFrame;
class QTabWidget;
class QCheckBox;

class KopeteAwayConfigUI;

class BehaviorConfig : public ConfigModule
{
	Q_OBJECT

public:
	BehaviorConfig(QWidget * parent);

	virtual void save();
	virtual void reopen();

private:
	QTabWidget* mBehaviorTabCtl;
	KopeteAwayConfigUI *mAwayConfigUI;

};
#endif
// vim: set noet ts=4 sts=4 sw=4:
