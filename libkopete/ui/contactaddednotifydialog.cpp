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
	: KDialogBase( Plain,  WDestructiveClose |  WStyle_DialogBorder,
				   Global::mainWidget(), "ContactAddedNotify", /*modal=*/false,
				   i18n("Somone has added you - Kopete"), Ok|Cancel    )
{
	d=new Private;
	d->widget=new ContactAddedNotifyWidget(this);
	d->account=account;
	d->contactId=contactId;
	d->widget->m_label->setText(i18n("<qt><img src=\"kopete-account-icon:%1\" / >The contact <b>%2</b> has added you in his contactlist. (Account %3)</qt>")
			.arg( account->protocol()->displayName() + QString::fromLatin1(":")+ account->accountLabel() ,
				  contactNick.isEmpty() ? contactId : contactNick + QString::fromLatin1(" < ") + contactId + QString::fromLatin1(" >")  ,
				  account->accountLabel()  	)   );
	if( hide & InfoButton)
		d->widget->m_infoButton->hide() ;
	if( hide & AuthorizeCheckBox )
		d->widget->m_authorizeCb->hide();
	if( hide & AddCheckBox )
		d->widget->m_addCb->hide();
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

	d->widget->btnClear->setIconSet( SmallIconSet( QApplication::reverseLayout() ?
			 QString::fromLatin1 ( "locationbar_erase" ) : QString::fromLatin1 ("clear_left") ) );
	connect( d->widget->btnClear, SIGNAL( clicked() ), this, SLOT( slotClearAddresseeClicked() ) );
	connect( d->widget->btnSelectAddressee, SIGNAL( clicked() ), this, SLOT( slotSelectAddresseeClicked() ) );
	connect( d->widget->m_infoButton, SIGNAL( clicked() ), this, SLOT( slotInfoClicked() ) );

	connect( this, SIGNAL(finished()) , this , SLOT(slotFinished()));

	setMainWidget(d->widget);
}


ContactAddedNotifyDialog::~ContactAddedNotifyDialog()
{
	delete d;
}

bool ContactAddedNotifyDialog::added()
{
	return d->widget->m_addCb->isChecked();
}

bool ContactAddedNotifyDialog::authorized()
{
	return d->widget->m_authorizeCb->isChecked();
}

QString ContactAddedNotifyDialog::displayName()
{
	return d->widget->m_displayNameEdit->text();
}

Group *ContactAddedNotifyDialog::group()
{
	QString grpName=d->widget->m_groupList->currentText();
	if(grpName.isEmpty())
		return Group::topLevel();

	return ContactList::self()->findGroup( grpName  );
}

MetaContact *ContactAddedNotifyDialog::addContact()
{
	if(!added() || !d->account)
		return 0L;

	MetaContact *metacontact=d->account->addContact(d->contactId, displayName(), group());
	if(!metacontact)
		return 0L;

	metacontact->setMetaContactId(d->addressbookId);

	return metacontact;
}





void ContactAddedNotifyDialog::slotSelectAddresseeClicked()
{
	KABC::Addressee a = Kopete::UI::AddressBookSelectorDialog::getAddressee( i18n("Addressbook association"), i18n("Choose the person who '%1' is.").arg(d->contactId ), d->addressbookId , this);

	if ( !a.isEmpty() )
	{
		d->widget->edtAddressee->setText( a.realName() );
		// set/update the MC's addressee uin field
		d->addressbookId = a.uid();
	}
}


void ContactAddedNotifyDialog::slotClearAddresseeClicked()
{
	d->widget->edtAddressee->setText( QString::null );
	d->addressbookId=QString::null;
}

void ContactAddedNotifyDialog::slotInfoClicked()
{
	emit infoClicked(d->contactId);
}

void ContactAddedNotifyDialog::slotFinished()
{
	if(result() == Accepted)
		emit applyClicked(d->contactId);
}



} // namespace UI
} // namespace Kopete
#include "contactaddednotifydialog.moc"
