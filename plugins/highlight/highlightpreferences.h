/*
    highlightpreferences.h  -  description

    Copyright (c) 2003      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef HighlightPREFERENCES_H
#define HighlightPREFERENCES_H

#include <kcmodule.h>
#include <qstring.h>
#include "highlightconfig.h"
#include "ui_highlightprefsbase.h"

namespace Ui { class HighlightPrefsUI; }
class Filter;

/**
  *@author Olivier Goffart
  */

class HighlightPreferences : public KCModule  {
   Q_OBJECT
public:

	explicit HighlightPreferences(QWidget *parent = 0, const QVariantList &args = QVariantList());
	~HighlightPreferences();

	virtual void save();
	virtual void load();

private:
	Ui::HighlightPrefsUI preferencesDialog;
	HighlightConfig *m_config;
	
	Filter *selectedItem();

	bool donttouch;

private slots:
	void slotCurrentFilterChanged();
	void slotAddFilter();
	void slotRemoveFilter();
	void slotRenameFilter();
	void slotSomethingHasChanged();
	void slotEditRegExp();
	void slotConfigureNotifications();
};

#endif
