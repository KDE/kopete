/***************************************************************************
                      dlgjabbersendraw.h  -  Raw XML dialog
                             -------------------
    begin                : Sun Aug 25 2002
    copyright            : (C) 2002 by Till Gerken,
                           Kopete Development team
    email                : till@tantalo.net
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGJABBERSENDRAW_H
#define DLGJABBERSENDRAW_H

#include <qwidget.h>
#include "dlgsendraw.h"

class JabberProtocol;

class dlgJabberSendRaw : public dlgSendRaw
{
	Q_OBJECT

	public:
		dlgJabberSendRaw(JabberProtocol *owner, QWidget *parent = 0, const char *name = 0);
		~dlgJabberSendRaw();

	public slots:
		void slotFinish();
		void slotCancel();

	private:
		JabberProtocol *plugin;

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
