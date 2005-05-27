
/***************************************************************************
                      dlgjabbersendraw.h  -  Raw XML dialog
                             -------------------
    begin                : Sun Aug 25 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
    email                : kopete-devel@kde.org
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

class JabberClient;

/**
 * A dialog to send raw strings to the jabber server.
 *
 * It comes with a QComboBox to choose some "template" strings
 * like "Availability Status", "Subscription",...
 *
 * @author Till Gerken <till@tantalo.net>
 * @author Chris TenHarmsel <tenharmsel@users.sf.net>
 */
class dlgJabberSendRaw:public DlgSendRaw
{
Q_OBJECT

public:
	dlgJabberSendRaw ( JabberClient *client, QWidget * parent = 0, const char *name = 0);
	virtual ~ dlgJabberSendRaw ();

public slots:

	/**
	 * Closes the SendRaw Dialog.
	 */
	void slotCancel ();

	/**
	 * Clears current xml message in tePacket.
	 */
	void slotClear ();

	/**
	 * Sets a xml message in tePacket(QTextWidget)
	 * according to the state of inputWidget.
	 */
	void slotCreateMessage (int);

	/**
	 * Sends a xml message to the server,
	 * clears tePacket afterwards.
	 */
	void slotSend();

private:
	/**
	 * This is what we talk through
	 */
	JabberClient *m_client;
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
