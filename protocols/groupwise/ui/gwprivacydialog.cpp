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
#include <kmessagebox.h>

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
 : KDialogBase(  parent, name, false, i18n( "Account specific privacy settings", "Manage Privacy for %1" ).arg( account->accountId() ),
 KDialogBase::Ok|KDialogBase::Apply|KDialogBase::Cancel, Ok, true ), m_account( account ), m_dirty( false )
{
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

	populateWidgets();

	m_widget->m_allowList->setSelectionMode( QListBox::Extended );
	m_widget->m_denyList->setSelectionMode( QListBox::Extended );

	connect( m_widget->m_btnAllow, SIGNAL( clicked() ), SLOT( slotAllowClicked() ) );
	connect( m_widget->m_btnBlock, SIGNAL( clicked() ), SLOT( slotBlockClicked() ) );
	//connect( m_widget->m_btnAdd, SIGNAL( clicked() ), SLOT( slotAddClicked() ) );
	connect( m_widget->m_btnRemove, SIGNAL( clicked() ), SLOT( slotRemoveClicked() ) );
	connect( m_widget->m_allowList, SIGNAL( selectionChanged() ), SLOT( slotAllowListClicked() ) );
	connect( m_widget->m_denyList, SIGNAL( selectionChanged() ), SLOT( slotDenyListClicked() ) );
	connect( mgr, SIGNAL( privacyChanged( const QString &, bool ) ), SLOT( slotPrivacyChanged() ) );
	m_widget->m_btnAdd->setEnabled( false );
	m_widget->m_btnAllow->setEnabled( false );
	m_widget->m_btnBlock->setEnabled( false );
	m_widget->m_btnRemove->setEnabled( false );

/*	showButtonOK( true );
	showButtonApply( true );
	showButtonCancel( true );
	*/
	show();
}

GroupWisePrivacyDialog::~GroupWisePrivacyDialog()
{
}

void GroupWisePrivacyDialog::populateWidgets()
{
	m_dirty = false;
	PrivacyManager * mgr = m_account->client()->privacyManager();

	// default policy
	QString defaultPolicyText = i18n( "<Everyone Else>" );
	if ( mgr->defaultAllow() )
		m_defaultPolicy = new QListBoxText( m_widget->m_allowList, defaultPolicyText );
	else
		m_defaultPolicy = new QListBoxText( m_widget->m_denyList, defaultPolicyText );

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
	updateButtonState();
}

void GroupWisePrivacyDialog::disableWidgets()
{
	if ( m_widget )
	{
		m_widget->m_btnAllow->setEnabled( false );
		m_widget->m_btnBlock->setEnabled( false );
		m_widget->m_btnAdd->setEnabled( false );
		m_widget->m_btnRemove->setEnabled( false );
	}
}

void GroupWisePrivacyDialog::slotBlockClicked()
{
	// take each selected item from the allow list and add it to the deny list
	// start at the bottom, as we are changing the contents of the list as we go
	for( int i = m_widget->m_allowList->count() - 1; i >= 0 ; --i )
	{
		if ( m_widget->m_allowList->isSelected( i ) )
		{
			m_dirty = true;
			QListBoxItem * lbi = m_widget->m_allowList->item( i );
			m_widget->m_allowList->takeItem( lbi );
			m_widget->m_denyList->insertItem( lbi );
		}
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::slotAllowClicked()
{
	// take each selected item from the deny list and add it to the allow list
	for( int i = m_widget->m_denyList->count() - 1; i >= 0 ; --i )
	{
		if ( m_widget->m_denyList->isSelected( i ) )
		{
			m_dirty = true;
			QListBoxItem * lbi = m_widget->m_denyList->item( i );
			m_widget->m_denyList->takeItem( lbi );
			m_widget->m_allowList->insertItem( lbi );
		}
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::slotRemoveClicked()
{
	// remove any selected items from either list, except the default policy
	for( int i = m_widget->m_denyList->count() - 1; i >= 0 ; --i )
	{
		if ( m_widget->m_denyList->isSelected( i ) )
		{
			m_dirty = true;
			QListBoxItem * lbi = m_widget->m_denyList->item( i );
			// can't remove the default policy
			if ( lbi == m_defaultPolicy )
				continue;
			m_widget->m_denyList->removeItem( i );
		}
	}
	for( int i = m_widget->m_allowList->count() - 1; i >= 0 ; --i )
	{
		if ( m_widget->m_allowList->isSelected( i ) )
		{
			m_dirty = true;
			QListBoxItem * lbi = m_widget->m_allowList->item( i );
			// can't remove the default policy
			if ( lbi == m_defaultPolicy )
				continue;
			m_widget->m_allowList->removeItem( i );
		}
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::slotAllowListClicked()
{
	// don't get into feedback
	disconnect( m_widget->m_denyList, SIGNAL( selectionChanged() ), this, SLOT( slotDenyListClicked() ) );
	m_widget->m_denyList->clearSelection();
	connect( m_widget->m_denyList, SIGNAL( selectionChanged() ), SLOT( slotDenyListClicked() ) );
	bool selected = false;
	for( int i = m_widget->m_allowList->count() - 1; i >= 0 ; --i )
	{
		if ( m_widget->m_allowList->isSelected( i ) )
		{
			selected = true;
			break;
		}
	}
	m_widget->m_btnAllow->setEnabled( false );
	m_widget->m_btnBlock->setEnabled( selected );
	m_widget->m_btnRemove->setEnabled( selected );
}

void GroupWisePrivacyDialog::slotDenyListClicked()
{
	// don't get into feedback
	disconnect( m_widget->m_allowList, SIGNAL( selectionChanged() ), this, SLOT( slotAllowListClicked() ) );
	m_widget->m_allowList->clearSelection();
	connect( m_widget->m_allowList, SIGNAL( selectionChanged() ), SLOT( slotAllowListClicked() ) );
	bool selected = false;
	for( int i = m_widget->m_denyList->count() - 1; i >= 0 ; --i )
	{
		if ( m_widget->m_denyList->isSelected( i ) )
		{
			selected = true;
			break;
		}
	}
	m_widget->m_btnAllow->setEnabled( selected );
	m_widget->m_btnBlock->setEnabled( false );
	m_widget->m_btnRemove->setEnabled( selected );
}

void GroupWisePrivacyDialog::slotPrivacyChanged()
{
	m_widget->m_denyList->clear();
	m_widget->m_allowList->clear();
	populateWidgets();
}

void GroupWisePrivacyDialog::updateButtonState()
{
	enableButtonApply( m_dirty );
}

void GroupWisePrivacyDialog::slotOk()
{
	if ( m_dirty )
		commitChanges();
	KDialogBase::slotOk();
}

void GroupWisePrivacyDialog::slotApply()
{
	if ( m_dirty )
	{
		commitChanges();
		m_dirty = false;
		updateButtonState();
	}
	KDialogBase::slotApply();
}

void GroupWisePrivacyDialog::commitChanges()
{
	if ( m_account->isConnected() )
	{
		bool defaultDeny;
		QStringList denyList;
		QStringList allowList;
		// pass on our current allow, deny and default policy to the PrivacyManager
		for( int i = 0; i < (int)m_widget->m_denyList->count(); ++i )
		{
			if ( m_widget->m_denyList->item( i ) == m_defaultPolicy )
				defaultDeny = true;
			else
			{
				PrivacyLBI * lbi = static_cast<PrivacyLBI *>( m_widget->m_denyList->item( i ) );
				denyList.append( lbi->dn() );
			}
		}
		for( int i = 0; i < (int)m_widget->m_allowList->count(); ++i )
		{
			if ( m_widget->m_allowList->item( i ) == m_defaultPolicy )
				defaultDeny = false;
			else
			{
				PrivacyLBI * lbi = static_cast<PrivacyLBI *>( m_widget->m_allowList->item( i ) );
				allowList.append( lbi->dn() );
			}
		}
		PrivacyManager * mgr = m_account->client()->privacyManager();
		mgr->setPrivacy( defaultDeny, allowList, denyList );
	}
	else
		errorNotConnected();
}

void GroupWisePrivacyDialog::errorNotConnected()
{
	KMessageBox::queuedMessageBox( this, KMessageBox::Information,
			i18n( "You can only change privacy settings while you are logged in to the GroupWise Messenger server." ) , i18n("'%1' Not Logged In").arg( m_account->accountId() ) );
}

#include "gwprivacydialog.moc"
