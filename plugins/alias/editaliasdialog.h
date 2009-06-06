/*
 Kopete Alias Plugin

 Copyright (c) 2005 by Matt Rogers <mattr@kde.org>
 Kopete Copyright (c) 2002-2005 by the Kopete Developers <kopete-devel@kde.org>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************

*/

#ifndef _EDITALIASDIALOG_H_
#define _EDITALIASDIALOG_H_

#include "ui_aliasdialog.h"

#include <QDialog>

class QWidget;

class EditAliasDialog : public QDialog, public Ui::AliasDialog
{
	Q_OBJECT
public:
	EditAliasDialog( QWidget* parent = 0 );
	virtual ~EditAliasDialog();
	
public slots:
	void checkButtonsEnabled();
};

#endif

// kate: space-indent off; replace-tabs off; tab-width 4; indent-mode csands;
