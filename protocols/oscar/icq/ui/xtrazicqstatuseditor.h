/*
    xtrazicqstatuseditor.h  -  Xtraz ICQ Status Editor

    Copyright (c) 2007 by Roman Jarosz <kedgedev@centrum.cz>
    Kopete    (c) 2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef XTRAZICQSTATUSEDITOR_H
#define XTRAZICQSTATUSEDITOR_H

#include <kdialog.h>

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/

namespace Ui { class XtrazICQStatusEditorUI; }

class ICQStatusManager;

namespace Xtraz
{

class StatusModel;

class ICQStatusEditor : public KDialog
{
	Q_OBJECT

public:
	explicit ICQStatusEditor( ICQStatusManager *statusManager, QWidget *parent = 0 );
	~ICQStatusEditor();

public slots:
	void save();

	void moveUp();
	void moveDown();

	void addStatus();
	void deleteStatus();

private slots:
	void updateButtons();

private:
	Ui::XtrazICQStatusEditorUI *mUi;
	Xtraz::StatusModel* mXtrazStatusModel;
	ICQStatusManager *mStatusManager;

};

}

#endif
