/***************************************************************************
                          wpuserinfo.h  -  WinPopup User Info
                             -------------------
    begin                : Tue May 06 2003
    copyright            : (C) 2003 by Tais M. Hansen
    email                : tais.hansen@osd.dk

    Based on code from   : (C) 2002-2003 by the Kopete developers
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

#ifndef WPUSERINFO_H
#define WPUSERINFO_H

// KDE Includes
#include <kdialogbase.h>
#include <kprocio.h>

// Local Includes
#include "wpuserinfowidget.h"

class WPAccount;
class WPContact;

class WPUserInfo : public KDialogBase
{
	Q_OBJECT

	public:
		WPUserInfo( WPContact *, WPAccount *, QWidget *parent = 0, const char* name = "WPUserInfo" );

		void startDetailsProcess(const QString &host);

	private slots:
		void slotDetailsProcessReady(KProcIO *d);
		void slotDetailsProcessExited(KProcess *d);
		void slotCloseClicked();

	signals:
		void closing();

	private:
		WPContact *m_contact;
		WPUserInfoWidget *m_mainWidget;

		QString Comment, Workgroup, OS, Software;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
