/***************************************************************************
                          highlightpreferences.h  -  description
                             -------------------
    begin                : mar 14 2003
    copyright            : (C) 2003 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HighlightPREFERENCES_H
#define HighlightPREFERENCES_H

#include "configmodule.h"

class HighlightPrefsUI;
class Filter;
class QListViewItem;

/**
  *@author Olivier Goffart
  */

class HighlightPreferences : public ConfigModule  {
   Q_OBJECT
public:

	HighlightPreferences(const QString &pixmap, QObject *parent=0);
	~HighlightPreferences();

	virtual void save();
	virtual void reopen();

private:
	HighlightPrefsUI *preferencesDialog;
	QMap <QListViewItem*,Filter*> m_filterItems;
	
	bool donttouch;
	
private slots: 
	void slotCurrentFilterChanged();
	void slotAddFilter();
	void slotRemoveFilter();
	void slotRenameFilter();
	void slotSomethingHasChanged();
	void slotEditRegExp(); 
};

#endif
