/*
    preferencesdialog.h  -  Kopete Setup Dialog

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <kdialogbase.h>
#include <qptrlist.h>

class ConfigModule;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class PreferencesDialog : public KDialogBase
{
	Q_OBJECT

public:
	static PreferencesDialog* preferencesDialog();

	~PreferencesDialog();

	virtual void show();

	void add( ConfigModule *page );
	void remove( ConfigModule *page );

public slots:
	virtual void slotApply();
	virtual void slotOk();

private:
	PreferencesDialog();

	QPtrList<ConfigModule> mModules;

	static PreferencesDialog *s_preferencesDialog;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

