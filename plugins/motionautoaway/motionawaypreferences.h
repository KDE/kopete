/*
    Kopete Motion Detector Auto-Away plugin

    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    
    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MOTIONAWAYPREFERENCES_H
#define MOTIONAWAYPREFERENCES_H

#include "kcmodule.h"

class Ui::motionawayPrefsUI;

/**
 * Preference widget for the Motion Away plugin
 * @author Duncan Mac-Vicar P.
 */
class MotionAwayPreferences : public KCModule
{
	Q_OBJECT
public:
	explicit MotionAwayPreferences ( QWidget* parent = 0, const QVariantList& args = QVariantList() );
	virtual void save();
	virtual void load();
	virtual void defaults();

private:
	Ui::motionawayPrefsUI *preferencesDialog;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
