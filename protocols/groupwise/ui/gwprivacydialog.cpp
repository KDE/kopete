//
// C++ Implementation: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <klocale.h>

#include "client.h"
#include "gwaccount.h"
#include "gwprivacy.h"
#include "privacymanager.h"

#include "gwprivacydialog.h"

GroupWisePrivacyDialog::GroupWisePrivacyDialog( GroupWiseAccount * account, QWidget *parent, const char *name )
 : KDialogBase( i18n( "Account specific privacy settings", "Manage Privacy for %1" ).arg( account->accountId() ),
 KDialogBase::Ok|KDialogBase::Apply|KDialogBase::Cancel, KDialogBase::Ok, KDialogBase::Cancel, parent, name, false )
{
	m_account = account;
	m_widget = new GroupWisePrivacyWidget( this );
	setMainWidget( m_widget );
	PrivacyManager * mgr = m_account->client()->privacyManager();
	// populate the widget;
	// admin lock
	if ( mgr->isPrivacyLocked() )
	{
		m_widget->m_status->setText( i18n( "Privacy settings have been administratively locked" ) );
		disableWidgets();
	}
	// default policy
	if ( mgr->defaultAllow() )
	{
		m_widget->m_allowList->insertItem( i18n( "<Everyone Else>" ) );
		m_defaultPolicy = m_widget->m_allowList->selectedItem();
	}
	else
	{
		m_widget->m_denyList->insertItem( i18n( "<Everyone Else>" ) );
		m_defaultPolicy = m_widget->m_denyList->selectedItem();
	}
	// allow list
	QStringList allowList = mgr->allowList();
	QStringList::Iterator end = allowList.end();
	for ( QStringList::Iterator it = allowList.begin(); it != end; ++it )
		m_widget->m_allowList->insertItem( *it );

	// deny list
	QStringList denyList = mgr->denyList();
	end = denyList.end();
	for ( QStringList::Iterator it = denyList.begin(); it != end; ++it )
		m_widget->m_denyList->insertItem( *it );

	show();
}

GroupWisePrivacyDialog::~GroupWisePrivacyDialog()
{
}

void GroupWisePrivacyDialog::disableWidgets()
{

}
