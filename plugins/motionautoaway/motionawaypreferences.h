/*
    motionawaypreferences.h

    Kopete Motion Detector Auto-Away plugin

    Copyright (c) 2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qcheckbox.h>
#include <qspinbox.h>

#include <qwidget.h>
#include <klineedit.h>
#include "configmodule.h"
#include "motionawayprefs.h"

/**
  * @author Duncan Mac-Vicar Prett <duncan@kde.org>
  */

class MotionAwayPreferences : public ConfigModule
{
Q_OBJECT
public:
	MotionAwayPreferences(const QString &pixmap,QObject *parent=0);
	~MotionAwayPreferences();
   virtual void save();

	bool goAvailable() { return preferencesDialog->mGoAvailable->isChecked(); };
	int awayTimeout() { return preferencesDialog->mAwayTimeout->value(); };
	const QString device() { return preferencesDialog->m_videoDevice->text(); };

signals:
	void saved();

private:
	motionawayPrefsUI *preferencesDialog;	

};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

