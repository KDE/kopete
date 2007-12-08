/*
   Kopete Oscar Protocol
   icqauthreplydialog.cpp - ICQ authorization reply dialog

   Copyright (c) 2004 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>

   Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "icqauthreplydialog.h"
#include "ui_icqauthreplyui.h"

#include <klocale.h>

#include <qlabel.h>
#include <qradiobutton.h>
#include <qlineedit.h>

ICQAuthReplyDialog::ICQAuthReplyDialog( QWidget *parent, bool wasRequested )
 : KDialog( parent )
{
	setCaption( i18n( "Authorization Reply" ) );
	setButtons( KDialog::Ok | KDialog::Cancel );

	m_ui = new Ui::ICQAuthReplyUI();
	QWidget *w = new QWidget( this );
	m_ui->setupUi( w );
	setMainWidget( w );
	m_wasRequested = wasRequested;
	
	if ( !m_wasRequested )
	{
		m_ui->lblReqReason->hide();
		m_ui->lblRequestReason->hide();
	}
	else
	{
		setAttribute( Qt::WA_DeleteOnClose );
	}
}

ICQAuthReplyDialog::~ICQAuthReplyDialog()
{
	delete m_ui;
}

void ICQAuthReplyDialog::setUser( const QString & user )
{
	if ( m_wasRequested )
		m_ui->lblUserReq->setText( 
			i18n( "<b>%1</b> requested authorization to add you to his/her contact list.", user ) );
	else
		m_ui->lblUserReq->setText( i18n( "Authorization reply to <b>%1</b>.", user ) );
}

void ICQAuthReplyDialog::setRequestReason( const QString & reason )
{
	m_ui->lblRequestReason->setText( reason );
}

void ICQAuthReplyDialog::setContact( const QString& contact )
{
	m_contact = contact;
}

QString ICQAuthReplyDialog::reason() const
{
	return m_ui->leReason->text();
}

QString ICQAuthReplyDialog::contact() const
{
	return m_contact;
}

bool ICQAuthReplyDialog::grantAuth() const
{
	return m_ui->rbGrant->isChecked();
}

#include "icqauthreplydialog.moc"
