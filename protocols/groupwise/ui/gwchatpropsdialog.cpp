/*
    Kopete Groupwise Protocol
    gwchatpropsdialog.h - dialog for viewing/modifying chat properties

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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>

#include <kdebug.h>
#include <kpushbutton.h>
#include <klocale.h>
#include "gwerror.h"
#include "gwchatpropswidget.h"

#include "gwchatpropsdialog.h"

GroupWiseChatPropsDialog::GroupWiseChatPropsDialog( QWidget * parent, const char * name )
 : KDialogBase( parent, name, false, i18n( "Chatroom properties" ),
				KDialogBase::Ok|KDialogBase::Cancel, Ok, true ), m_dirty( false )
{
	initialise();
}

GroupWiseChatPropsDialog::GroupWiseChatPropsDialog( const GroupWise::Chatroom & room, bool readOnly,
							   QWidget * parent, const char * name )
	: KDialogBase( parent, name, false, i18n( "Chatroom properties" ),
				   KDialogBase::Ok|KDialogBase::Cancel, Ok, true ), m_dirty( false )
{
	initialise();
	m_widget->m_description->setText( room.description );
	m_widget->m_displayName->setText( room.displayName );
	m_widget->m_disclaimer->setText( room.disclaimer );
	m_widget->m_owner->setText( room.ownerDN );
	m_widget->m_query->setText( room.query );
	m_widget->m_topic->setText( room.topic );
	m_widget->m_archive->setChecked( room.archive );
	m_widget->m_maxUsers->setText( QString::number( room.maxUsers ) );
	m_widget->m_createdOn->setText( room.createdOn.toString() );
	m_widget->m_creator->setText( room.creatorDN );
	
	m_widget->m_chkRead->setChecked( room.chatRights & GroupWise::Chatroom::Read || room.chatRights & GroupWise::Chatroom::Write || room.chatRights & GroupWise::Chatroom::Owner );
	m_widget->m_chkWrite->setChecked( room.chatRights & GroupWise::Chatroom::Write || room.chatRights & GroupWise::Chatroom::Owner );
	m_widget->m_chkModify->setChecked( room.chatRights & GroupWise::Chatroom::Modify || room.chatRights & GroupWise::Chatroom::Owner );

	if ( readOnly )
	{
		m_widget->m_description->setReadOnly( true );
		m_widget->m_disclaimer->setReadOnly( true );
		m_widget->m_owner->setReadOnly( true );
		m_widget->m_query->setReadOnly( true );
		m_widget->m_topic->setReadOnly( true );
		m_widget->m_archive->setEnabled( false );
		m_widget->m_maxUsers->setReadOnly( true );
		m_widget->m_createdOn->setReadOnly( true );
		m_widget->m_creator->setReadOnly( true );
		m_widget->m_chkRead->setEnabled( false );
		m_widget->m_chkWrite->setEnabled( false );
		m_widget->m_chkModify->setEnabled( false );
		m_widget->m_btnAddAcl->setEnabled( false );
		m_widget->m_btnEditAcl->setEnabled( false );
		m_widget->m_btnDeleteAcl->setEnabled( false );
	}
	
}

void GroupWiseChatPropsDialog::initialise()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	m_widget = new GroupWiseChatPropsWidget( this );
	connect( m_widget->m_topic, SIGNAL( textChanged( const QString & )  ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_owner, SIGNAL( textChanged( const QString & ) ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_createdOn, SIGNAL( textChanged( const QString & ) ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_creator, SIGNAL( textChanged( const QString & ) ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_description, SIGNAL( textChanged( const QString & ) ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_disclaimer, SIGNAL( textChanged( const QString & ) ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_query, SIGNAL( textChanged( const QString & ) ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_archive, SIGNAL( textChanged( const QString & ) ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_maxUsers, SIGNAL( textChanged( const QString & ) ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_btnAddAcl, SIGNAL( clicked() ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_btnEditAcl, SIGNAL( clicked() ), SLOT( slotWidgetChanged() ) );
	connect( m_widget->m_btnDeleteAcl, SIGNAL( clicked() ), SLOT( slotWidgetChanged() ) );
	setMainWidget( m_widget );
	show();
}

GroupWise::Chatroom GroupWiseChatPropsDialog::room()
{
	GroupWise::Chatroom room;
	room.description = m_widget->m_description->text();
	room.displayName = m_widget->m_displayName->text();
	room.disclaimer = m_widget->m_disclaimer->text();
	room.ownerDN = m_widget->m_owner->text();
	room.query = m_widget->m_query->text();
	room.topic = m_widget->m_topic->text();
	room.archive = m_widget->m_archive->isChecked();
	room.maxUsers = m_widget->m_maxUsers->text().toInt();
	
// 	room.
	return room;
}

void GroupWiseChatPropsDialog::slotWidgetChanged()
{
	m_dirty = true;
}

#include "gwchatpropsdialog.moc"
