/***************************************************************************
                          ircpreferences.h  -  description
                             -------------------
    begin                : Wed Jan 23 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IRCPREFERENCES_H
#define IRCPREFERENCES_H

#include "configmodule.h"

class ircPrefsUI;

/**
  *@author nbetcher
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
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
