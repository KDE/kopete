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
#include "icqauthreplyui.h"

#include <klocale.h>

#include <qlabel.h>
#include <qradiobutton.h>
#include <qlineedit.h>

ICQAuthReplyDialog::ICQAuthReplyDialog( QWidget *parent, const char *name, bool wasRequested )
 : KDialogBase( parent, name, true, i18n( "Authorization Reply" ), KDialogBase::Ok | KDialogBase::Cancel )
{
	m_ui = new ICQAuthReplyUI( this );
	setMainWidget( m_ui );
	m_wasRequested = wasRequested;
	
	if ( !m_wasRequested )
	{
		m_ui->lblReqReason->hide();
		m_ui->lblRequestReason->hide();
	}
	else
	{
		this->setWFlags( this->getWFlags() | Qt::WDestructiveClose );
	}
}

ICQAuthReplyDialog::~ICQAuthReplyDialog()
{
}

void ICQAuthReplyDialog::setUser( const QString & user )
{
	if ( m_wasRequested )
		m_ui->lblUserReq->setText( 
			i18n( "<b>%1</b> requested authorization to add you to his/her contact list." ).arg( user ) );
	else
		m_ui->lblUserReq->setText( i18n( "Authorization reply to <b>%1</b>." ).arg( user ) );
}

void ICQAuthReplyDialog::setRequestReason( const QString & reason )
{
	m_ui->lblRequestReason->setText( reason );
}

QString ICQAuthReplyDialog::reason()
{
	return m_ui->leReason->text();
}

bool ICQAuthReplyDialog::grantAuth()
{
	return m_ui->rbGrant->isChecked();
}

#include "icqauthreplydialog.moc"
