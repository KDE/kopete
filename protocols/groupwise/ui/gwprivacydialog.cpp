/*
    Kopete Groupwise Protocol
    gwprivacydialog.cpp - dialog summarising, and editing, the user's privacy settings

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

#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qlistview.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "client.h"
#include "gwaccount.h"
#include "gwprivacy.h"
#include "gwprotocol.h"
#include "gwsearch.h"
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
 KDialogBase::Ok|KDialogBase::Apply|KDialogBase::Cancel, Ok, true ), m_account( account ), m_dirty( false ), m_searchDlg(0)
{
	m_privacy = new GroupWisePrivacyWidget( this );
	setMainWidget( m_privacy );
	PrivacyManager * mgr = m_account->client()->privacyManager();
	// populate the widget;
	// admin lock
	if ( mgr->isPrivacyLocked() )
	{
		m_privacy->m_status->setText( i18n( "Privacy settings have been administratively locked" ) );
		disableWidgets();
	}

	populateWidgets();

	m_privacy->m_allowList->setSelectionMode( QListBox::Extended );
	m_privacy->m_denyList->setSelectionMode( QListBox::Extended );

	connect( m_privacy->m_btnAllow, SIGNAL( clicked() ), SLOT( slotAllowClicked() ) );
	connect( m_privacy->m_btnBlock, SIGNAL( clicked() ), SLOT( slotBlockClicked() ) );
	connect( m_privacy->m_btnAdd, SIGNAL( clicked() ), SLOT( slotAddClicked() ) );
	connect( m_privacy->m_btnRemove, SIGNAL( clicked() ), SLOT( slotRemoveClicked() ) );
	connect( m_privacy->m_allowList, SIGNAL( selectionChanged() ), SLOT( slotAllowListClicked() ) );
	connect( m_privacy->m_denyList, SIGNAL( selectionChanged() ), SLOT( slotDenyListClicked() ) );
	connect( mgr, SIGNAL( privacyChanged( const QString &, bool ) ), SLOT( slotPrivacyChanged() ) );
	m_privacy->m_btnAdd->setEnabled( true );
	m_privacy->m_btnAllow->setEnabled( false );
	m_privacy->m_btnBlock->setEnabled( false );
	m_privacy->m_btnRemove->setEnabled( false );

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
		m_defaultPolicy = new QListBoxText( m_privacy->m_allowList, defaultPolicyText );
	else
		m_defaultPolicy = new QListBoxText( m_privacy->m_denyList, defaultPolicyText );

	QPixmap icon = m_account->protocol()->groupwiseAvailable.iconFor( m_account );

	// allow list
	QStringList allowList = mgr->allowList();
	QStringList::Iterator end = allowList.end();
	for ( QStringList::Iterator it = allowList.begin(); it != end; ++it )
	{
		GroupWise::ContactDetails cd = m_account->client()->userDetailsManager()->details( *it );
		if ( cd.fullName.isEmpty() )
			cd.fullName = cd.givenName + " " + cd.surname;
		new PrivacyLBI( m_privacy->m_allowList, icon, cd.fullName, *it );
	}
	// deny list
	QStringList denyList = mgr->denyList();
	end = denyList.end();
	for ( QStringList::Iterator it = denyList.begin(); it != end; ++it )
	{
		GroupWise::ContactDetails cd = m_account->client()->userDetailsManager()->details( *it );
		if ( cd.fullName.isEmpty() )
			cd.fullName = cd.givenName + " " + cd.surname;
		new PrivacyLBI( m_privacy->m_denyList, icon, cd.fullName, *it );
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::disableWidgets()
{
	if ( m_privacy )
	{
		m_privacy->m_btnAllow->setEnabled( false );
		m_privacy->m_btnBlock->setEnabled( false );
		m_privacy->m_btnAdd->setEnabled( false );
		m_privacy->m_btnRemove->setEnabled( false );
	}
}

void GroupWisePrivacyDialog::slotBlockClicked()
{
	// take each selected item from the allow list and add it to the deny list
	// start at the bottom, as we are changing the contents of the list as we go
	for( int i = m_privacy->m_allowList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy->m_allowList->isSelected( i ) )
		{
			m_dirty = true;
			QListBoxItem * lbi = m_privacy->m_allowList->item( i );
			m_privacy->m_allowList->takeItem( lbi );
			m_privacy->m_denyList->insertItem( lbi );
		}
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::slotAllowClicked()
{
	// take each selected item from the deny list and add it to the allow list
	for( int i = m_privacy->m_denyList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy->m_denyList->isSelected( i ) )
		{
			m_dirty = true;
			QListBoxItem * lbi = m_privacy->m_denyList->item( i );
			m_privacy->m_denyList->takeItem( lbi );
			m_privacy->m_allowList->insertItem( lbi );
		}
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::slotAddClicked()
{
	if ( !m_searchDlg )
	{
		m_searchDlg = new KDialogBase( this, "privacysearchdialog", false, 
				i18n( "Search for Contact to Block" ),
				KDialogBase::Ok|KDialogBase::Cancel );
		m_search = new GroupWiseContactSearch( m_account, QListView::Multi, false, m_searchDlg, "privacysearchwidget" );
		m_searchDlg->setMainWidget( m_search );
		connect( m_searchDlg, SIGNAL( okClicked() ), SLOT( slotSearchedForUsers() ) );
		connect( m_search, SIGNAL( selectionValidates( bool ) ), m_searchDlg, SLOT( enableButtonOK( bool ) ) );
		m_searchDlg->enableButtonOK( false );
	}
	m_searchDlg->show();
}

void GroupWisePrivacyDialog::slotSearchedForUsers()
{
	// create an item for each result, in the block list
	QValueList< ContactDetails > selected = m_search->selectedResults();
	QValueList< ContactDetails >::Iterator it = selected.begin();
	const QValueList< ContactDetails >::Iterator end = selected.end();
	QPixmap icon = m_account->protocol()->groupwiseAvailable.iconFor( m_account );
	for ( ; it != end; ++it )
	{
		m_dirty = true;
		m_account->client()->userDetailsManager()->addDetails( *it );
		if ( (*it).fullName.isEmpty() )
			(*it).fullName = (*it).givenName + " " + (*it).surname;
		new PrivacyLBI( m_privacy->m_denyList, icon, (*it).fullName, (*it).dn );
	}
}

void GroupWisePrivacyDialog::slotRemoveClicked()
{
	// remove any selected items from either list, except the default policy
	for( int i = m_privacy->m_denyList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy->m_denyList->isSelected( i ) )
		{
			m_dirty = true;
			QListBoxItem * lbi = m_privacy->m_denyList->item( i );
			// can't remove the default policy
			if ( lbi == m_defaultPolicy )
				continue;
			m_privacy->m_denyList->removeItem( i );
		}
	}
	for( int i = m_privacy->m_allowList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy->m_allowList->isSelected( i ) )
		{
			m_dirty = true;
			QListBoxItem * lbi = m_privacy->m_allowList->item( i );
			// can't remove the default policy
			if ( lbi == m_defaultPolicy )
				continue;
			m_privacy->m_allowList->removeItem( i );
		}
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::slotAllowListClicked()
{
	// don't get into feedback
	disconnect( m_privacy->m_denyList, SIGNAL( selectionChanged() ), this, SLOT( slotDenyListClicked() ) );
	m_privacy->m_denyList->clearSelection();
	connect( m_privacy->m_denyList, SIGNAL( selectionChanged() ), SLOT( slotDenyListClicked() ) );
	bool selected = false;
	for( int i = m_privacy->m_allowList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy->m_allowList->isSelected( i ) )
		{
			selected = true;
			break;
		}
	}
	m_privacy->m_btnAllow->setEnabled( false );
	m_privacy->m_btnBlock->setEnabled( selected );
	m_privacy->m_btnRemove->setEnabled( selected );
}

void GroupWisePrivacyDialog::slotDenyListClicked()
{
	// don't get into feedback
	disconnect( m_privacy->m_allowList, SIGNAL( selectionChanged() ), this, SLOT( slotAllowListClicked() ) );
	m_privacy->m_allowList->clearSelection();
	connect( m_privacy->m_allowList, SIGNAL( selectionChanged() ), SLOT( slotAllowListClicked() ) );
	bool selected = false;
	for( int i = m_privacy->m_denyList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy->m_denyList->isSelected( i ) )
		{
			selected = true;
			break;
		}
	}
	m_privacy->m_btnAllow->setEnabled( selected );
	m_privacy->m_btnBlock->setEnabled( false );
	m_privacy->m_btnRemove->setEnabled( selected );
}

void GroupWisePrivacyDialog::slotPrivacyChanged()
{
	m_privacy->m_denyList->clear();
	m_privacy->m_allowList->clear();
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
		bool defaultDeny = false;
		QStringList denyList;
		QStringList allowList;
		// pass on our current allow, deny and default policy to the PrivacyManager
		for( int i = 0; i < (int)m_privacy->m_denyList->count(); ++i )
		{
			if ( m_privacy->m_denyList->item( i ) == m_defaultPolicy )
				defaultDeny = true;
			else
			{
				PrivacyLBI * lbi = static_cast<PrivacyLBI *>( m_privacy->m_denyList->item( i ) );
				denyList.append( lbi->dn() );
			}
		}
		for( int i = 0; i < (int)m_privacy->m_allowList->count(); ++i )
		{
			if ( m_privacy->m_allowList->item( i ) == m_defaultPolicy )
				defaultDeny = false;
			else
			{
				PrivacyLBI * lbi = static_cast<PrivacyLBI *>( m_privacy->m_allowList->item( i ) );
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
