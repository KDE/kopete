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
#include <kiconloader.h>
#include <klocale.h>

#include "client.h"
#include "gwaccount.h"
#include "gwprivacy.h"
#include "gwprotocol.h"

#include "privacymanager.h"
#include "userdetailsmanager.h"
#include "gwprivacydialog.h"

class PrivacyLBI : public QListBoxPixmap
{
public:
	PrivacyLBI( QListBox * listBox, const QPixmap & pixmap, const QString & text, const QString & dn )
	: QListBoxPixmap( listBox, pixmap, text ), m_dn( dn )
	{
	}
	QString dn() { return m_dn; }
private:
	QString m_dn;
};

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
	QString defaultPolicyText = i18n( "<Everyone Else>" );
	if ( mgr->defaultAllow() )
	{
	
		m_widget->m_allowList->insertItem( defaultPolicyText );
		m_defaultPolicy = m_widget->m_allowList->selectedItem();
	}
	else
	{
		m_widget->m_denyList->insertItem( defaultPolicyText );
		m_defaultPolicy = m_widget->m_denyList->selectedItem();
	}
	QPixmap icon = m_account->protocol()->groupwiseAvailable.iconFor( m_account );

	// allow list
	QStringList allowList = mgr->allowList();
	QStringList::Iterator end = allowList.end();
	for ( QStringList::Iterator it = allowList.begin(); it != end; ++it )
	{
		GroupWise::ContactDetails cd = m_account->client()->userDetailsManager()->details( *it );
		if ( cd.fullName.isEmpty() )
			cd.fullName = cd.givenName + " " + cd.surname;
		new PrivacyLBI( m_widget->m_allowList, icon, cd.fullName, *it );
	}
	// deny list
	QStringList denyList = mgr->denyList();
	end = denyList.end();
	for ( QStringList::Iterator it = denyList.begin(); it != end; ++it )
	{
		GroupWise::ContactDetails cd = m_account->client()->userDetailsManager()->details( *it );
		if ( cd.fullName.isEmpty() )
			cd.fullName = cd.givenName + " " + cd.surname;
		new PrivacyLBI( m_widget->m_denyList, icon, cd.fullName, *it );
	}
	show();
}

GroupWisePrivacyDialog::~GroupWisePrivacyDialog()
{
}

void GroupWisePrivacyDialog::disableWidgets()
{

}
