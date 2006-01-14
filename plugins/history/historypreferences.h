/*
    historypreferences.h

    Copyright (c) 2003 by Olivier Goffart             <ogoffart @ kde.org>
              (c) 2003 by Stefan Gehn                 <metz AT gehn.net>
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
#include <qstring.h>

class HistoryPrefsUI;

/**
 * @author Stefan Gehn
 */
class HistoryPreferences : public KCModule
{
	Q_OBJECT
	public:
		HistoryPreferences(QWidget *parent=0, const char* name=0,
			const QStringList &args = QStringList());
		~HistoryPreferences();

		virtual void save();
		virtual void load();

	private slots:
		void slotModified();
		void slotShowPreviousChanged(bool);

	private:
		HistoryPrefsUI *p;
};

#endif
