//
// C++ Implementation: groupwisecontactproperties
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmap.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kopeteglobal.h>
#include <kopeteonlinestatus.h>
#include <kopetemetacontact.h>
#include <kopeteuiglobal.h>

#include "gwcontact.h"
#include "gwcontactpropswidget.h"
#include "gwprotocol.h"

#include "gwcontactproperties.h"

GroupWiseContactProperties::GroupWiseContactProperties( GroupWiseContact * contact, QObject *parent, const char *name)
 : QObject(parent, name)
{
	m_dialog = new KDialogBase( Kopete::UI::Global::mainWidget(), "gwcontactpropsdialog", false, i18n( "Contact Properties" ), KDialogBase::Ok );
	m_propsWidget = new GroupWiseContactPropsWidget( m_dialog );
	// set up the contents of the props widget
	m_propsWidget->m_userId->setText( contact->contactId() );
	m_propsWidget->m_status->setText( contact->onlineStatus().description() );
	m_propsWidget->m_displayName->setText( contact->metaContact()->displayName() );
	m_propsWidget->m_firstName->setText( contact->property( Kopete::Global::Properties::self()->firstName() ).value().toString() );
	m_propsWidget->m_lastName->setText( contact->property( Kopete::Global::Properties::self()->lastName() ).value().toString() );
	
	setupProperties( contact->serverProperties() );
	
	// insert the props widget into the dialog
	m_dialog->setMainWidget( m_propsWidget );
	m_dialog->show();
}

GroupWiseContactProperties::GroupWiseContactProperties( GroupWise::ContactDetails cd, QObject *parent, const char *name )
 : QObject(parent, name)
{
	m_dialog = new KDialogBase( Kopete::UI::Global::mainWidget(), "gwcontactpropsdialog", false, i18n( "Contact Properties" ), KDialogBase::Ok );
	m_propsWidget = new GroupWiseContactPropsWidget( m_dialog );
	// set up the contents of the props widget
	m_propsWidget->m_userId->setText( GroupWiseProtocol::protocol()->dnToDotted( cd.dn ) );
	m_propsWidget->m_status->setText( GroupWiseProtocol::protocol()->gwStatusToKOS( cd.status ).description() );
	m_propsWidget->m_displayName->setText( cd.fullName.isEmpty() ? ( cd.givenName + " " + cd.surname ) : cd.fullName );
	m_propsWidget->m_firstName->setText( cd.givenName );
	m_propsWidget->m_lastName->setText( cd.surname );
	
	setupProperties( cd.properties );
	
	// insert the props widget into the dialog
	m_dialog->setMainWidget( m_propsWidget );
	m_dialog->show();
}

void GroupWiseContactProperties::setupProperties( QMap< QString, QString > serverProps )
{
	// now do the properties
	m_propsWidget->m_propsView->header()->hide();
	m_propsWidget->m_propsView->setAllColumnsShowFocus( true );
	QMap< QString, QString >::Iterator it;
	QMap< QString, QString >::Iterator end = serverProps.end();
	for ( it = serverProps.begin(); it != end; ++it )
	{
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
			localised = i18n( "EMail Address" );
		else
			localised = key;

		new QListViewItem( m_propsWidget->m_propsView, localised, it.data() );
	}
}

GroupWiseContactProperties::~GroupWiseContactProperties()
{
}

#include "gwcontactproperties.moc"
