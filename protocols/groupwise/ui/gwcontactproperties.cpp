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
	// now do the properties
	m_propsWidget->m_propsView->header()->hide();
	m_propsWidget->m_propsView->setAllColumnsShowFocus( true );
	QMap< QString, QString > serverProps = contact->serverProperties();
	QMap< QString, QString >::Iterator it;
	QMap< QString, QString >::Iterator end = serverProps.end();
	for ( it = serverProps.begin(); it != end; ++it )
	{
		new QListViewItem( m_propsWidget->m_propsView, it.key(), it.data() );
	}
	// insert the props widget into the dialog
	m_dialog->setMainWidget( m_propsWidget );
	m_dialog->show();
}


GroupWiseContactProperties::~GroupWiseContactProperties()
{
}


#include "gwcontactproperties.moc"
