/*
    tooltipeditdialog.cpp  -  Kopete Tooltip Editor

    Copyright (c) 2004 by Stefan Gehn <metz AT gehn.net>
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
#include <qhbox.h>
#include <kdialogbase.h>
class TooltipEditWidget;

class TooltipEditDialog : public KDialogBase
{
	Q_OBJECT

	public:
		TooltipEditDialog(QWidget *parent=0, const char* name="ToolTipEditDialog");

	private slots:
		void slotUnusedSelected(QListViewItem *);
		void slotUsedSelected(QListViewItem *);
		void slotUpButton();
		void slotDownButton();
		void slotAddButton();
		void slotRemoveButton();
		void slotOkClicked();

	signals:
		void changed(bool);

	private:
		TooltipEditWidget *mMainWidget;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
