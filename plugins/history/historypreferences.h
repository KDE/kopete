/*
    historypreferences.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include "configmodule.h"
#include <qcolor.h>

class HistoryPrefsUI;

/**
  *@author Olivier Goffart
  */

class HistoryPreferences : public ConfigModule  {
   Q_OBJECT
public:

	HistoryPreferences( QObject *parent=0);
	~HistoryPreferences();

	virtual void save();
	virtual void reopen();

	int nbChatwindow() const;
	int nbAutoChatwindow() const;
	QColor historyColor() const;
private:
	HistoryPrefsUI *m_widget;
};

#endif
