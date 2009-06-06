/*
    xtrazicqstatusdialog.h  -  Xtraz ICQ Status Dialog

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

#ifndef XTRAZICQSTATUSDIALOG_H
#define XTRAZICQSTATUSDIALOG_H

#include <kdialog.h>

#include "xtrazstatus.h"

/**
	@author Roman Jarosz <kedgedev@centrum.cz>
*/

namespace Ui { class XtrazICQStatusUI; }

namespace Xtraz
{

class ICQStatusDialog : public KDialog
{
public:
	ICQStatusDialog( QWidget *parent = 0 );
	~ICQStatusDialog();

	void setXtrazStatus( Xtraz::Status status );

	Xtraz::Status xtrazStatus() const;

	bool append() const;

private:
	Ui::XtrazICQStatusUI *mXtrazStatusUI;
};

}

#endif
