/***************************************************************************
                          preferencesdialog.h  -  description
                             -------------------
    begin                : Wed Dec 26 2001
    copyright            : (C) 2001 by Duncan Mac-Vicar Prett
    email                : duncan@puc.cl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <kdialogbase.h>
#include <qlist.h>

class ConfigModule;

/**
  *@author Duncan Mac-Vicar Prett
  */
class PreferencesDialog : public KDialogBase
{
	friend class ConfigModule;

	Q_OBJECT
	
	public:
		PreferencesDialog(QWidget *parent=0);
		~PreferencesDialog();
	
	public:
		virtual void show();
	
	public slots:
		virtual void slotApply();
		virtual void slotOk();

	private:
		void add(ConfigModule *page);
		void remove(ConfigModule *page);

	private:
		QList<ConfigModule> mModules;

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

