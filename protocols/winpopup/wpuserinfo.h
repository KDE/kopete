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

#include <QProcess>

#include <KDialog>

class WPContact;
namespace Ui { class WPUserInfoWidget; }

class WPUserInfo : public KDialog
{
	Q_OBJECT

	public:
		explicit WPUserInfo( WPContact *, QWidget *parent = 0 );
		~WPUserInfo();

		void startDetailsProcess(const QString &host);

	private slots:
		void slotDetailsProcess(int i = 1, QProcess::ExitStatus status = QProcess::CrashExit);
		void slotDetailsProcessFinished(int, QProcess::ExitStatus);
		void slotCloseClicked();

	signals:
		void closing();

	private:
		WPContact *m_contact;
		Ui::WPUserInfoWidget *m_mainWidget;

		QString Comment, Workgroup, OS, Software;
		QProcess *detailsProcess;
		bool noComment;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
