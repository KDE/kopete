/*
    Kopete Groupwise Protocol
    gwcontactproperties.cpp - dialog showing a contact's server side properties

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qclipboard.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <klistview.h>
#include <qmap.h>
#include <qpopupmenu.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kopeteglobal.h>
#include <kopeteonlinestatus.h>
#include <kopetemetacontact.h>
#include <kopeteuiglobal.h>
#include <kaction.h>
#include <kstdaction.h>

#include "gwcontact.h"
#include "gwcontactpropswidget.h"
#include "gwprotocol.h"

#include "gwcontactproperties.h"

GroupWiseContactProperties::GroupWiseContactProperties( GroupWiseContact * contact, QWidget *parent, const char *name)
 : QObject(parent, name)
{
	init();
	// set up the contents of the props widget
	m_propsWidget->m_userId->setText( contact->contactId() );
	m_propsWidget->m_status->setText( contact->onlineStatus().description() );
	m_propsWidget->m_displayName->setText( contact->metaContact()->displayName() );
	m_propsWidget->m_firstName->setText( contact->property( Kopete::Global::Properties::self()->firstName() ).value().toString() );
	m_propsWidget->m_lastName->setText( contact->property( Kopete::Global::Properties::self()->lastName() ).value().toString() );
	
	setupProperties( contact->serverProperties() );
	m_dialog->show();
}

GroupWiseContactProperties::GroupWiseContactProperties( GroupWise::ContactDetails cd, QWidget *parent, const char *name )
 : QObject(parent, name)
{
	init();
	// set up the contents of the props widget
	m_propsWidget->m_userId->setText( GroupWiseProtocol::protocol()->dnToDotted( cd.dn ) );
	m_propsWidget->m_status->setText( GroupWiseProtocol::protocol()->gwStatusToKOS( cd.status ).description() );
	m_propsWidget->m_displayName->setText( cd.fullName.isEmpty() ? ( cd.givenName + " " + cd.surname ) : cd.fullName );
	m_propsWidget->m_firstName->setText( cd.givenName );
	m_propsWidget->m_lastName->setText( cd.surname );

	setupProperties( cd.properties );

	m_dialog->show();
}

GroupWiseContactProperties::~GroupWiseContactProperties()
{
}

void GroupWiseContactProperties::init()
{
	m_dialog = new KDialogBase( ::qt_cast<QWidget*>( parent() ), "gwcontactpropsdialog", false, i18n( "Contact Properties" ), KDialogBase::Ok );
	m_propsWidget = new GroupWiseContactPropsWidget( m_dialog );
	// set up the context menu and copy action
	m_copyAction = KStdAction::copy( this, SLOT( slotCopy() ), 0 );
	connect( m_propsWidget->m_propsView, 
			 SIGNAL( contextMenuRequested( QListViewItem *, const QPoint & , int) ),
			 SLOT( slotShowContextMenu( QListViewItem *, const QPoint & ) ) );

	// insert the props widget into the dialog
	m_dialog->setMainWidget( m_propsWidget );
}

void GroupWiseContactProperties::setupProperties( QMap< QString, QString > serverProps )
{
	m_propsWidget->m_propsView->header()->hide();
	QMap< QString, QString >::Iterator it;
	QMap< QString, QString >::Iterator end = serverProps.end();
	for ( it = serverProps.begin(); it != end; ++it )
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << " adding property: " << it.key() << ", " << it.data() << endl;
		QString key = it.key();
		QString localised;
		if ( key == "telephoneNumber" )
			localised = i18n( "Telephone Number" );
		else if ( key == "OU" )
			localised = i18n( "Department" );
		else if ( key == "L" )
			localised = i18n( "Location" );
		else if ( key == "mailstop" )
			localised = i18n( "Mailstop" );
		else if ( key == "personalTitle" )
			localised = i18n( "Personal Title" );
		else if ( key == "title" )
			localised = i18n( "Title" );
		else if ( key == "Internet EMail Address" )
			localised = i18n( "Email Address" );
		else
			localised = key;

		new KListViewItem( m_propsWidget->m_propsView, localised, it.data() );
	}
}

void GroupWiseContactProperties::slotShowContextMenu( QListViewItem * item, const QPoint & pos )
{
	if ( item )
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "for item " << item->text(0) << ", " << item->text(1) << endl;
	else
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "no selected item" << endl;
	QPopupMenu * popupMenu = new QPopupMenu( m_propsWidget->m_propsView );
	m_copyAction->plug( popupMenu );
	popupMenu->exec( pos );
}

void GroupWiseContactProperties::slotCopy()
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if ( m_propsWidget->m_propsView->currentItem() )
	{
		QClipboard *cb = kapp->clipboard();
		cb->setText( m_propsWidget->m_propsView->currentItem()->text( 1 ) );
	}
}
#include "gwcontactproperties.moc"
