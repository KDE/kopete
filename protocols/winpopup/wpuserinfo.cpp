/***************************************************************************
                          wpuserinfo.cpp  -  WinPopup User Info
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

// QT Includes
#include <qregexp.h>

// KDE Includes
#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>
#include <ksimpleconfig.h>

// Local Includes
#include "wpuserinfo.h"
#include "wpaccount.h"
#include "wpcontact.h"

WPUserInfo::WPUserInfo( WPContact *contact, WPAccount */*account*/, QWidget *parent, const char* name )
	: KDialogBase( parent, name, false, QString::null, Close, Close, false ), m_contact(contact),
	  Comment(i18n("N/A")), Workgroup(i18n("N/A")), OS(i18n("N/A")), Software(i18n("N/A"))
{
//	kdDebug( 14170 ) << k_funcinfo << endl;

	setCaption( i18n( "User Info for %1" ).arg( m_contact->nickName() ) );

	m_mainWidget = new WPUserInfoWidget( this, "WPUserInfo::m_mainWidget" );
	setMainWidget( m_mainWidget );

	m_mainWidget->sComputerName->setText( m_contact->contactId() );

	m_mainWidget->sComment->setText(i18n("Looking"));
	m_mainWidget->sWorkgroup->setText(i18n("Looking"));
	m_mainWidget->sOS->setText(i18n("Looking"));
	m_mainWidget->sServer->setText(i18n("Looking"));

	connect( this, SIGNAL( closeClicked() ), this, SLOT( slotCloseClicked() ) );

	startDetailsProcess(m_contact->contactId());
}

// I decided to do this direct here to avoid "Handstände" with signals and stuff
// if we would do this in libwinpopup. GF
void WPUserInfo::startDetailsProcess(const QString &host)
{
	KGlobal::config()->setGroup("WinPopup");
	QString theSMBClientPath = KGlobal::config()->readEntry("SMBClientPath", "/usr/bin/smbclient");

	KProcIO *details = new KProcIO;
	*details << theSMBClientPath << "-N" << "-E" << "-g" << "-L" << host << "-";

	connect(details, SIGNAL(readReady(KProcIO *)), this, SLOT(slotDetailsProcessReady(KProcIO *)));
	connect(details, SIGNAL(processExited(KProcess *)), this, SLOT(slotDetailsProcessExited(KProcess *)));

	if (!details->start(KProcess::NotifyOnExit, KProcess::Stderr)) {
		slotDetailsProcessExited(details);
		kdDebug(14170) << "DetailsProcess not started!" << endl;
	}
}

void WPUserInfo::slotDetailsProcessReady(KProcIO *d)
{
	QString tmpLine = QString::null;
	QRegExp info("^Domain=\\[(.*)\\]\\sOS=\\[(.*)\\]\\sServer=\\[(.*)\\]$"), host("^Server\\|(.*)\\|(.*)$");

	while (d->readln(tmpLine) > -1) {
		if (info.search(tmpLine) != -1) {
			Workgroup = info.cap(1);
			OS = info.cap(2);
			Software = info.cap(3);
		}
		if (host.search(tmpLine) != -1) {
			Comment = host.cap(2);
		}
	}
}

void WPUserInfo::slotDetailsProcessExited(KProcess *d)
{
	delete d;

	m_mainWidget->sComment->setText(Comment);
	m_mainWidget->sWorkgroup->setText(Workgroup);
	m_mainWidget->sOS->setText(OS);
	m_mainWidget->sServer->setText(Software);
}

void WPUserInfo::slotCloseClicked()
{
	kdDebug( 14170 ) << k_funcinfo << endl;

	emit closing();
}

#include "wpuserinfo.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
