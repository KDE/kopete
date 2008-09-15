/*
    behaviourconfig_events.cpp

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

#include "behaviorconfig_events.h"

#include <QMovie>

BehaviorConfig_Events::BehaviorConfig_Events(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	if ( !QMovie::supportedFormats().contains("mng") )
	{
		kcfg_trayflashNotify->setEnabled(false);
		kcfg_trayflashNotify->setToolTip(i18n("Animation is not possible as your Qt version does not support the mng video format."));
	}
}

#include "behaviorconfig_events.moc"
