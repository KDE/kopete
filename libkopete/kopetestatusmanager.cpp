/*
    kopetestatusmanager.cpp - Kopete Status Manager

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "kopetestatusmanager.h"

#include <QApplication>
#include <QtCore/QFile>
#include <QtXml/QDomElement>
#include <QtCore/QTimer>

#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kdialog.h>
#include <kmessagebox.h>

#include "kopeteuiglobal.h"
#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetecontact.h"
#include "kopeteonlinestatusmanager.h"
#include "kopetebehaviorsettings.h"
#include "kopetestatusitems.h"
#include "kopeteidletimer.h"

namespace Kopete {

StatusManager *StatusManager::instance = 0L;

class StatusManager::Private
{
public:
	Status::StatusGroup *root;
	QHash<QString, Status::StatusItem *> uidHash;

	int awayTimeout;
	bool goAvailable;
	bool useCustomStatus;

	uint globalStatusCategory;
	Kopete::StatusMessage globalStatusMessage;
	Kopete::StatusMessage customStatusMessage;
	
	bool away;
	QList<Kopete::Account*> autoAwayAccounts;

	Kopete::IdleTimer* idleTimer;
};

StatusManager::StatusManager()
	: QObject( qApp ), d( new Private )
{
	d->away = false;
	d->root = 0;
	d->idleTimer = 0;
	loadXML();

	loadSettings();
	loadBehaviorSettings();
	connect( Kopete::BehaviorSettings::self(), SIGNAL(configChanged()),
	         this, SLOT(loadBehaviorSettings()) );

	connect( Kopete::AccountManager::self(), SIGNAL(accountUnregistered(const Kopete::Account*)),
	         this, SLOT(accountUnregistered(const Kopete::Account*)));

	connect( Kopete::AccountManager::self(), SIGNAL(accountOnlineStatusChanged(Kopete::Account*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
		 this, SLOT(checkIdleTimer()));

}

StatusManager::~StatusManager()
{
	instance = 0L;

	delete d->idleTimer;

	delete d->root;
	delete d;
}

void StatusManager::saveXML()
{
	QString filename = KStandardDirs::locateLocal( "data", QLatin1String( "kopete/statuses.xml" ) );
	KSaveFile file(filename);
	if( file.open() )
	{
		QTextStream stream (&file);
		stream.setCodec(QTextCodec::codecForName("UTF-8"));

		QDomDocument doc( QString::fromLatin1( "kopete-statuses" ) );
		doc.appendChild( StatusManager::storeStatusItem( d->root ) );
		doc.save( stream, 4 );

		file.close();
	}
}

void StatusManager::loadXML()
{
	delete d->root;

	d->uidHash.clear();
	d->root = 0;

	const QString filename = KStandardDirs::locateLocal( "data", QLatin1String( "kopete/statuses.xml" ) );

	QDomDocument doc;
	QFile file( filename );
	if ( file.open( QIODevice::ReadOnly ) )
	{
		if ( doc.setContent( &file ) )
		{
			Kopete::Status::StatusItem* rootItem = StatusManager::parseStatusItem( doc.documentElement() );
			if ( rootItem )
			{
				if ( rootItem->isGroup() )
					d->root = qobject_cast<Status::StatusGroup *>(rootItem);
				else
					delete rootItem;
			}
		}
		file.close();
	}
	
	if ( !d->root )
	{
		d->root = defaultStatuses();
		saveXML();
	}

	updateUidHash( d->root );
}

StatusManager *StatusManager::self()
{
	if ( !instance )
		instance = new StatusManager;

	return instance;
}

void StatusManager::setRootGroup( Kopete::Status::StatusGroup *rootGroup )
{
	if ( !rootGroup || rootGroup == d->root )
		return;

	delete d->root;

	d->uidHash.clear();
	d->root = rootGroup;
	updateUidHash( d->root );
	
	emit changed();
}

Status::StatusGroup *StatusManager::getRootGroup() const
{
	return d->root;
}

Kopete::Status::StatusGroup *StatusManager::copyRootGroup() const
{
	return qobject_cast<Kopete::Status::StatusGroup *>(d->root->copy());
}

const Status::StatusItem *StatusManager::itemForUid( const QString &uid ) const
{
	return d->uidHash.value( uid, 0 );
}

QDomElement StatusManager::storeStatusItem( const Status::StatusItem *item )
{
	QDomDocument statusDoc;
	QString rootName = ( item->isGroup() ) ? QLatin1String( "group" ) : QLatin1String( "status" );
	statusDoc.appendChild( statusDoc.createElement( rootName ) );
	statusDoc.documentElement().setAttribute( "uid", item->uid() );
	statusDoc.documentElement().setAttribute( "category", item->category() );

	QDomElement title = statusDoc.createElement( QLatin1String( "title" ) );
	title.appendChild( statusDoc.createTextNode( item->title() ) );
	statusDoc.documentElement().appendChild( title );

	if ( item->isGroup() )
	{
		const Status::StatusGroup *group = qobject_cast<const Kopete::Status::StatusGroup*>( item );
		const QList<Status::StatusItem *> childs = group->childList();
		foreach ( Status::StatusItem *child , childs )
			statusDoc.documentElement().appendChild( storeStatusItem( child ) );
	}
	else
	{
		const Status::Status *status = qobject_cast<const Kopete::Status::Status*>( item );
		QDomElement message = statusDoc.createElement( QLatin1String( "message" ) );
		message.appendChild( statusDoc.createTextNode( status->message() ) );
		statusDoc.documentElement().appendChild( message );
	}

	return statusDoc.documentElement();
}

Status::StatusItem *StatusManager::parseStatusItem( QDomElement element )
{
	if ( element.isNull() )
		return 0;
		
	if ( element.tagName() == QString::fromUtf8( "group" ) )
	{
		Status::StatusGroup* group = new Status::StatusGroup( element.attribute( "uid" ) );
		group->setCategory( (OnlineStatusManager::Category)element.attribute( "category", "0" ).toInt() );

		QDomNode childNode = element.firstChild();
		while ( !childNode.isNull() )
		{
			QDomElement childElement = childNode.toElement();
			if ( childElement.tagName() == QLatin1String( "title" ) )
				group->setTitle( childElement.text() );
			else if ( childElement.tagName() == QLatin1String( "group" ) || childElement.tagName() == QLatin1String( "status" ) )
			{
				Status::StatusItem *item = StatusManager::parseStatusItem( childElement );
				if ( item )
					group->appendChild( item );
			}
			childNode = childNode.nextSibling();
		}
		return group;
	}
	else if ( element.tagName() == QString::fromUtf8( "status" ) )
	{
		Status::Status* status = new Status::Status( element.attribute( "uid" ) );
		status->setCategory( (OnlineStatusManager::Category)element.attribute( "category", "0" ).toInt() );
		
		QDomNode childNode = element.firstChild();
		while ( !childNode.isNull() )
		{
			QDomElement childElement = childNode.toElement();
			if ( childElement.tagName() == QLatin1String( "title" ) )
				status->setTitle( childElement.text() );
			else if ( childElement.tagName() == QLatin1String( "message" ) )
				status->setMessage( childElement.text() );

			childNode = childNode.nextSibling();
		}
		return status;
	}

	return 0;
}

void StatusManager::updateUidHash( Status::StatusItem *item )
{
	if ( item->isGroup() )
	{
		Kopete::Status::StatusGroup *group = qobject_cast<Kopete::Status::StatusGroup*>(item);
		QList<Kopete::Status::StatusItem*> childs = group->childList();
		foreach( Kopete::Status::StatusItem* child, childs )
			updateUidHash( child );
	}
	else
	{
		d->uidHash[item->uid()] = item;
	}
}

Status::StatusGroup *StatusManager::defaultStatuses() const
{
	Status::StatusGroup* group = new Status::StatusGroup();
	
	Status::Status* status = new Status::Status();
	status->setTitle( i18n( "Online" ) );
	status->setCategory( OnlineStatusManager::Online );
	group->appendChild( status );

	status = new Status::Status();
	status->setTitle( i18n( "Away" ) );
	status->setMessage( i18n( "I am gone right now, but I will be back later" ) );
	status->setCategory( OnlineStatusManager::Away );
	group->appendChild( status );

	status = new Status::Status();
	status->setTitle( i18n( "Busy" ) );
	status->setMessage( i18n( "Sorry, I am busy right now" ) );
	status->setCategory( OnlineStatusManager::Busy );
	group->appendChild( status );

	status = new Status::Status();
	status->setTitle( i18n( "Invisible" ) );
	status->setCategory( OnlineStatusManager::Invisible );
	group->appendChild( status );

	status = new Status::Status();
	status->setTitle( i18n( "Offline" ) );
	status->setCategory( OnlineStatusManager::Offline );
	group->appendChild( status );

	return group;
}

void StatusManager::setGlobalStatus( uint category, const Kopete::StatusMessage &statusMessage )
{
	d->globalStatusCategory = category;
	d->globalStatusMessage = statusMessage;

	KConfigGroup config( KGlobal::config(), "Status Manager" );
	config.writeEntry( "GlobalStatusCategory", d->globalStatusCategory );
	config.writeEntry( "GlobalStatusTitle", d->globalStatusMessage.title() );
	config.writeEntry( "GlobalStatusMessage", d->globalStatusMessage.message() );
	config.sync();

	emit globalStatusChanged();
}

void StatusManager::setGlobalStatusMessage( const Kopete::StatusMessage &statusMessage )
{
	d->globalStatusMessage = statusMessage;
	
	KConfigGroup config( KGlobal::config(), "Status Manager" );
	config.writeEntry( "GlobalStatusTitle", d->globalStatusMessage.title() );
	config.writeEntry( "GlobalStatusMessage", d->globalStatusMessage.message() );
	config.sync();

	// Iterate each connected account, updating its status message but keeping the
	// same onlinestatus.
	QList<Kopete::Account*> accountList = Kopete::AccountManager::self()->accounts();
	foreach ( Kopete::Account *account, accountList )
	{
		Kopete::Contact *self = account->myself();
		bool isInvisible = self && self->onlineStatus().status() == Kopete::OnlineStatus::Invisible;
		if ( self && account->isConnected() && !isInvisible )
		{
			account->setOnlineStatus ( self->onlineStatus(), statusMessage );
		}
	}

	emit globalStatusChanged();
}

Kopete::StatusMessage StatusManager::globalStatusMessage() const
{
	return d->globalStatusMessage;
}

uint StatusManager::globalStatusCategory() const
{
	return d->globalStatusCategory;
}

void StatusManager::askAndSetActive()
{
	kDebug(14010) << "Found Activity. Confirming if we should go active";

	// First Create a Dialog
	KDialog *dialog = new KDialog(Kopete::UI::Global::mainWidget());
	dialog->setCaption(i18n("Going Online - Kopete"));
	dialog->setButtons(KDialog::Yes | KDialog::No);
	dialog->setDefaultButton(KDialog::Yes);
	dialog->setEscapeButton(KDialog::No);
	dialog->setAttribute(Qt::WA_DeleteOnClose, true);

	// Set the Text in the Dialog
	KMessageBox::createKMessageBox(dialog, QMessageBox::Question,
		i18n("Do You Want to Change Status to Available?"),
		QStringList(), QString(), NULL, KMessageBox::NoExec);

	// If yes is clicked, go online
	connect(dialog, SIGNAL(yesClicked()), this, SLOT(setActive()));

	// If the user does not click something by the time we go away, kill the dialog
	QTimer::singleShot(Kopete::BehaviorSettings::self()->autoAwayTimeout() * 1000, dialog, SLOT(close()));

	// Show the Dialog
	dialog->show();
}

void StatusManager::setActive()
{
	kDebug(14010) << "Found activity on desktop, setting accounts online";
	if( d->away )
	{
		d->away = false;
		if ( d->goAvailable )
		{
			QList<Kopete::Account*>::iterator it, itEnd = d->autoAwayAccounts.end();
			for( it = d->autoAwayAccounts.begin(); it != itEnd; ++it )
			{
				if( (*it)->isConnected() && (*it)->isAway() )
				{
					(*it)->setOnlineStatus( Kopete::OnlineStatusManager::self()->onlineStatus( (*it)->protocol(),
						Kopete::OnlineStatusManager::Online ), globalStatusMessage(), Kopete::Account::KeepSpecialFlags );
				}
			}
			d->autoAwayAccounts.clear();
		}
	}
}

void StatusManager::setAutoAway()
{
	kDebug(14010) << "Going AutoAway!";
	if ( !d->away )
	{
		d->away = true;
		
		// Set all accounts that are not away already to away.
		// We remember them so later we only set the accounts to
		// available that we set to away (and not the user).
		QList<Kopete::Account *> accountList = Kopete::AccountManager::self()->accounts();

		QList<Kopete::Account*>::iterator it, itEnd = accountList.end();
		for( it = accountList.begin(); it != itEnd; ++it )
		{
			if( (*it)->myself()->onlineStatus().status() == Kopete::OnlineStatus::Online )
			{
				d->autoAwayAccounts.append( (*it) );
				
				if( d->useCustomStatus )
				{
					// Display a specific away message
					(*it)->setOnlineStatus( Kopete::OnlineStatusManager::self()->onlineStatus( (*it)->protocol(),
						Kopete::OnlineStatusManager::Idle ), d->customStatusMessage, Kopete::Account::KeepSpecialFlags );
				}
				else
				{
					// Display the last global away message used
					(*it)->setOnlineStatus( Kopete::OnlineStatusManager::self()->onlineStatus( (*it)->protocol(),
						Kopete::OnlineStatusManager::Idle ), d->globalStatusMessage, Kopete::Account::KeepSpecialFlags );
				}
			}
		}
	}
}

bool StatusManager::autoAway()
{
	return d->away;
}

bool StatusManager::globalAway()
{
	return ( d->globalStatusCategory == OnlineStatusManager::Away ||
	         d->globalStatusCategory == OnlineStatusManager::ExtendedAway ||
	         d->globalStatusCategory == OnlineStatusManager::Busy ||
	         d->globalStatusCategory == OnlineStatusManager::Offline );
}

void StatusManager::accountUnregistered( const Kopete::Account *account )
{
	d->autoAwayAccounts.removeAll( const_cast<Kopete::Account *>(account) );
}

void StatusManager::checkIdleTimer()
{
	// TODO: should we check for d->autoAwayAccounts to see whether to stop the timer?
	Kopete::IdleTimer* idleTimer = Kopete::IdleTimer::self();
	idleTimer->unregisterTimeout( this );

	if(Kopete::AccountManager::self()->isAnyAccountConnected()) {
		if ( Kopete::BehaviorSettings::self()->useAutoAway() ) {
			if (Kopete::BehaviorSettings::self()->autoAwayAskAvailable())
				idleTimer->registerTimeout( d->awayTimeout, this, SLOT(askAndSetActive()), SLOT(setAutoAway()) );
			else
				idleTimer->registerTimeout( d->awayTimeout, this, SLOT(setActive()), SLOT(setAutoAway()) );
		}
	}
}

void StatusManager::loadSettings()
{
	KConfigGroup config( KGlobal::config(), "Status Manager" );
	d->globalStatusCategory = config.readEntry( "GlobalStatusCategory", 0 );

	Kopete::StatusMessage statusMessage;
	statusMessage.setTitle( config.readEntry( "GlobalStatusTitle", QString() ) );
	statusMessage.setMessage( config.readEntry( "GlobalStatusMessage", QString() ) );
	d->globalStatusMessage = statusMessage;
}

void StatusManager::loadBehaviorSettings()
{
	d->awayTimeout = Kopete::BehaviorSettings::self()->autoAwayTimeout();
	d->goAvailable = Kopete::BehaviorSettings::self()->autoAwayGoAvailable();
	d->useCustomStatus = Kopete::BehaviorSettings::self()->useCustomAwayMessage();
	
	Kopete::StatusMessage customStatusMessage;
	customStatusMessage.setTitle( Kopete::BehaviorSettings::self()->autoAwayCustomTitle() );
	customStatusMessage.setMessage( Kopete::BehaviorSettings::self()->autoAwayCustomMessage() );
	d->customStatusMessage = customStatusMessage;

	checkIdleTimer();
}

}

#include "kopetestatusmanager.moc"
