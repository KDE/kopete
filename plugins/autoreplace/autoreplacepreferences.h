/***************************************************************************
                          autoreplacepreferences.h  -  description
                             -------------------
    begin                : 20030426
    copyright            : (C) 2003 by Roberto Pariset
    email                : victorheremita@fastwebnet.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AutoReplacePREFERENCES_H
#define AutoReplacePREFERENCES_H

#include "configmodule.h"

class AutoReplacePrefsUI;

	// TODO
	// add button enabled only when k and v are present 
	// remove button enabled only when a QListViewItem is selected 
	// signal/slot when map changes (needed?)
	// dont autoreplace outgoing messages checkbox

class AutoReplacePreferences : public ConfigModule  {
   Q_OBJECT
public:

	AutoReplacePreferences( const QString & pixmap, QObject * parent = 0 );
	~AutoReplacePreferences();

	virtual void save();
	virtual void reopen();

	typedef QMap<QString, QString> WordsToReplace;
	
	WordsToReplace getMap() const { return map; };	// O(1) implicity shared
	bool getAutoreplaceIncoming() const { return autoreplaceIncoming; };
	bool getAddDot() const { return addDot; };
	bool getUpper() const { return upper; };

private:
	AutoReplacePrefsUI * preferencesDialog;
	WordsToReplace map;
	QStringList wordsList;

	bool autoreplaceIncoming;
	bool autoreplaceOutgoing;
	bool addDot;
	bool upper;

private slots:
	void slotAddCouple();
	void slotRemoveCouple();
	/* void slotEnableAdd();
	void slotEnableRemove(); */
};

#endif
