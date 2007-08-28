/*
    webpresencepreferences.h

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef WEBPRESENCEPREFERENCES_H
#define WEBPRESENCEPREFERENCES_H

#define KDE3_SUPPORT
#include <kcmodule.h>
#undef KDE3_SUPPORT

namespace Ui { class WebPresencePrefsUI; }
//class KAutoConfig;

//TODO: Port to KConfigXT
/**
 * Preference widget for the Now Listening plugin, copied from the Cryptography plugin
 * @author Olivier Goffart
 */
class WebPresencePreferences : public KCModule  {
   Q_OBJECT

public:
	explicit WebPresencePreferences(QWidget *parent = 0, const QStringList &args = QStringList());
	~WebPresencePreferences();

	virtual void save();
	virtual void defaults();

private:
	Ui::WebPresencePrefsUI *preferencesDialog;
	//KAutoConfig *kautoconfig;

private slots: // Public slots
	void widgetModified();

};

#endif // WEBPRESENCEPREFERENCES_H

// vim: set noet ts=4 sts=4 sw=4:
