/***************************************************************************
                          msnpreferences.h  -  description
                             -------------------
    begin                : Wed Jan 23 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MSNPREFERENCES_H
#define MSNPREFERENCES_H

#include <qwidget.h>

#include <klineedit.h>

#include "configmodule.h"
#include "ircprefs.h"

/**
  *@author duncan
  */

class IRCPreferences : public ConfigModule
{
Q_OBJECT
public:
	IRCPreferences(const QString &pixmap,QObject *parent=0);
	~IRCPreferences();
   virtual void save();
signals:
	void saved();

private:
	ircPrefsUI *preferencesDialog;	
private slots:
	void slotHighlightNick();
	void slotHighlightOthers();
	void slotUseMDI();

};

#endif
