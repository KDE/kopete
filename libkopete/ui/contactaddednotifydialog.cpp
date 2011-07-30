/*
    Copyright (c) 2005      Olivier Goffart           <ogoffart@kde.org>

    Kopete    (c) 2005-2007 by the Kopete developers  <kopete-devel@kde.org>

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
#include "ui_contactaddednotifywidget.h"

namespace Kopete {

namespace UI {

struct ContactAddedNotifyDialog::Private
{
	Ui::ContactAddedNotifyWidget *widget;
	Account *account;
	QString contactId;
	QString addressbookId;
};


ContactAddedNotifyDialog::ContactAddedNotifyDialog(const QString& contactId,
		const QString& contactNick, Kopete::Account *account, const HideWidgetOptions &hide)
	: KDialog( Global::mainWidget() ), d(new Private())
{
	setCaption( i18n("Someone Has Added You") );
	setButtons( KDialog::Ok | KDialog::Cancel );
    setAttribute( Qt::WA_DeleteOnClose );

	d->widget=new Ui::ContactAddedNotifyWidget;
	QWidget* w = new QWidget(this);
	d->widget->setupUi(w);
	setMainWidget(w);

	d->account=account;
	d->contactId=contactId;
	d->widget->m_label->setText(i18n("<qt><img src=\"kopete-account-icon:%1\" /> The contact <b>%2</b> has added you to his/her contact list. (Account %3)</qt>",
			QString(QUrl::toPercentEncoding( account->protocol()->pluginId() )) + QString::fromLatin1(":")
			                 +  QString(QUrl::toPercentEncoding( account->accountId() )) ,
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
	QListIterator<Group *> it(Kopete::ContactList::self()->groups());
	while ( it.hasNext() )
	{
		Group *g = it.next();
		QString groupname = g->displayName();
		if ( g->type() == Group::Normal && !groupname.isEmpty() )
		{
			d->widget->m_groupList->addItem(groupname);
		}
	}
	d->widget->m_groupList->setEditText(QString()); //default to top-level

	connect( d->widget->widAddresseeLink, SIGNAL(addresseeChanged(KABC::Addressee)), this, SLOT(slotAddresseeSelected(KABC::Addressee)) );
	connect( d->widget->m_infoButton, SIGNAL(clicked()), this, SLOT(slotInfoClicked()) );

	connect( this, SIGNAL(okClicked()) , this , SLOT(slotFinished()));

}


ContactAddedNotifyDialog::~ContactAddedNotifyDialog()
{
	delete d->widget;
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

	metacontact->setKabcId(d->addressbookId);

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
