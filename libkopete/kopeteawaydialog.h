/*
    kopeteawaydialog.h  -  Kopete Away Dialog

	Copyright (c) 2002 by Hendrik vom Lehn <hvl@linux-4-ever.de>

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

#ifndef KOPETEAWAYDIALOG_HI
#define KOPETEAWAYDIALOG_HI

class QString;
class KopeteAway;
class KConfig;

#include "kopeteawaydialogbase.h"
   
/**
 * @author Hendrik vom Lehn <hvl@linux-4-ever.de>
 *
 */

class KopeteAwayDialog : public KopeteAwayBase
{
	Q_OBJECT

	public:
	KopeteAwayDialog();
	
	private:
	KopeteAway *awayInstance;
	KConfig *config;
	
	private slots:
		void slotOkayClicked();
		void slotCancelClicked();
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

