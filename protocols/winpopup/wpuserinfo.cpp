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
#include <QRegExp>

// KDE Includes
#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KConfig>

// Local Includes
#include "wpuserinfo.h"
#include "wpaccount.h"
#include "wpcontact.h"
#include "ui_wpuserinfowidget.h"

WPUserInfo::WPUserInfo( WPContact *contact, QWidget *parent )
	: KDialog( parent ), m_contact(contact),
	  Comment(i18n("N/A")), Workgroup(i18n("N/A")), OS(i18n("N/A")), Software(i18n("N/A"))
{
	setButtons( KDialog::Close );
	setDefaultButton(KDialog::Close);
//	kDebug( 14170 ) ;

	setCaption( i18n( "User Info for %1", m_contact->nickName() ) );

	QWidget* w = new QWidget( this );
	m_mainWidget = new Ui::WPUserInfoWidget();
	m_mainWidget->setupUi( w );
	setMainWidget( w );

	m_mainWidget->sComputerName->setText( m_contact->contactId() );

//	m_mainWidget->sComment->setText(i18n("Looking"));
//	m_mainWidget->sWorkgroup->setText(i18n("Looking"));
//	m_mainWidget->sOS->setText(i18n("Looking"));
//	m_mainWidget->sServer->setText(i18n("Looking"));

	connect( this, SIGNAL( closeClicked() ), this, SLOT( slotCloseClicked() ) );

	startDetailsProcess(m_contact->contactId());
}

WPUserInfo::~WPUserInfo()
{
	delete m_mainWidget;
}

// I decided to do this direct here to avoid "Handstände" with signals and stuff
// if we would do this in libwinpopup. GF
void WPUserInfo::startDetailsProcess(const QString &host)
{
	KConfigGroup group = KGlobal::config()->group("WinPopup");
	QString theSMBClientPath = group.readEntry("SMBClientPath", "/usr/bin/smbclient");

	detailsProcess = new QProcess(this);
	QStringList args;
	args << "-N" << "-g" << "-L" << host << "-";

	connect(detailsProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotDetailsProcessFinished(int, QProcess::ExitStatus)));

	detailsProcess->setProcessChannelMode(QProcess::MergedChannels);
	detailsProcess->start(theSMBClientPath, args);
}

void WPUserInfo::slotDetailsProcessFinished(int, QProcess::ExitStatus)
{
	QByteArray outputData = detailsProcess->readAll();
	QRegExp info("Domain=\\[(.[^\\]]+)\\]\\sOS=\\[(.[^\\]]+)\\]\\sServer=\\[(.[^\\]]+)\\]"),
			host("Server\\|" + m_contact->contactId() + "\\|(.*)");

	if (!outputData.isEmpty()) {
		QString output = QString::fromUtf8(outputData.data());
		QStringList outputList = output.split('\n');
		foreach (QString line, outputList) {
			if (info.indexIn(line) != -1) {
				Workgroup = info.cap(1);
				OS = info.cap(2);
				Software = info.cap(3);
			}
			if (host.indexIn(line) != -1) {
				Comment = host.cap(1);
			}
		}
	}

	delete detailsProcess;
	detailsProcess = 0;

	m_mainWidget->sComment->setText(Comment);
	m_mainWidget->sWorkgroup->setText(Workgroup);
	m_mainWidget->sOS->setText(OS);
	m_mainWidget->sServer->setText(Software);
}

void WPUserInfo::slotCloseClicked()
{
	kDebug( 14170 ) ;

	emit closing();
}

#include "wpuserinfo.moc"

// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; indent-width 4; replace-trailing-space-save on;
