/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AliasPREFERENCES_H
#define AliasPREFERENCES_H

#include "kcautoconfigmodule.h"

class AliasDialogBase;

class AliasPreferences : public KCAutoConfigModule
{
	Q_OBJECT

public:
	AliasPreferences( QWidget *parent = 0, const char *name = 0, const QStringList &args = QStringList() );
	~AliasPreferences();

	virtual void save();
	virtual void load();

private slots:
	void slotAddAlias();
	void slotEditAlias();
	void slotDeleteAliases();
	void slotCheckAliasSelected();
	
private:
	AliasDialogBase * preferencesDialog;
	void addAlias( QString &alias, QString &command );

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

