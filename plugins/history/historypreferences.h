/*
    historypreferences.h

    Copyright (c) 2003      by Olivier Goffart <ogoffart@kde.org>
    Copyright (c) 2003      by Stefan Gehn <metz@gehn.net>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef HISTORYPREFERENCES_H
#define HISTORYPREFERENCES_H

#include <kcmodule.h>

namespace Ui { class HistoryPrefsUI; }

/**
 * @author Stefan Gehn
 */
class HistoryPreferences : public KCModule
{
	Q_OBJECT
	public:
		explicit HistoryPreferences(QWidget *parent=0, const QVariantList &args = QVariantList());
		~HistoryPreferences();

		virtual void save();
		virtual void load();

	private slots:
		void slotModified();
		void slotShowPreviousChanged(bool);

	private:
		Ui::HistoryPrefsUI *p;
};

#endif
