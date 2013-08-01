/*
    kopetecontact.cpp - Kopete Contact

    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @ kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetecontact.h"

#include <QApplication>
#include <QTextDocument>
#include <QTimer>

#include <KDebug>

#include <kdeversion.h>
#include <kinputdialog.h>

#include <kabcpersistence.h>
#include <kdialog.h>
#include <klocale.h>
#include <kicon.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <k3listviewsearchline.h>

#include "kopetecontactlist.h"
#include "kopeteglobal.h"
#include "kopeteuiglobal.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetestdaction.h"
#include "kopetechatsession.h"
#include "kopeteview.h"
#include "kopetemetacontact.h"
#include "kopeteappearancesettings.h"
#include "kopetebehaviorsettings.h"
#include "metacontactselectorwidget.h"
#include "kopeteemoticons.h"
#include "kopetestatusmessage.h"
#include "kopeteinfodialog.h"
#include "kopetedeletecontacttask.h"

//For the moving to another metacontact dialog
#include <qlabel.h>
#include <qimage.h>
#include <qmime.h>
#include <kvbox.h>
#include <k3listview.h>
#include <qcheckbox.h>


namespace Kopete {

class Contact::Private
{
public:
	bool fileCapable;

	OnlineStatus onlineStatus;
	Account *account;

	MetaContact *metaContact;

	QString contactId;
	QString icon;

	QTime idleTimer;
	unsigned long int idleTime;

	Kopete::StatusMessage statusMessage;
	KToggleAction* toggleAlwaysVisibleAction;

	Kopete::Contact::NameType preferredNameType;
	QString oldName;
};

/* static */
Kopete::Contact::NameType Kopete::Contact::nameTypeFromString(const QString &nameType)
{
	if (nameType == "nickName")
		return Kopete::Contact::NickName;
	else if (nameType == "customName")
		return Kopete::Contact::CustomName;
	else if (nameType == "formattedName")
		return Kopete::Contact::FormattedName;
	else if (nameType == "contactId")
		return Kopete::Contact::ContactId;
	else // fallback to custom name
		return Kopete::Contact::CustomName;
}

/* static */
const QString Kopete::Contact::nameTypeToString(Kopete::Contact::NameType nameType)
{
	switch (nameType)
	{
		case Kopete::Contact::NickName:
			return QString("nickName");
		case Kopete::Contact::FormattedName:
			return QString("formattedName");
		case Kopete::Contact::ContactId:
			return QString("contactId");
		case Kopete::Contact::CustomName:
		default: // fallback to custom name
			return QString("customName");
	}
}

Contact::Contact( Account *account, const QString &contactId,
	MetaContact *parent, const QString &icon )
	: ContactListElement( parent ), d(new Private())
{
	//kDebug( 14010 ) << "Creating contact with id " << contactId;

	d->contactId = contactId;
	d->metaContact = parent;
	connect( d->metaContact, SIGNAL(destroyed(QObject*)), this, SLOT(slotMetaContactDestroyed(QObject*)) );

	d->fileCapable = false;
	d->account = account;
	d->idleTime = 0;
	d->icon = icon;
	d->preferredNameType = Kopete::Contact::CustomName;
	d->oldName = displayName();

	connect( this, SIGNAL(propertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)),
		this, SLOT(slotPropertyChanged(Kopete::PropertyContainer*,QString,QVariant,QVariant)) );

	bool duplicate = false;
	// If can happend that a MetaContact may be used without a account
	// (ex: for unit tests or chat window style preview)
	if ( account )
	{
		// Don't register myself contacts because otherwise we can't have own contact in contact list.
		if ( d->metaContact != Kopete::ContactList::self()->myself() )
			duplicate = !account->registerContact( this );

		connect( account, SIGNAL(isConnectedChanged()), SLOT(slotAccountIsConnectedChanged()) );
	}

	// Need to check this because myself() may have no parent
	// Maybe too the metaContact doesn't have a valid protocol()
	// (ex: for unit tests or chat window style preview)

	// if alreadyRegistered is true (which mean that this is duplicate contact) we will not add
	// parent and the contact will die out on next Kopete restart.
	if( !duplicate && parent && protocol() )
		parent->addContact( this );
}

Contact::~Contact()
{
	//kDebug(14010) ;
	emit( contactDestroyed( this ) );
	delete d;
}



OnlineStatus Contact::onlineStatus() const
{
	if ( this == account()->myself() || account()->isConnected() )
		return d->onlineStatus;
	else
		return protocol()->accountOfflineStatus();
}

void Contact::setOnlineStatus( const OnlineStatus &status )
{
	if( status == d->onlineStatus )
		return;

	const bool oldCanAcceptFiles = canAcceptFiles();
	OnlineStatus oldStatus = d->onlineStatus;
	d->onlineStatus = status;

	Kopete::Global::Properties *globalProps = Kopete::Global::Properties::self();

	// Contact changed from Offline to another known online status
	if( oldStatus.status() == OnlineStatus::Offline &&
		status.status() != OnlineStatus::Unknown &&
		status.status() != OnlineStatus::Offline )
	{
		if ( !hasProperty( globalProps->onlineSince().key() ) )
			setProperty( globalProps->onlineSince(), QDateTime::currentDateTime() );
		// kDebug(14010) << "REMOVING lastSeen property for " << nickName();
		removeProperty( globalProps->lastSeen() );
	}
	else if( oldStatus.status() != OnlineStatus::Offline &&
		oldStatus.status() != OnlineStatus::Unknown &&
		status.status() == OnlineStatus::Offline ) // Contact went back offline
	{
		removeProperty( globalProps->onlineSince() );
		// kDebug(14010) << "SETTING lastSeen property for " << nickName();
		setProperty( globalProps->lastSeen(), QDateTime::currentDateTime() );
	}

	if ( this == account()->myself() || account()->isConnected() )
		emit onlineStatusChanged( this, status, oldStatus );

	if ( oldCanAcceptFiles != canAcceptFiles() )
		emit canAcceptFilesChanged();
}

Kopete::StatusMessage Contact::statusMessage() const
{
	return d->statusMessage;
}

void Contact::setStatusMessage( const Kopete::StatusMessage &statusMessage )
{
	bool emitUpdate = true;

	if ( d->statusMessage.title() == statusMessage.title() && d->statusMessage.message() == statusMessage.message() )
		emitUpdate = false;

	d->statusMessage = statusMessage;

	kDebug(14010) << "Setting up the status title property with this: " << statusMessage.title();
	if( !statusMessage.title().isEmpty() )
		setProperty( Kopete::Global::Properties::self()->statusTitle(), statusMessage.title() );
	else
		removeProperty( Kopete::Global::Properties::self()->statusTitle() );

	kDebug(14010) << "Setting up the status message property with this: " << statusMessage.message();
	if( !statusMessage.message().isEmpty() )
		setProperty( Kopete::Global::Properties::self()->statusMessage(), statusMessage.message() );
	else
		removeProperty( Kopete::Global::Properties::self()->statusMessage() );

	if ( emitUpdate )
		emit statusMessageChanged( this );
}

void Contact::slotAccountIsConnectedChanged()
{
	if ( this == account()->myself() )
		return;

	if ( account()->isConnected() )
		emit onlineStatusChanged( this, d->onlineStatus, protocol()->accountOfflineStatus() );
	else
		emit onlineStatusChanged( this, protocol()->accountOfflineStatus(), d->onlineStatus );
}


void Contact::sendFile( const KUrl &, const QString &, uint )
{
	kWarning( 14010 ) << "Plugin "
		<< protocol()->pluginId() << " has enabled file sending, "
		<< "but didn't implement it!" << endl;
}

void Contact::slotAddContact()
{
	if( metaContact() )
	{
		metaContact()->setTemporary( false );
		ContactList::self()->addMetaContact( metaContact() );
	}
}

KMenu* Contact::popupMenu( ChatSession * )
{
	return popupMenu();
}

KMenu* Contact::popupMenu()
{
	KMenu *menu = new KMenu();

	QString titleText;
	const QString nick = displayName();
	if( nick == contactId() )
		titleText = QString::fromLatin1( "%1 (%2)" ).arg( contactId(), onlineStatus().description() );
	else
		titleText = QString::fromLatin1( "%1 <%2> (%3)" ).arg( nick, contactId(), onlineStatus().description() );
	menu->addTitle( titleText );

	if( metaContact() && metaContact()->isTemporary() && contactId() != account()->myself()->contactId() )
	{
		KAction *actionAddContact = new KAction( KIcon("list-add-user"), i18n( "&Add to Your Contact List" ), menu );
		connect( actionAddContact, SIGNAL(triggered(bool)), this, SLOT(slotAddContact()) );

		menu->addAction(actionAddContact);
		menu->addSeparator();
	}

	// FIXME: After KDE 3.2 we should make isReachable do the isConnected call so it can be removed here - Martijn
	const bool reach = account()->isConnected() && isReachable();
	const bool myself = (this == account()->myself());

	KAction *actionSendMessage = KopeteStdAction::sendMessage( this, SLOT(sendMessage()), menu );
	actionSendMessage->setEnabled( reach && !myself );
	menu->addAction( actionSendMessage );

	KAction *actionChat = KopeteStdAction::chat( this, SLOT(startChat()), menu );
	actionChat->setEnabled( reach && !myself );
	menu->addAction( actionChat );

	KAction *actionSendFile = KopeteStdAction::sendFile( this, SLOT(sendFile()), menu );
	actionSendFile->setEnabled( reach && d->fileCapable && !myself );
	menu->addAction( actionSendFile );

	// Protocol specific options will go below this separator
	// through the use of the customContextMenuActions() function

	// Get the custom actions from the protocols ( pure virtual function )
	QList<KAction*> *customActions = customContextMenuActions();
	if( customActions && !customActions->isEmpty() )
	{
		menu->addSeparator();
		QList<KAction*>::iterator it, itEnd = customActions->end();
		for( it = customActions->begin(); it != itEnd; ++it )
			menu->addAction( (*it) );
	}
	delete customActions;

	menu->addSeparator();

	if( metaContact() && !metaContact()->isTemporary() )
	{
		KAction* changeMetaContact = KopeteStdAction::changeMetaContact( this, SLOT(changeMetaContact()), menu );
		menu->addAction( changeMetaContact );

		d->toggleAlwaysVisibleAction = new KToggleAction( i18n( "Visible when offline" ), menu );
		d->toggleAlwaysVisibleAction->setChecked( property( Kopete::Global::Properties::self()->isAlwaysVisible() ).value().toBool() );
		menu->addAction( d->toggleAlwaysVisibleAction );
		connect( d->toggleAlwaysVisibleAction, SIGNAL(toggled(bool)), this, SLOT(toggleAlwaysVisible()) );
	}

	menu->addAction( KopeteStdAction::contactInfo( this, SLOT(slotUserInfo()), menu ) );

#if 0 //this is not fully implemented yet (and doesn't work).  disable for now   - Olivier 2005-01-11
	if ( account()->isBlocked( d->contactId ) )
		KopeteStdAction::unblockContact( this, SLOT(slotUnblock()), menu, "actionUnblockContact" )->plug( menu );
	else
		KopeteStdAction::blockContact( this, SLOT(slotBlock()), menu, "actionBlockContact" )->plug( menu );
#endif

	if( metaContact() && !metaContact()->isTemporary() )
		menu->addAction( KopeteStdAction::deleteContact( this, SLOT(slotDelete()), menu ) );

	return menu;
}

void Contact::toggleAlwaysVisible()
{
	bool alwaysVisible = property( Kopete::Global::Properties::self()->isAlwaysVisible() ).value().toBool();
	setProperty( Kopete::Global::Properties::self()->isAlwaysVisible(), !alwaysVisible );
	d->toggleAlwaysVisibleAction->setChecked( !alwaysVisible );
}

void Contact::changeMetaContact()
{
	QPointer <KDialog> moveDialog = new KDialog( Kopete::UI::Global::mainWidget() );
	moveDialog->setCaption( i18n( "Move Contact" ) );
	moveDialog->setButtons( KDialog::Ok | KDialog::Cancel );
	moveDialog->setDefaultButton( KDialog::Ok );
	moveDialog->showButtonSeparator( true );

	KVBox *w = new KVBox( moveDialog );
	w->setSpacing( KDialog::spacingHint() );
	Kopete::UI::MetaContactSelectorWidget *selector = new Kopete::UI::MetaContactSelectorWidget(w);
	selector->setLabelMessage(i18n( "Select the meta contact to which you want to move this contact:" ));
	// exclude this metacontact as a target metacontact for the move
	selector->excludeMetaContact( metaContact() );
	QCheckBox *chkCreateNew = new QCheckBox( i18n( "Create a new metacontact for this contact" ), w );
	chkCreateNew ->setWhatsThis( i18n( "If you select this option, a new metacontact will be created in the top-level group "
		"with the name of this contact and the contact will be moved to it." ) );
	QObject::connect( chkCreateNew , SIGNAL(toggled(bool)) ,  selector , SLOT (setDisabled(bool)) ) ;

	moveDialog->setMainWidget(w);
	if( moveDialog->exec() == QDialog::Accepted )
	{
		Kopete::MetaContact *mc = selector->metaContact();
		if(chkCreateNew->isChecked())
		{
			mc=new Kopete::MetaContact();

			if ( metaContact() )
			{	// Add new metaContact to old groups so we don't move it to Top Level group
				foreach ( Kopete::Group* group, metaContact()->groups() )
					mc->addToGroup( group );
			}

			Kopete::ContactList::self()->addMetaContact(mc);
		}
		if( mc )
		{
			setMetaContact( mc );
		}
	}

	if ( moveDialog )
		moveDialog->deleteLater();
}

void Contact::slotMetaContactDestroyed( QObject* mc )
{
	if (mc != d->metaContact)
		return;

	d->metaContact = 0;
}

void Contact::setMetaContact( MetaContact *m )
{
	MetaContact *old = d->metaContact;
	if(old==m) //that make no sens
		return;

	if( old )
	{
		old->removeContact( this );
		disconnect( old, SIGNAL(destroyed(QObject*)), this, SLOT(slotMetaContactDestroyed(QObject*)) );

		if(old->contacts().isEmpty())
		{
			//remove the old metacontact.  (this delete the MC)
			ContactList::self()->removeMetaContact(old);
		}
		else
		{
			d->metaContact = m; //i am forced to do that now if i want the next line works
			//remove cached data for this protocol which will not be removed since we disconnected
			protocol()->serialize( old );
		}
	}

	d->metaContact = m;
	setParent( m );

	if( m )
	{
		m->addContact( this );
		connect( m, SIGNAL(destroyed(QObject*)), this, SLOT(slotMetaContactDestroyed(QObject*)) );
		// it is necessary to call this write here, because MetaContact::addContact() does not differentiate
		// between adding completely new contacts (which should be written to kabc) and restoring upon restart
		// (where no write is needed).
		KABCPersistence::self()->write( m );
	}
	sync();
}

void Contact::serialize( QMap<QString, QString> &/*serializedData*/,
	QMap<QString, QString> & /* addressBookData */ )
{
}

bool Contact::isReachable()
{
	// The default implementation returns false when offline and true
	// otherwise. Subclass if you need more control over the process.
	return onlineStatus().status() != OnlineStatus::Offline;
}


void Contact::startChat()
{
	KopeteView *v=manager( CanCreate )->view(true, QString::fromLatin1("kopete_chatwindow") );
	if(v)
		v->raise(true);
}

void Contact::sendMessage()
{
	KopeteView *v=manager( CanCreate )->view(true, QString::fromLatin1("kopete_emailwindow") );
	if(v)
		v->raise(true);
}

void Contact::execute()
{
	// FIXME: After KDE 3.2 remove the isConnected check and move it to isReachable - Martijn
	if ( account()->isConnected() && isReachable() )
	{
		KopeteView *v=manager( CanCreate )->view(true, Kopete::BehaviorSettings::self()->viewPlugin() );
		if(v)
			v->raise(true);
	}
	else
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
			i18n( "This user is not reachable at the moment. Please try a protocol that supports offline sending, or wait "
			"until this user comes online." ), i18n( "User is Not Reachable" ) );
	}
}

void Contact::slotDelete()
{
	if ( KMessageBox::warningContinueCancel( Kopete::UI::Global::mainWidget(),
		i18n( "Are you sure you want to remove the contact  '%1' from your contact list?" ,
		 d->contactId ), i18n( "Remove Contact" ), KGuiItem(i18n("Remove"), QString::fromLatin1("list-remove-user") ), KStandardGuiItem::cancel(),
		QString::fromLatin1("askRemoveContact"), KMessageBox::Notify | KMessageBox::Dangerous )
		== KMessageBox::Continue )
	{
		Kopete::DeleteContactTask *deleteTask = new Kopete::DeleteContactTask(this);
		deleteTask->start();
	}
}

void Contact::deleteContact()
{
	// Default implementation simply deletes the contact
	deleteLater();
}


MetaContact * Contact::metaContact() const
{
	return d->metaContact;
}

QString Contact::contactId() const
{
	return d->contactId;
}

Protocol * Contact::protocol() const
{
	return d->account ? d->account->protocol() : 0L;
}

Account * Contact::account() const
{
	return d->account;
}



void Contact::sync(unsigned int)
{
	/* Default implementation does nothing */
}

QString& Contact::icon() const
{
	return d->icon;
}

void Contact::setIcon( const QString& icon )
{
	d->icon = icon;
	return;
}

QList<KAction *> *Contact::customContextMenuActions()
{
	return 0L;
}

QList<KAction*> *Contact::customContextMenuActions( ChatSession * /* manager */ )
{
	return customContextMenuActions();
}

bool Contact::isOnline() const
{
	return onlineStatus().isDefinitelyOnline();
}

bool Contact::isFileCapable() const
{
	return d->fileCapable;
}

void Contact::setFileCapable( bool filecap )
{
	if ( d->fileCapable != filecap )
	{
		d->fileCapable = filecap;
		emit canAcceptFilesChanged();
	}
}

bool Contact::canAcceptFiles() const
{
	return isOnline() && d->fileCapable;
}

unsigned long int Contact::idleTime() const
{
	if(d->idleTime==0)
		return 0;

	return d->idleTime+(d->idleTimer.elapsed()/1000);
}

void Contact::setIdleTime( unsigned long int t )
{
	bool idleChanged = false;
	if(d->idleTime != t)
		idleChanged = true;
	d->idleTime=t;
	if(t > 0)
		d->idleTimer.start();
//FIXME: if t == 0, idleTime() will now return garbage
//	else
//		d->idleTimer.stop();
	if(idleChanged)
		emit idleStateChanged(this);
}

QString Contact::toolTip() const
{
	Kopete::Property p;
	QString tip;
	const QStringList shownProps = Kopete::AppearanceSettings::self()->toolTipContents();

	// --------------------------------------------------------------------------
	// Fixed part of tooltip

	QString iconName;
	if ( this == account()->myself() )
	{
		iconName = QString::fromLatin1("kopete-account-icon:%1:%2")
			.arg( QString(QUrl::toPercentEncoding( protocol()->pluginId() )),
			      QString(QUrl::toPercentEncoding( account()->accountId() )) );

	}
	else
	{
		iconName = QString::fromLatin1("kopete-contact-icon:%1:%2:%3")
			.arg( QString(QUrl::toPercentEncoding( protocol()->pluginId() )),
			      QString(QUrl::toPercentEncoding( account()->accountId() )),
			      QString(QUrl::toPercentEncoding( contactId() )) );
	}

	QString nick = displayName();
	if ( nick == contactId() )
	{
		tip = i18nc( "@label:textbox %3 is contact-display-name, %1 is its status",
			"<b><nobr>%3</nobr></b><br /><img src=\"%2\">&nbsp;%1",
			Kopete::Message::escape( onlineStatus().description() ), iconName,
				Kopete::Message::escape( d->contactId ) );
	}
	else
	{
		tip = i18nc( "@label:textbox %4 is contact-display-name, %3 is contact-id, %1 is its status",
			"<nobr><b>%4</b> (%3)</nobr><br /><img src=\"%2\">&nbsp;%1",
				Kopete::Message::escape( onlineStatus().description() ), iconName,
					Kopete::Message::escape( contactId() ),
					Kopete::Emoticons::parseEmoticons( Kopete::Message::escape( nick ) ) );
	}

	// --------------------------------------------------------------------------
	// Configurable part of tooltip

	// FIXME: It shouldn't use QString to identity the properties. Instead it should use PropertyTmpl::key()
	for(QStringList::ConstIterator it=shownProps.constBegin(); it!=shownProps.constEnd(); ++it)
	{
		if((*it) == Kopete::Global::Properties::self()->fullName().key())
		{
			const QString name = formattedName();
			if(!name.isEmpty())
			{
				tip += i18nc("@label:textbox formatted name",
							"<br /><b>Full Name:</b>&nbsp;<nobr>%1</nobr>", Qt::escape(name));
			}
		}
		else if ((*it) == Kopete::Global::Properties::self()->idleTime().key())
		{
			const QString time = formattedIdleTime();
			if(!time.isEmpty())
			{
				tip += i18nc("@label:textbox formatted idle time",
					"<br /><b>Idle:</b>&nbsp;<nobr>%1</nobr>", time);
			}
		}
		else if ((*it) == QString::fromLatin1("homePage"))
		{
			const QString url = property(*it).value().toString();
			if(!url.isEmpty())
			{
				tip += i18nc("@label:textbox formatted url",
					"<br /><b>Home Page:</b>&nbsp;<a href=\"%1\"><nobr>%2</nobr></a>",
						QString(QUrl::toPercentEncoding( url )), Kopete::Message::escape( Qt::escape(url) ) );
			}
		}
		else if ((*it) == Kopete::Global::Properties::self()->statusTitle().key() )
		{
			const QString statusTitle = property(*it).value().toString();
			if(!statusTitle.isEmpty())
			{
				tip += i18nc("@label:textbox formatted status title",
				             "<br /><b>Status&nbsp;Title:</b>&nbsp;%1",  Kopete::Emoticons::parseEmoticons( Kopete::Message::escape(statusTitle) ) );
			}
		}
		else if ((*it) == Kopete::Global::Properties::self()->statusMessage().key() )
		{
			const QString statusmsg = property(*it).value().toString();
			if(!statusmsg.isEmpty())
			{
				tip += i18nc("@label:textbox formatted status message",
							"<br /><b>Status&nbsp;Message:</b>&nbsp;%1",  Kopete::Emoticons::parseEmoticons( Kopete::Message::escape(statusmsg) ) );
			}
		}
		else
		{
			p = property(*it);
			if(!p.isNull())
			{
				QVariant val = p.value();
				QString valueText;

				switch(val.type())
				{
					case QVariant::DateTime:
						valueText = KGlobal::locale()->formatDateTime(val.toDateTime());
						valueText = Kopete::Message::escape( valueText );
						break;
					case QVariant::Date:
						valueText = KGlobal::locale()->formatDate(val.toDate());
						valueText = Kopete::Message::escape( valueText );
						break;
					case QVariant::Time:
						valueText = KGlobal::locale()->formatTime(val.toTime());
						valueText = Kopete::Message::escape( valueText );
						break;
					default:
						if( p.isRichText() )
						{
							valueText = val.toString();
						}
						else
						{
							valueText = Kopete::Message::escape( val.toString() );
						}
				}

				if (valueText.size() > 1000) {
					valueText.truncate(997);
					valueText += "...";
				}

				tip += i18nc("@label:textbox property label %2 is name, %1 is value",
					"<br /><nobr><b>%2:</b></nobr>&nbsp;%1",
						valueText, Qt::escape(p.tmpl().label()) );
			}
		}
	}

	return tip;
}

QString Kopete::Contact::formattedName() const
{
	if( hasProperty( Kopete::Global::Properties::self()->fullName().key() ) )
		return property( Kopete::Global::Properties::self()->fullName() ).value().toString();

	QString ret;
	Kopete::Property first, last;

	first = property( Kopete::Global::Properties::self()->firstName() );
	last = property( Kopete::Global::Properties::self()->lastName() );
	if(!first.isNull())
	{
		if(!last.isNull()) // contact has both first and last name
		{
			ret = i18nc("firstName lastName", "%2 %1",
				 last.value().toString(),
				 first.value().toString());
		}
		else // only first name set
		{
			ret = first.value().toString();
		}
	}
	else if(!last.isNull()) // only last name set
	{
		ret = last.value().toString();
	}

	return ret;
}

QString Kopete::Contact::formattedIdleTime() const
{
	QString ret;
	unsigned long int leftTime = idleTime();

	if ( leftTime > 0 )
	{	// FIXME: duplicated from code in kopetecontact listview.cpp
		unsigned long int days, hours, mins, secs;

		days = leftTime / ( 60*60*24 );
		leftTime = leftTime % ( 60*60*24 );
		hours = leftTime / ( 60*60 );
		leftTime = leftTime % ( 60*60 );
		mins = leftTime / 60;
		secs = leftTime % 60;

		if ( days != 0 )
		{
			ret = i18nc( "<days>d <hours>h <minutes>m <seconds>s",
				"%4d %3h %2m %1s" ,
				  secs ,
				  mins ,
				  hours ,
				  days );
		}
		else if ( hours != 0 )
		{
			ret = i18nc( "<hours>h <minutes>m <seconds>s", "%3h %2m %1s" ,
				  secs ,
				  mins ,
				  hours );
		}
		else
		{
			// xgettext: no-c-format
			ret = i18nc( "<minutes>m <seconds>s", "%2m %1s" ,
				  secs ,
				  mins );
		}
	}
	return ret;
}

void Kopete::Contact::slotBlock()
{
	account()->block( d->contactId );
}

void Kopete::Contact::slotUnblock()
{
	account()->unblock( d->contactId );
}

void Kopete::Contact::setNickName( const QString &name )
{
	setProperty( Kopete::Global::Properties::self()->nickName(), name );
}

QString Kopete::Contact::nickName() const
{
	const QString nick = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if( !nick.isEmpty() )
		return nick;

	return contactId();
}

void Kopete::Contact::setCustomName( const QString &name )
{
	setProperty( Kopete::Global::Properties::self()->customName(), name );
}

QString Kopete::Contact::customName() const
{
	const QString name = property( Kopete::Global::Properties::self()->customName() ).value().toString();
	if (!name.isEmpty())
		return name;
	return nickName();
}

void Kopete::Contact::setPhoto(const QString &photoPath)
{
	setProperty( Kopete::Global::Properties::self()->photo(), photoPath );
}

void Kopete::Contact::slotPropertyChanged(Kopete::PropertyContainer *, const QString &key,
	const QVariant &, const QVariant &)
{
	if (key != Kopete::Global::Properties::self()->customName().key()
		&& key != Kopete::Global::Properties::self()->fullName().key()
		&& key != Kopete::Global::Properties::self()->firstName().key()
		&& key != Kopete::Global::Properties::self()->lastName().key()
		&& key != Kopete::Global::Properties::self()->nickName().key())
		return;

	const QString oldName = d->oldName;
	const QString newName = displayName();
	if (oldName != newName) {
		d->oldName = newName;
		emit displayNameChanged(oldName, newName);
	}
}

void Kopete::Contact::setPreferredNameType(Kopete::Contact::NameType preferredNameType)
{
	if (d->preferredNameType != preferredNameType)
	{
		const QString oldName = displayName();
		d->preferredNameType = preferredNameType;
		const QString newName = displayName();
		if (oldName != newName) {
			d->oldName = newName;
			emit displayNameChanged(oldName, newName);
		}
	}
}

Kopete::Contact::NameType Kopete::Contact::preferredNameType() const
{
	return d->preferredNameType;
}

QString Kopete::Contact::displayName() const
{
	QString name;
	switch (d->preferredNameType)
	{
		case NickName:
			name = nickName();
			break;
		case FormattedName:
			name = formattedName();
			break;
		case ContactId:
			name = contactId();
			break;
		case CustomName:
		default: // fallback to custom name
			name = customName();
			break;
	}
	if (name.isEmpty())
		return contactId();
	else
		return name;
}


} //END namespace Kopete

#include "kopetecontact.moc"
