/*
    identitystatuswidget.cpp  -  Kopete identity status configuration widget

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2007      by Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2003-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "identitystatuswidget.h"
#include "ui_identitystatusbase.h"

#include <KIcon>
#include <KMenu>
#include <QTimeLine>
#include <QToolTip>
#include <QCursor>
#include <QUrl>
#include <KColorScheme>
#include <kopeteidentity.h>
#include <kopeteidentitymanager.h>
#include <kopeteaccount.h>
#include <kopeteaccountmanager.h>
#include <kopetecontact.h>
#include <kopeteprotocol.h>
#include <kopetestdaction.h>
#include <avatardialog.h>
#include <KDebug>

class IdentityStatusWidget::Private
{
public:
	Kopete::Identity *identity;
	Ui::IdentityStatusBase ui;
	QTimeLine *timeline;
	QString photoPath;
	QHash<QListWidgetItem *,Kopete::Account *> accountHash;
	bool dirty;
};

IdentityStatusWidget::IdentityStatusWidget(Kopete::Identity *identity, QWidget *parent)
: QWidget(parent)
{
	d = new Private();
	d->identity = 0;
	
	// animation for showing/hiding
	d->timeline = new QTimeLine( 150, this );
	d->timeline->setCurveShape( QTimeLine::EaseInOutCurve );
	connect( d->timeline, SIGNAL(valueChanged(qreal)),
			 this, SLOT(slotAnimate(qreal)) );

	d->ui.setupUi(this);
	d->ui.accounts->setContextMenuPolicy( Qt::CustomContextMenu );
	QWidget::setVisible( false );

	setIdentity(identity);

	// user input signals
	connect( d->ui.accounts, SIGNAL(customContextMenuRequested(const QPoint &)),
			this, SLOT(showAccountContextMenu(const QPoint &)) );
	connect( d->ui.photo, SIGNAL(clicked()), 
			 this, SLOT(slotPhotoClicked()));

	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account*)),
			this, SLOT(slotAccountRegistered(Kopete::Account*)));
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered(const Kopete::Account*)),
			this, SLOT(slotAccountUnregistered(const Kopete::Account*)));

	connect( Kopete::IdentityManager::self(), SIGNAL(identityUnregistered(const Kopete::Identity*)),
			this, SLOT(slotIdentityUnregistered(const Kopete::Identity*)));
}

IdentityStatusWidget::~IdentityStatusWidget()
{
	delete d;
}

void IdentityStatusWidget::setIdentity(Kopete::Identity *identity)
{
	d->identity = identity;
	load();

	if (d->identity)
	{
		//TODO: Handle identity changes
		//connect( d->identity, SIGNAL(identityChanged(Kopete::Identity*)), this, SLOT(slotUpdateAccountStatus()));
		;
	}
}

Kopete::Identity *IdentityStatusWidget::identity() const
{
	return d->identity;
}

void IdentityStatusWidget::setVisible( bool visible )
{
	// animate the widget disappearing
	d->timeline->setDirection( visible ?  QTimeLine::Forward
										: QTimeLine::Backward );
	d->timeline->start();
}

void IdentityStatusWidget::slotAnimate(qreal amount)
{
	if (amount == 0)
	{
		QWidget::setVisible( false );
		return;
	}
	
	if (amount == 1.0)
	{
		layout()->setSizeConstraint( QLayout::SetDefaultConstraint );
		setFixedHeight( sizeHint().height() );
		return;
	}

	if (!isVisible())
		QWidget::setVisible( true );

	setFixedHeight( sizeHint().height() * amount );
}

void IdentityStatusWidget::load()
{
	// clear
	d->ui.accounts->clear();
	d->accountHash.clear();

	if (!d->identity)
		return;

	Kopete::Global::Properties *props = Kopete::Global::Properties::self();

	// photo
	if (d->identity->hasProperty(props->photo().key()))
	{
		d->photoPath = d->identity->property(props->photo()).value().toString();
		d->ui.photo->setIcon( QIcon( d->photoPath ) );
	} else {
		d->ui.photo->setIcon( KIcon( "user" ) );
    }

	d->ui.identityName->setText(d->identity->label());

	foreach(Kopete::Account *a, d->identity->accounts()) {
		addAccountItem( a );
	}
	if ( d->identity->accounts().isEmpty() ) {
		new QListWidgetItem( KIcon("configure" ), i18nc("Label to tell the user no accounts existed", "No accounts configured" ), d->ui.accounts );
	}
	resizeAccountListWidget();
}

void IdentityStatusWidget::slotAccountRegistered( Kopete::Account *account )
{
	// Kopete::Identity is poorly integrated as of Kopete 0.40.0.  As a result Kopete::Accounts are
	// always created as belonging to the default Identity, which means that they are added to the
	// wrong identity in the IdentityStatus widget.  To fix this properly it would be necessary to 
	// initialise the identities before the accounts on startup and pass the identity into the
	// account constructor.  I'm not making changes like that for KDE 4.0 so just hide this widget
	// now.  See also slotAccountUnregistered()

	// the following code can be reenabled when Accounts have the right Identity on
	// accountRegistered()
#if 0
	addAccountItem( account );
	resizeAccountListWidget();
#else
	Q_UNUSED( account )
	QWidget::setVisible( false );
#endif
}

void IdentityStatusWidget::slotAccountUnregistered( const Kopete::Account *account )
{
	QListWidgetItem * item = 0;

	QHashIterator<QListWidgetItem*, Kopete::Account *> i( d->accountHash );
	while ( i.hasNext() ) {
		i.next();
		Kopete::Account * candidate = i.value();
		if ( candidate == account ) {
			item = i.key();
		}
	}
	if( !item )
		return;
	d->ui.accounts->takeItem( d->ui.accounts->row( item ) );
	d->accountHash.remove( item );
	delete item;
	resizeAccountListWidget();
}

void IdentityStatusWidget::addAccountItem( Kopete::Account *account )
{
	// debug to diagnose if the account was created with the right identity.  see comment in
	// slotAccountRegistered
	//kDebug() << "Adding Account item for identity: " << ( account->identity() ? account->identity()->label() : "" ) << ", showing identity " << ( d->identity ? d->identity->label() : "" )<< " in widget.";
	if ( !account || ( account->identity() != d->identity ) ) {
		return;
	}

	connect( account->myself(),
			SIGNAL(onlineStatusChanged(Kopete::Contact *, const Kopete::OnlineStatus &, const Kopete::OnlineStatus &)),
			this, SLOT(slotAccountStatusIconChanged(Kopete::Contact *)) );

	QListWidgetItem * item = new QListWidgetItem( account->accountIcon(), account->accountLabel(), d->ui.accounts );
	item->setToolTip( account->myself()->toolTip() );
	d->accountHash.insert( item, account );

	slotAccountStatusIconChanged( account->myself() );
}

void IdentityStatusWidget::slotAccountStatusIconChanged( Kopete::Contact *contact )
{
	///kdDebug( 14000 ) << k_funcinfo << contact->property( Kopete::Global::Properties::self()->awayMessage() ).value() << endl;
	Kopete::OnlineStatus status = contact->onlineStatus();
	QListWidgetItem * item = 0;
	QHashIterator<QListWidgetItem*, Kopete::Account *> i( d->accountHash );
	while ( i.hasNext() ) {
		i.next();
		Kopete::Account * candidate = i.value();
		if ( candidate == contact->account() ) {
			item = i.key();
		}
	}
	if( !item )
		return;

	QPixmap pm = status.iconFor( contact->account() );
	if( pm.isNull() )
		item->setIcon( KIconLoader::unknown() );
	else
		item->setIcon( QIcon( pm ) );

	item->setToolTip( contact->toolTip() );
}

void IdentityStatusWidget::showAccountContextMenu( const QPoint & point )
{
	QListWidgetItem * item = d->ui.accounts->itemAt( point );
	if ( item && !d->accountHash.isEmpty() ) {
		Kopete::Account * account = d->accountHash[ item ];
		if ( account ) {
			KActionMenu *actionMenu = account->actionMenu();
			actionMenu->menu()->exec( d->ui.accounts->mapToGlobal( point ) );
			delete actionMenu;
		}
	}
}

void IdentityStatusWidget::slotPhotoClicked()
{
	bool ok, changed = false;
	const QString photoPath = Kopete::UI::AvatarDialog::getAvatar( this, d->photoPath, &ok);
	if ( ok ) {
		Kopete::Global::Properties *props = Kopete::Global::Properties::self();
		if ( photoPath.isEmpty() ) {
			d->identity->removeProperty( props->photo() );
			d->photoPath.clear();
			changed = true;
		}
		else if ( photoPath != d->photoPath ) {
			d->identity->setProperty(props->photo(), photoPath);
			d->photoPath = photoPath;
			changed = true;
		}

		if ( changed ) {
			load();
		}
	}
}

void IdentityStatusWidget::resizeAccountListWidget()
{
	int frameWidth = d->ui.accounts->frameWidth();
	int itemHeight = d->ui.accounts->itemDelegate()->sizeHint(
			QStyleOptionViewItem(), d->ui.accounts->model()->index( 0, 0 ) ).height();
	int itemCount = d->ui.accounts->count();
	d->ui.accounts->setFixedHeight( 2 * frameWidth
			+ itemHeight * ( itemCount ? itemCount : 1 ) );
	layout()->invalidate();
	setFixedHeight( sizeHint().height() );
	//adjustSize();
}

void IdentityStatusWidget::slotIdentityUnregistered( const Kopete::Identity* identity )
{
	if ( identity == d->identity )
	{
		disconnect( identity );
		hide();
		setIdentity( Kopete::IdentityManager::self()->defaultIdentity() );
	}
}

#include "identitystatuswidget.moc"
// vim: set noet ts=4 sts=4 sw=4:

