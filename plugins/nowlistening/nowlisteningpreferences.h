/*
    nowlisteningpreferences.h

    Kopete Now Listening To plugin

    Copyright (c) 2002,2003,2004 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef NOWLISTENINGPREFERENCES_H
#define NOWLISTENINGPREFERENCES_H

#include <kcmodule.h>

#include <QVariantList>

namespace Ui { class NowListeningPrefsUI; }

/**
  *@author Will Stephenson
  */

class NowListeningPreferences : public KCModule
{
Q_OBJECT
public:
	explicit NowListeningPreferences(QWidget *parent = 0, const QVariantList &args = QVariantList());
	virtual ~NowListeningPreferences();
	virtual void save();
	virtual void load();
	virtual void defaults();

private slots:
	void slotSettingsChanged();

private:
	Ui::NowListeningPrefsUI *preferencesDialog;

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

