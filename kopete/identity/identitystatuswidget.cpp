/*
    identitystatuswidget.cpp  -  Kopete identity status configuration widget

    Copyright (c) 2007-2009 by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2007      by Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2003-2009 by the Kopete developers  <kopete-devel@kde.org>

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
#include "addaccountwizard.h"

#include <KIcon>
#include <KMenu>
#include <KActionMenu>
#include <QTimeLine>
#include <QToolTip>
#include <QCursor>
#include <QUrl>
#include <QHelpEvent>
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
#include <kopeteuiglobal.h>
#include <KCMultiDialog>

#include "kopetestatusrootaction.h"

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
: QWidget(parent), d(new Private())
{
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
	connect( d->ui.accounts, SIGNAL(customContextMenuRequested(QPoint)),
			this, SLOT(showAccountContextMenu(QPoint)) );
	connect( d->ui.accounts, SIGNAL(itemClicked(QListWidgetItem*)),
			this, SLOT(slotAccountClicked(QListWidgetItem*)) );
	connect( d->ui.accounts, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
			this, SLOT(slotAccountDoubleClicked(QListWidgetItem*)) );
	connect( d->ui.photo, SIGNAL(clicked()), 
			 this, SLOT(slotPhotoClicked()));

	connect( Kopete::AccountManager::self(), SIGNAL(accountRegistered(Kopete::Account*)),
			this, SLOT(slotAccountRegistered(Kopete::Account*)));
	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered(const Kopete::Account*)),
			this, SLOT(slotAccountUnregistered(const Kopete::Account*)));

	connect( Kopete::IdentityManager::self(), SIGNAL(identityUnregistered(const Kopete::Identity*)),
			this, SLOT(slotIdentityUnregistered(const Kopete::Identity*)));

	d->ui.accounts->viewport()->installEventFilter( this );
}

IdentityStatusWidget::~IdentityStatusWidget()
{
	delete d;
}

void IdentityStatusWidget::setIdentity(Kopete::Identity *identity)
{
	if (d->identity == identity)
		return;

	if (d->identity)
	{
		// if we were showing an identity before, disconnect the signal to handle updates
		disconnect( d->identity, SIGNAL(propertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)),
		            this, SLOT(slotIdentityPropertyChanged(Kopete::PropertyContainer*)) );
		disconnect( d->identity, SIGNAL(identityChanged(Kopete::Identity*)),
		         this, SLOT(slotIdentityChanged(Kopete::Identity*)));
	}
	d->identity = identity;
	load();

	if (d->identity)
	{
		// Handle identity changes
		connect( d->identity, SIGNAL(propertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)),
		         this, SLOT(slotIdentityPropertyChanged(Kopete::PropertyContainer*)) );
		connect( d->identity, SIGNAL(identityChanged(Kopete::Identity*)),
		         this, SLOT(slotIdentityChanged(Kopete::Identity*)));
	}
}

Kopete::Identity *IdentityStatusWidget::identity() const
{
	return d->identity;
}

void IdentityStatusWidget::setVisible( bool visible )
{
	if ( visible == isVisible() )
		return;

	// animate the widget disappearing
	d->timeline->setDirection( visible ?  QTimeLine::Forward
										: QTimeLine::Backward );
	d->timeline->start();
}

bool IdentityStatusWidget::eventFilter( QObject *watched, QEvent *event )
{
	if( event->type() == QEvent::ToolTip && watched == d->ui.accounts->viewport() )
	{
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
		QListWidgetItem* item = d->ui.accounts->itemAt( helpEvent->pos() );
		if ( item )
		{
			const Kopete::Account * account = d->accountHash.value( item, 0 );
			if ( account )
				item->setToolTip( account->myself()->toolTip() );
		}
	}
	return QWidget::eventFilter( watched, event );
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

	setFixedHeight( sizeHint().height() * amount );

	if (!isVisible())
		QWidget::setVisible( true );
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
		d->photoPath.clear();
		d->ui.photo->setIcon( KIcon( "user-identity" ) );
	}

	d->ui.identityName->setText(d->identity->label());

	foreach(Kopete::Account *a, d->identity->accounts()) {
		addAccountItem( a );
	}
	if ( d->identity->accounts().isEmpty() ) {
		new QListWidgetItem( KIcon("configure" ), i18nc("Button to open account configuration widget", "Click to add an account" ), d->ui.accounts );
	}
	resizeAccountListWidget();
}

void IdentityStatusWidget::slotAccountRegistered( Kopete::Account *account )
{
	if (account && account->identity() == d->identity && d->accountHash.isEmpty())
	{
		// Remove "Add account" placeholder
		d->ui.accounts->clear();
	}

	addAccountItem( account );
	resizeAccountListWidget();
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

	if ( d->identity && d->identity->accounts().isEmpty() ) {
		new QListWidgetItem( KIcon("configure" ), i18nc("Button to open account configuration widget", "Click to add an account" ), d->ui.accounts );
	}
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
			SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
			this, SLOT(slotAccountStatusIconChanged(Kopete::Contact*)) );

	QListWidgetItem * item = new QListWidgetItem( account->accountIcon(), account->accountLabel(), d->ui.accounts );
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

	item->setIcon ( status.iconFor( contact->account() ) );
}

void IdentityStatusWidget::showAccountContextMenu( const QPoint & point )
{
	QListWidgetItem * item = d->ui.accounts->itemAt( point );
	if ( item && !d->accountHash.isEmpty() ) {
		Kopete::Account * account = d->accountHash[ item ];
		if ( account ) {
			KActionMenu *actionMenu = new KActionMenu( account->accountId(), account );

			if ( !account->hasCustomStatusMenu() )
				Kopete::StatusRootAction::createAccountStatusActions( account, actionMenu );

			account->fillActionMenu( actionMenu );

			actionMenu->menu()->exec( d->ui.accounts->mapToGlobal( point ) );
			delete actionMenu;
		}
	}
}

void IdentityStatusWidget::slotAccountClicked( QListWidgetItem * item )
{
	Q_UNUSED( item );

	if ( d->identity && d->identity->accounts().isEmpty() )
	{
		Q_ASSERT(d->accountHash.isEmpty());
		// "Add an account" item
		AddAccountWizard *addwizard = new AddAccountWizard( this, true );
		addwizard->setIdentity(identity());
		addwizard->show();
	}
}

void IdentityStatusWidget::slotAccountDoubleClicked( QListWidgetItem * item )
{
	//Account toggles connect/disconnect at double click!
	if ( item && !d->accountHash.isEmpty() )
	{
		Kopete::Account * account = d->accountHash[ item ];
		if ( account ) {
			if ( account->myself()->onlineStatus().status() == Kopete::OnlineStatus::Offline )
			{
				Kopete::OnlineStatus s(Kopete::OnlineStatus::Online);
				account->connect( s );
			} else {
				account->disconnect();
			}
		}
	}
}

void IdentityStatusWidget::slotPhotoClicked()
{
	if ( !d->identity )
		return;

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
	int itemHeight = d->ui.accounts->sizeHintForRow( 0 );
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

void IdentityStatusWidget::slotIdentityPropertyChanged(Kopete::PropertyContainer *container)
{
	Kopete::Identity *identity = dynamic_cast<Kopete::Identity*>(container);

	slotIdentityChanged(identity);
}

void IdentityStatusWidget::slotIdentityChanged(Kopete::Identity *identity)
{
	// if it is not the identity currently being shown, there is no need to update
	if (identity != d->identity)
		return;

	load();
}

#include "identitystatuswidget.moc"
// vim: set noet ts=4 sts=4 sw=4:

