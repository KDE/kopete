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

#include "kcautoconfigmodule.h"

class AutoReplacePrefsUI;
class AutoReplaceConfig;

	// TODO
	// add button enabled only when k and v are present
	// remove button enabled only when a QListViewItem is selected
	// signal/slot when map changes (needed?)
	// capital letter not just at the beginning but always after ". ", "! "...

class AutoReplacePreferences : public KCAutoConfigModule
{
	Q_OBJECT

public:
	AutoReplacePreferences( QWidget *parent = 0, const char *name = 0, const QStringList &args = QStringList() );
	~AutoReplacePreferences();

	virtual void save();
	virtual void load();
    virtual void defaults();

private slots:
	//void slotSettingsDirty();
	void slotAddCouple();
	void slotEditCouple();
	void slotRemoveCouple();
	void slotEnableAddEdit( const QString & );
	void slotSelectionChanged();

protected slots:
	virtual void slotWidgetModified();
private:
	AutoReplacePrefsUI * preferencesDialog;
	AutoReplaceConfig *m_config;

	bool m_wordListChanged;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

