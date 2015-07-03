/*
    Kopete Groupwise Protocol
    gwcontactproperties.cpp - dialog showing a contact's server side properties

    Copyright (c) 2006,2007 Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Kopete (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwcontactproperties.h"
#include <qclipboard.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmap.h>
#include <QMenu>
#include <QHeaderView>
#include <QTreeWidget>

#include <kdebug.h>
#include <klocale.h>
#include <kopeteglobal.h>
#include <kopeteonlinestatus.h>
#include <kopetemetacontact.h>
#include <kopeteuiglobal.h>
#include <kaction.h>
#include <kstandardaction.h>

#include "gwcontact.h"
#include "gwprotocol.h"

GroupWiseContactProperties::GroupWiseContactProperties( GroupWiseContact * contact, QWidget *parent )
 : QObject(parent)
{
	init();
	// set up the contents of the props widget
	m_ui.userId->setText( contact->contactId() );
	m_ui.status->setText( contact->onlineStatus().description() );
	m_ui.displayName->setText( contact->metaContact()->displayName() );
	m_ui.firstName->setText( contact->property( Kopete::Global::Properties::self()->firstName() ).value().toString() );
	m_ui.lastName->setText( contact->property( Kopete::Global::Properties::self()->lastName() ).value().toString() );
	
	setupProperties( contact->serverProperties() );
	m_dialog->show();
}

GroupWiseContactProperties::GroupWiseContactProperties( GroupWise::ContactDetails cd, QWidget *parent )
 : QObject(parent)
{
	init();
	// set up the contents of the props widget
	m_ui.userId->setText( GroupWiseProtocol::protocol()->dnToDotted( cd.dn ) );
	m_ui.status->setText( GroupWiseProtocol::protocol()->gwStatusToKOS( cd.status ).description() );
	m_ui.displayName->setText( cd.fullName.isEmpty() ? ( cd.givenName + ' ' + cd.surname ) : cd.fullName );
	m_ui.firstName->setText( cd.givenName );
	m_ui.lastName->setText( cd.surname );

	setupProperties( cd.properties );

	m_dialog->show();
}

GroupWiseContactProperties::~GroupWiseContactProperties()
{
}

void GroupWiseContactProperties::init()
{
	m_dialog = new KDialog( qobject_cast<QWidget*>( parent() ));
	m_dialog->setCaption(i18n( "Contact Properties" ));
	m_dialog->setButtons(KDialog::Ok);
	m_dialog->setDefaultButton(KDialog::Ok);
	m_dialog->setModal(false);
	QWidget * wid = new QWidget( m_dialog );
	m_dialog->setMainWidget( wid );
	m_ui.setupUi( wid );
	m_copyAction = KStandardAction::copy( this, SLOT(copy()), 0 );
	m_ui.propsView->addAction( m_copyAction );
}

void GroupWiseContactProperties::setupProperties( QMap< QString, QVariant > serverProps )
{
	m_ui.propsView->header()->hide();
	QMapIterator< QString, QVariant > i( serverProps );
	while ( i.hasNext() )
	{
		i.next();
		QString key = i.key();
		kDebug() << " adding property: " << key << ", " << i.value();
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

		QTreeWidgetItem * item = new QTreeWidgetItem( m_ui.propsView, 0 );
		item->setText( 0, localised );
		item->setText( 1, i.value().toString() );
	}
}

void GroupWiseContactProperties::copy()
{
	kDebug() ;
	QList<QTreeWidgetItem *> selection = m_ui.propsView->selectedItems();
	if ( !selection.isEmpty() )
	{
		QClipboard *cb = QApplication::clipboard();
		cb->setText( selection.first()->text( 1 ) );
	}
}
#include "gwcontactproperties.moc"
