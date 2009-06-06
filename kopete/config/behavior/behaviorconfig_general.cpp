/*
    behaviourconfig_general.cpp

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

#include "behaviorconfig_general.h"

BehaviorConfig_General::BehaviorConfig_General(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	connect(kcfg_useMessageQueue, SIGNAL(toggled(bool)), this, SLOT(queueToggled(bool)));
}

void BehaviorConfig_General::queueToggled(bool checked)
{
	kcfg_queueUnreadMessages->setEnabled(checked);
}

#include "behaviorconfig_general.moc"
