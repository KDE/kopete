/*
    Kopete Groupwise Protocol
    gwchatpropsdialog.h - dialog for viewing/modifying chat properties

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2005      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwchatpropsdialog.h"
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <QPainter>

#include <kdebug.h>
#include <kpushbutton.h>
#include <klocale.h>
#include "gwerror.h"

GroupWiseChatPropsDialog::GroupWiseChatPropsDialog( QWidget * parent )
 : KDialog( parent ), m_dirty( false )
{
	setCaption(i18n( "Chatroom properties" ));
	setButtons(KDialog::Ok|KDialog::Cancel);
	setDefaultButton(Ok);
	setModal(false);
	showButtonSeparator(true);
	initialise();
}

GroupWiseChatPropsDialog::GroupWiseChatPropsDialog( const GroupWise::Chatroom & room, bool readOnly,
							   QWidget * parent )
	: KDialog( parent)
				   , m_dirty( false )
{
	setCaption(i18n( "Chatroom properties" ));
	setButtons(KDialog::Ok|KDialog::Cancel);
	setDefaultButton(Ok);
	setModal(false);
	showButtonSeparator(true);
	initialise();
	m_ui.description->setText( room.description );
	m_ui.displayName->setText( room.displayName );
	m_ui.disclaimer->setText( room.disclaimer );
	m_ui.owner->setText( room.ownerDN );
	m_ui.query->setText( room.query );
	m_ui.topic->setText( room.topic );
	m_ui.archive->setChecked( room.archive );
	m_ui.maxUsers->setText( QString::number( room.maxUsers ) );
	m_ui.createdOn->setText( room.createdOn.toString() );
	m_ui.creator->setText( room.creatorDN );
	
	m_ui.chkRead->setChecked( room.chatRights & GroupWise::Chatroom::Read || room.chatRights & GroupWise::Chatroom::Write || room.chatRights & GroupWise::Chatroom::Owner );
	m_ui.chkWrite->setChecked( room.chatRights & GroupWise::Chatroom::Write || room.chatRights & GroupWise::Chatroom::Owner );
	m_ui.chkModify->setChecked( room.chatRights & GroupWise::Chatroom::Modify || room.chatRights & GroupWise::Chatroom::Owner );

	if ( readOnly )
	{
		m_ui.description->setReadOnly( true );
		m_ui.disclaimer->setReadOnly( true );
		m_ui.owner->setReadOnly( true );
		m_ui.query->setReadOnly( true );
		m_ui.topic->setReadOnly( true );
		m_ui.archive->setEnabled( false );
		m_ui.maxUsers->setReadOnly( true );
		m_ui.createdOn->setReadOnly( true );
		m_ui.creator->setReadOnly( true );
		m_ui.chkRead->setEnabled( false );
		m_ui.chkWrite->setEnabled( false );
		m_ui.chkModify->setEnabled( false );
		m_ui.addAcl->setEnabled( false );
		m_ui.editAcl->setEnabled( false );
		m_ui.deleteAcl->setEnabled( false );
	}
	
}

GroupWiseChatPropsDialog::~GroupWiseChatPropsDialog()
{
}

void GroupWiseChatPropsDialog::initialise()
{
	kDebug() ;
	QWidget * wid = new QWidget( this );
	m_ui.setupUi( wid );
	setMainWidget( wid );
	connect( m_ui.topic, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()) );
	connect( m_ui.owner, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()) );
	connect( m_ui.createdOn, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()) );
	connect( m_ui.creator, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()) );
	connect( m_ui.description, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()) );
	connect( m_ui.disclaimer, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()) );
	connect( m_ui.query, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()) );
	connect( m_ui.archive, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()) );
	connect( m_ui.maxUsers, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()) );
	connect( m_ui.addAcl, SIGNAL(clicked()), SLOT(slotWidgetChanged()) );
	connect( m_ui.editAcl, SIGNAL(clicked()), SLOT(slotWidgetChanged()) );
	connect( m_ui.deleteAcl, SIGNAL(clicked()), SLOT(slotWidgetChanged()) );

	show();
}

GroupWise::Chatroom GroupWiseChatPropsDialog::room()
{
	GroupWise::Chatroom room;
	room.description = m_ui.description->text();
	room.displayName = m_ui.displayName->text();
	room.disclaimer = m_ui.disclaimer->text();
	room.ownerDN = m_ui.owner->text();
	room.query = m_ui.query->text();
	room.topic = m_ui.topic->text();
	room.archive = m_ui.archive->isChecked();
	room.maxUsers = m_ui.maxUsers->text().toInt();
	
// 	room.
	return room;
}

void GroupWiseChatPropsDialog::slotWidgetChanged()
{
	m_dirty = true;
}

#include "gwchatpropsdialog.moc"
