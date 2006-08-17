/*
    Copyright (c) 2005      Olivier Goffart           <ogoffart@ kde.org>

    Kopete    (c) 2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "contactaddednotifydialog.h"


#include <qvbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qstylesheet.h>
#include <qapplication.h>

#include <klocale.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kiconloader.h>

#include <kabc/addressee.h>

#include "kopetegroup.h"
#include "kopeteaccount.h"
#include "kopeteuiglobal.h"
#include "kopeteprotocol.h"
#include "kopetecontactlist.h"
#include "kopetemetacontact.h"
#include "addressbooklinkwidget.h"
#include "addressbookselectordialog.h"


#include "contactaddednotifywidget.h"

namespace Kopete {

namespace UI {

struct ContactAddedNotifyDialog::Private
{
	ContactAddedNotifyWidget *widget;
	Account *account;
	QString contactId;
	QString addressbookId;
};


ContactAddedNotifyDialog::ContactAddedNotifyDialog(const QString& contactId,
		const QString& contactNick, Kopete::Account *account, uint hide)
	: KDialogBase( Global::mainWidget(), "ContactAddedNotify", /*modal=*/false,
				   i18n("Someone Has Added You"), Ok|Cancel    )
{

	setWFlags(WDestructiveClose |  getWFlags() );
	
	d=new Private;
	d->widget=new ContactAddedNotifyWidget(this);
	setMainWidget(d->widget);
	
	d->account=account;
	d->contactId=contactId;
	d->widget->m_label->setText(i18n("<qt><img src=\"kopete-account-icon:%1\" /> The contact <b>%2</b> has added you to his/her contactlist. (Account %3)</qt>")
			.arg( KURL::encode_string( account->protocol()->pluginId() ) + QString::fromLatin1(":")
			                  + KURL::encode_string( account->accountId() ) ,
				  contactNick.isEmpty() ? contactId : contactNick + QString::fromLatin1(" < ") + contactId + QString::fromLatin1(" >")  ,
				  account->accountLabel()  	)   );
	if( hide & InfoButton)
		d->widget->m_infoButton->hide() ;
	if( hide & AuthorizeCheckBox )
	{
		d->widget->m_authorizeCb->hide();
		d->widget->m_authorizeCb->setChecked(false);
	}
	if( hide & AddCheckBox )
	{
		d->widget->m_addCb->hide();
		d->widget->m_addCb->setChecked(false);
	}
	if( hide & AddGroupBox )
		d->widget->m_contactInfoBox->hide();

	// Populate the groups list
	Kopete::GroupList groups=Kopete::ContactList::self()->groups();
	for( Kopete::Group *it = groups.first(); it; it = groups.next() )
	{
		QString groupname = it->displayName();
		if ( it->type() == Group::Normal && !groupname.isEmpty() )
		{
			d->widget->m_groupList->insertItem(groupname);
		}
	}
	d->widget->m_groupList->setCurrentText(QString::null); //default to top-level

	connect( d->widget->widAddresseeLink, SIGNAL( addresseeChanged( const KABC::Addressee& ) ), this, SLOT( slotAddresseeSelected( const KABC::Addressee& ) ) );
	connect( d->widget->m_infoButton, SIGNAL( clicked() ), this, SLOT( slotInfoClicked() ) );

	connect( this, SIGNAL(okClicked()) , this , SLOT(slotFinished()));

}


ContactAddedNotifyDialog::~ContactAddedNotifyDialog()
{
	delete d;
}

bool ContactAddedNotifyDialog::added() const
{
	return d->widget->m_addCb->isChecked();
}

bool ContactAddedNotifyDialog::authorized() const
{
	return d->widget->m_authorizeCb->isChecked();
}

QString ContactAddedNotifyDialog::displayName() const
{
	return d->widget->m_displayNameEdit->text();
}

Group *ContactAddedNotifyDialog::group() const
{
	QString grpName=d->widget->m_groupList->currentText();
	if(grpName.isEmpty())
		return Group::topLevel();

	return ContactList::self()->findGroup( grpName  );
}

MetaContact *ContactAddedNotifyDialog::addContact() const
{
	if(!added() || !d->account)
		return 0L;

	MetaContact *metacontact=d->account->addContact(d->contactId, displayName(), group());
	if(!metacontact)
		return 0L;

	metacontact->setMetaContactId(d->addressbookId);

	return metacontact;
}

void ContactAddedNotifyDialog::slotAddresseeSelected( const KABC::Addressee & addr )
{
	if ( !addr.isEmpty() )
	{
		d->addressbookId = addr.uid();
	}
}

void ContactAddedNotifyDialog::slotInfoClicked()
{
	emit infoClicked(d->contactId);
}

void ContactAddedNotifyDialog::slotFinished()
{
	emit applyClicked(d->contactId);
}



} // namespace UI
} // namespace Kopete
#include "contactaddednotifydialog.moc"
