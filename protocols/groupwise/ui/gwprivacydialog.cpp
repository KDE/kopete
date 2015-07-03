/*
    Kopete Groupwise Protocol
    gwprivacydialog.cpp - dialog summarising, and editing, the user's privacy settings

    Copyright (c) 2006      Novell, Inc	 	 	 http://www.opensuse.org
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

#include "gwprivacydialog.h"
#include <q3listbox.h>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QLabel>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "client.h"
#include "gwaccount.h"
#include "ui_gwcontactsearch.h"
#include "gwprotocol.h"
#include "gwsearch.h"
#include "privacymanager.h"
#include "userdetailsmanager.h"

class PrivacyLBI : public Q3ListBoxPixmap
{
public:
	PrivacyLBI( Q3ListBox * listBox, const QPixmap & pixmap, const QString & text, const QString & dn )
	: Q3ListBoxPixmap( listBox, pixmap, text ), m_dn( dn )
	{
	}
	QString dn() { return m_dn; }
private:
	QString m_dn;
};

GroupWisePrivacyDialog::GroupWisePrivacyDialog( GroupWiseAccount * account, QWidget *parent, const char * /*name*/ )
 : KDialog(  parent)
 , m_account( account ), m_dirty( false ), m_searchDlg(0)
{
	setCaption(i18nc( "Account specific privacy settings", "Manage Privacy for %1", account->accountId() ));
	setButtons(KDialog::Ok|KDialog::Apply|KDialog::Cancel);
	setDefaultButton(Ok);
	setModal(false);
	QWidget * wid = new QWidget( this );
	m_privacy.setupUi( wid );
	setMainWidget( wid );
	PrivacyManager * mgr = m_account->client()->privacyManager();
	// populate the widget;
	// admin lock
	if ( mgr->isPrivacyLocked() )
	{
		m_privacy.status->setText( i18n( "Privacy settings have been administratively locked" ) );
		disableWidgets();
	}

	populateWidgets();

	m_privacy.allowList->setSelectionMode( Q3ListBox::Extended );
	m_privacy.denyList->setSelectionMode( Q3ListBox::Extended );

	connect( m_privacy.btnAllow, SIGNAL(clicked()), SLOT(slotAllowClicked()) );
	connect( m_privacy.btnBlock, SIGNAL(clicked()), SLOT(slotBlockClicked()) );
	connect( m_privacy.btnAdd, SIGNAL(clicked()), SLOT(slotAddClicked()) );
	connect( m_privacy.btnRemove, SIGNAL(clicked()), SLOT(slotRemoveClicked()) );
	connect( m_privacy.allowList, SIGNAL(selectionChanged()), SLOT(slotAllowListClicked()) );
	connect( m_privacy.denyList, SIGNAL(selectionChanged()), SLOT(slotDenyListClicked()) );
	connect( mgr, SIGNAL(privacyChanged(QString,bool)), SLOT(slotPrivacyChanged()) );
	connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
	connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
	m_privacy.btnAdd->setEnabled( true );
	m_privacy.btnAllow->setEnabled( false );
	m_privacy.btnBlock->setEnabled( false );
	m_privacy.btnRemove->setEnabled( false );

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
		m_defaultPolicy = new Q3ListBoxText( m_privacy.allowList, defaultPolicyText );
	else
		m_defaultPolicy = new Q3ListBoxText( m_privacy.denyList, defaultPolicyText );

	QPixmap icon = m_account->protocol()->groupwiseAvailable.iconFor( m_account ).pixmap( 16 );

	// allow list
	QStringList allowList = mgr->allowList();
	QStringList::Iterator end = allowList.end();
	for ( QStringList::Iterator it = allowList.begin(); it != end; ++it )
	{
		GroupWise::ContactDetails cd = m_account->client()->userDetailsManager()->details( *it );
		if ( cd.fullName.isEmpty() )
			cd.fullName = cd.givenName + ' ' + cd.surname;
		new PrivacyLBI( m_privacy.allowList, icon, cd.fullName, *it );
	}
	// deny list
	QStringList denyList = mgr->denyList();
	end = denyList.end();
	for ( QStringList::Iterator it = denyList.begin(); it != end; ++it )
	{
		GroupWise::ContactDetails cd = m_account->client()->userDetailsManager()->details( *it );
		if ( cd.fullName.isEmpty() )
			cd.fullName = cd.givenName + ' ' + cd.surname;
		new PrivacyLBI( m_privacy.denyList, icon, cd.fullName, *it );
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::disableWidgets()
{
	m_privacy.btnAllow->setEnabled( false );
	m_privacy.btnBlock->setEnabled( false );
	m_privacy.btnAdd->setEnabled( false );
	m_privacy.btnRemove->setEnabled( false );
}

void GroupWisePrivacyDialog::slotBlockClicked()
{
	// take each selected item from the allow list and add it to the deny list
	// start at the bottom, as we are changing the contents of the list as we go
	for( int i = m_privacy.allowList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy.allowList->isSelected( i ) )
		{
			m_dirty = true;
			Q3ListBoxItem * lbi = m_privacy.allowList->item( i );
			m_privacy.allowList->takeItem( lbi );
			m_privacy.denyList->insertItem( lbi );
		}
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::slotAllowClicked()
{
	// take each selected item from the deny list and add it to the allow list
	for( int i = m_privacy.denyList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy.denyList->isSelected( i ) )
		{
			m_dirty = true;
			Q3ListBoxItem * lbi = m_privacy.denyList->item( i );
			m_privacy.denyList->takeItem( lbi );
			m_privacy.allowList->insertItem( lbi );
		}
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::slotAddClicked()
{
	if ( !m_searchDlg )
	{
		m_searchDlg = new KDialog( this);
		m_searchDlg->setCaption(i18n( "Search for Contact to Block" ));
		m_searchDlg->setButtons(KDialog::Ok|KDialog::Cancel );
		m_searchDlg->setDefaultButton(KDialog::Ok);
		m_searchDlg->setModal(false);
		m_search = new GroupWiseContactSearch( m_account, QAbstractItemView::ExtendedSelection, false, m_searchDlg );
		m_searchDlg->setMainWidget( m_search );
		QObject::connect( m_searchDlg, SIGNAL(okClicked()), SLOT(slotSearchedForUsers()) );
		QObject::connect( m_search, SIGNAL(selectionValidates(bool)), m_searchDlg, SLOT(enableButtonOk(bool)) );
		m_searchDlg->enableButtonOk( false );
	}
	m_searchDlg->show();
}

void GroupWisePrivacyDialog::slotSearchedForUsers()
{
	// create an item for each result, in the block list
	QList< ContactDetails > selected = m_search->selectedResults();
	QList< ContactDetails >::Iterator it = selected.begin();
	const QList< ContactDetails >::Iterator end = selected.end();
	QPixmap icon = m_account->protocol()->groupwiseAvailable.iconFor( m_account ).pixmap ( 16 );
	for ( ; it != end; ++it )
	{
		m_dirty = true;
		m_account->client()->userDetailsManager()->addDetails( *it );
		if ( (*it).fullName.isEmpty() )
			(*it).fullName = (*it).givenName + ' ' + (*it).surname;
		new PrivacyLBI( m_privacy.denyList, icon, (*it).fullName, (*it).dn );
	}
}

void GroupWisePrivacyDialog::slotRemoveClicked()
{
	// remove any selected items from either list, except the default policy
	for( int i = m_privacy.denyList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy.denyList->isSelected( i ) )
		{
			m_dirty = true;
			Q3ListBoxItem * lbi = m_privacy.denyList->item( i );
			// can't remove the default policy
			if ( lbi == m_defaultPolicy )
				continue;
			m_privacy.denyList->removeItem( i );
		}
	}
	for( int i = m_privacy.allowList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy.allowList->isSelected( i ) )
		{
			m_dirty = true;
			Q3ListBoxItem * lbi = m_privacy.allowList->item( i );
			// can't remove the default policy
			if ( lbi == m_defaultPolicy )
				continue;
			m_privacy.allowList->removeItem( i );
		}
	}
	updateButtonState();
}

void GroupWisePrivacyDialog::slotAllowListClicked()
{
	// don't get into feedback
	disconnect( m_privacy.denyList, SIGNAL(selectionChanged()), this, SLOT(slotDenyListClicked()) );
	m_privacy.denyList->clearSelection();
	connect( m_privacy.denyList, SIGNAL(selectionChanged()), SLOT(slotDenyListClicked()) );
	bool selected = false;
	for( int i = m_privacy.allowList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy.allowList->isSelected( i ) )
		{
			selected = true;
			break;
		}
	}
	m_privacy.btnAllow->setEnabled( false );
	m_privacy.btnBlock->setEnabled( selected );
	m_privacy.btnRemove->setEnabled( selected );
}

void GroupWisePrivacyDialog::slotDenyListClicked()
{
	// don't get into feedback
	disconnect( m_privacy.allowList, SIGNAL(selectionChanged()), this, SLOT(slotAllowListClicked()) );
	m_privacy.allowList->clearSelection();
	connect( m_privacy.allowList, SIGNAL(selectionChanged()), SLOT(slotAllowListClicked()) );
	bool selected = false;
	for( int i = m_privacy.denyList->count() - 1; i >= 0 ; --i )
	{
		if ( m_privacy.denyList->isSelected( i ) )
		{
			selected = true;
			break;
		}
	}
	m_privacy.btnAllow->setEnabled( selected );
	m_privacy.btnBlock->setEnabled( false );
	m_privacy.btnRemove->setEnabled( selected );
}

void GroupWisePrivacyDialog::slotPrivacyChanged()
{
	m_privacy.denyList->clear();
	m_privacy.allowList->clear();
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
	KDialog::accept();
}

void GroupWisePrivacyDialog::slotApply()
{
	if ( m_dirty )
	{
		commitChanges();
		m_dirty = false;
		updateButtonState();
	}
#ifdef __GNUC__
#warning "kde4: port it"
#endif
	//KDialogBase::slotApply();
}

void GroupWisePrivacyDialog::commitChanges()
{
	if ( m_account->isConnected() )
	{
		bool defaultDeny = false;
		QStringList denyList;
		QStringList allowList;
		// pass on our current allow, deny and default policy to the PrivacyManager
		for( int i = 0; i < (int)m_privacy.denyList->count(); ++i )
		{
			if ( m_privacy.denyList->item( i ) == m_defaultPolicy )
				defaultDeny = true;
			else
			{
				PrivacyLBI * lbi = static_cast<PrivacyLBI *>( m_privacy.denyList->item( i ) );
				denyList.append( lbi->dn() );
			}
		}
		for( int i = 0; i < (int)m_privacy.allowList->count(); ++i )
		{
			if ( m_privacy.allowList->item( i ) == m_defaultPolicy )
				defaultDeny = false;
			else
			{
				PrivacyLBI * lbi = static_cast<PrivacyLBI *>( m_privacy.allowList->item( i ) );
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
			i18n( "You can only change privacy settings while you are logged in to the GroupWise Messenger server." ) , i18n("'%1' Not Logged In", m_account->accountId() ) );
}

#include "gwprivacydialog.moc"
