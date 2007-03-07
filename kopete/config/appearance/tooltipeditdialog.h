/*
    tooltipeditdialog.cpp  -  Kopete Tooltip Editor

    Copyright (c) 2004 by Stefan Gehn <metz@gehn.net>

    Kopete    (c) 2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef TOOLTIPEDITDIALOG_H
#define TOOLTIPEDITDIALOG_H

#include <kdebug.h>
#include <kdialog.h>

#include "ui_tooltipeditwidget.h"

class QStandardItemModel;

class TooltipEditDialog : public KDialog, private Ui::TooltipEditWidget
{
	Q_OBJECT

	public:
		TooltipEditDialog(QWidget *parent=0);

	private slots:
		void slotUnusedSelected(const QItemSelection&);
		void slotUsedSelected(const QItemSelection&);
		void slotUpButton();
		void slotDownButton();
		void slotAddButton();
		void slotRemoveButton();
		void slotOkClicked();

	signals:
		void changed(bool);

	private:
		QWidget *mMainWidget;

		QStandardItemModel *mUnusedEntries;
		QStandardItemModel *mUsedEntries;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
