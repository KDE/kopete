/*
    kopetecontact.cpp - Kopete Contact

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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

#include <qapplication.h>
#include <qstylesheet.h>

#include <kdebug.h>

#include <kdeversion.h>
#if KDE_IS_VERSION( 3, 1, 90 )
#include <kinputdialog.h>
#else
#include <klineeditdlg.h>
#endif

#include <kdialogbase.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

#include "kopetecontactlist.h"
#include "kopeteuiglobal.h"
#include "kopeteprefs.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetestdaction.h"
#include "kopetemessagemanager.h"
#include "kopeteview.h"
#include "kopetemetacontact.h"


//For the moving to another metacontact dialog
#include <qlabel.h>
#include <qstylesheet.h>
#include <qmime.h>
#include <qvbox.h>
#include <klistview.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>


// FIXME: What are these doing here and why are they #defines and not const ints? - Martijn
#define EMAIL_WINDOW 0
#define CHAT_WINDOW 1

struct KopeteContactPrivate
{
public:
	QString displayName;
	bool fileCapable;

	KopeteOnlineStatus onlineStatus;
	KopeteAccount *account;

	KopeteMetaContact *metaContact;

	KAction *actionSendMessage;
	KAction *actionChat;
	KAction *actionDeleteContact;
	KAction *actionChangeMetaContact;
	KAction *actionChangeAlias;
	KAction *actionUserInfo;
	KAction *actionSendFile;
	KAction *actionAddContact;

	QString contactId;
	QString icon;

	QTime idleTimer;
	unsigned long int idleTime;

	QMap< QString, KopeteContactProperty> properties;
};

KopeteContact::KopeteContact( KopeteAccount *account, const QString &contactId, KopeteMetaContact *parent, const QString &icon )
	: QObject( parent )
{
	d = new KopeteContactPrivate;

	//kdDebug( 14010 ) << k_funcinfo << "Creating contact with id " << contactId << endl;

	d->contactId = contactId;

	d->metaContact = parent;
	d->fileCapable = false;
	d->displayName = contactId;
	d->account = account;
	d->idleTime = 0;

	if ( account )
		account->registerContact( this );

	d->icon = icon;

	// Need to check this because myself() has no parent
	if( parent )
	{
		connect( parent, SIGNAL( aboutToSave( KopeteMetaContact * ) ),
			protocol(), SLOT( slotMetaContactAboutToSave( KopeteMetaContact * ) ) );

		parent->addContact( this );
	}
}

KopeteContact::~KopeteContact()
{
	kdDebug() << k_funcinfo << endl;
	emit( contactDestroyed( this ) );
	d->metaContact = 0L;
	delete d;
}

/*void KopeteContact::setConversations( int value ) const
{
	d->conversations = value;
}

const int KopeteContact::conversations() const
{
	return d->conversations;
}*/

void KopeteContact::rename( const QString &name )
{
	// Default implementation immediately sets the new name
	setDisplayName( name );
}

void KopeteContact::setDisplayName( const QString &name )
{
	if( name == d->displayName )
		return;

	QString old= d->displayName ;
	d->displayName = name;
	emit displayNameChanged( old , name );
}

QString KopeteContact::displayName() const
{
	return d->displayName;
}

const KopeteOnlineStatus& KopeteContact::onlineStatus() const
{
	return d->onlineStatus;
}

void KopeteContact::setOnlineStatus( const KopeteOnlineStatus &status )
{
	if( status == d->onlineStatus )
		return;

	KopeteOnlineStatus oldStatus = d->onlineStatus;
	d->onlineStatus = status;

	// Contact changed from Offline to another status
	if( oldStatus.status() == KopeteOnlineStatus::Offline &&
		status.status() != KopeteOnlineStatus::Offline )
	{
		setProperty( QString::fromLatin1("onlineSince"), QDateTime::currentDateTime() );
	}
	else if( status.status() == KopeteOnlineStatus::Offline ) // Contact went back offline
	{
		removeProperty( QString::fromLatin1("onlineSince") );
	}

	emit onlineStatusChanged( this, status, oldStatus );
}

void KopeteContact::sendFile( const KURL &, const QString &, uint )
{
	kdWarning( 14010 ) << k_funcinfo << "Plugin "
		<< protocol()->pluginId() << " has enabled file sending, "
		<< "but didn't implement it!" << endl;
}

void KopeteContact::slotAddContact()
{
	if( metaContact() )
	{
		metaContact()->setTemporary( false );
		KopeteContactList::contactList()->addMetaContact( metaContact() );
	}
}

KPopupMenu* KopeteContact::popupMenu( KopeteMessageManager *manager )
{
	// FIXME:
	// This should perhaps be KActionCollection * KopeteContact::contactActions()
	// to avoid passing around KPopupMenu's (Jason)
	//
	// KActionCollections are bad for popup menus because they are unordered.
	// in fact, I think customContextMenuActions should be remade into a popupmenu,
	// or a QPtrList<KAction>, or something that has a notion of order, because
	// currently the customContextMenuActions do not return in the order they are
	// added, which makes for a mess when you want certain things at the top and
	// others later on.

	// Build the menu
	KPopupMenu *menu = new KPopupMenu();

	// Initialize the context menu
	d->actionChat        = KopeteStdAction::chat( this,        SLOT( startChat() ),             menu, "actionChat" );
	d->actionSendFile    = KopeteStdAction::sendFile( this,    SLOT( sendFile() ),              menu, "actionSendFile" );
	d->actionUserInfo    = KopeteStdAction::contactInfo( this, SLOT( slotUserInfo() ),          menu, "actionUserInfo" );
	d->actionSendMessage = KopeteStdAction::sendMessage( this, SLOT( sendMessage() ),           menu, "actionSendMessage" );
	d->actionChangeAlias = KopeteStdAction::changeAlias( this, SLOT( slotChangeDisplayName() ), menu, "actionChangeAlias" );
	d->actionDeleteContact = KopeteStdAction::deleteContact( this, SLOT( slotDeleteContact() ), menu, "actionDeleteContact" );
	d->actionChangeMetaContact = KopeteStdAction::changeMetaContact( this, SLOT( slotChangeMetaContact() ), menu, "actionChangeMetaContact" );
		//FIXME: after string frezee add a &
	d->actionAddContact = new KAction( i18n( "Add to Your Contact List" ), QString::fromLatin1( "bookmark_add" ), 0,
		this, SLOT( slotAddContact() ), menu, "actionAddContact" );

	// FIXME: After KDE 3.2 we should make isReachable do the isConnected call so it can be removed here - Martijn
	bool reach = isReachable() && d->account->isConnected(); // save calling a method several times
	d->actionChat->setEnabled( reach );
	d->actionSendFile->setEnabled( reach && d->fileCapable );
	d->actionSendMessage->setEnabled( reach );
	// FIXME: is userinfo supposed to work while being offline.
	// Some protocols might have valid userinfo while the contact is offline [mETz]

	QString titleText;
#if QT_VERSION < 0x030200
	if( displayName() == contactId() )
		titleText = QString::fromLatin1( "%1 (%2)" ).arg( displayName() ).arg( d->onlineStatus.description() );
	else
		titleText = QString::fromLatin1( "%1 <%2> (%3)" ).arg( displayName() ).arg( contactId() ).arg( d->onlineStatus.description() );
#else
	if( displayName() == contactId() )
		titleText = QString::fromLatin1( "%1 (%2)" ).arg( displayName(), d->onlineStatus.description() );
	else
		titleText = QString::fromLatin1( "%1 <%2> (%3)" ).arg( displayName(), contactId(), d->onlineStatus.description() );
#endif

	menu->insertTitle( titleText );

	if( metaContact() && metaContact()->isTemporary() )
	{
		d->actionAddContact->plug( menu );
		menu->insertSeparator();
	}

	d->actionSendMessage->plug( menu );
	d->actionChat->plug( menu );
	d->actionSendFile->plug( menu );

	// Protocol specific options will go below this separator
	// through the use of the customContextMenuActions() function

	// Get the custom actions from the protocols ( pure virtual function )
	QPtrList<KAction> *customActions = customContextMenuActions( manager );
	if( customActions )
	{
		if ( !customActions->isEmpty() )
		{
			menu->insertSeparator();

			for( KAction *a = customActions->first(); a; a = customActions->next() )
				a->plug( menu );
		}
	}

	delete customActions;

	menu->insertSeparator();
	if( metaContact() && !metaContact()->isTemporary() )
		d->actionChangeMetaContact->plug( menu );

	d->actionUserInfo->plug( menu );

	if( metaContact() && !metaContact()->isTemporary() )
	{
		d->actionChangeAlias->plug( menu );
		d->actionDeleteContact->plug( menu );
	}

	return menu;
}

void KopeteContact::slotChangeDisplayName()
{
	QString newName =
#if KDE_IS_VERSION( 3, 1, 90 )
		KInputDialog::getText( i18n( "Change Alias" ), i18n( "New alias for %1:" ).arg( contactId() ), displayName());
#else
		KLineEditDlg::getText( i18n( "Change Alias" ), i18n( "New alias for %1:" ).arg( contactId() ), displayName());
#endif

	if( !newName.isNull() )
		setDisplayName( newName );
}

void KopeteContact::slotChangeMetaContact()
{
	KDialogBase *moveDialog = new KDialogBase( Kopete::UI::Global::mainWidget(), "moveDialog", true, i18n( "Move Contact" ),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );

	QVBox *w = new QVBox( moveDialog );
	w->setSpacing( 8 );
	new QLabel( i18n( "Select the meta contact to which you want to move this contact" ), w );
	KListView *selectMetaContactListBox = new KListView ( w, "selectMetaContactListBox" );
	selectMetaContactListBox->addColumn( i18n( "Display Name" ) );
	selectMetaContactListBox->addColumn( i18n( "Contact IDs" ) );

	QMap<QListViewItem*,KopeteMetaContact*> map;
	QPtrList<KopeteMetaContact> metaContacts = KopeteContactList::contactList()->metaContacts();
	for( KopeteMetaContact *mc = metaContacts.first(); mc ; mc = metaContacts.next() )
	{
		if( !mc->isTemporary() && mc != metaContact() )
		{
			QString t;
			bool f=true;
			QPtrList<KopeteContact> contacts = mc->contacts();
			for( KopeteContact *c = contacts.first(); c ; c = contacts.next() )
			{
				if( !f )
					t += QString::fromLatin1( " ; " );
				t += c->contactId();
				f = false;
			}

			map.insert(  new QListViewItem(selectMetaContactListBox, mc->displayName(),t) , mc  ) ;
		}
	}

	selectMetaContactListBox->sort();

	QCheckBox *chkCreateNew = new QCheckBox( i18n( "Create a new metacontact for this contact" ), w );
	QWhatsThis::add( chkCreateNew , i18n( "If you select this option, a new metacontact will be created in the top-level group "
		"with the name of this contact and the contact will be moved to it." ) );
	QObject::connect( chkCreateNew , SIGNAL( toggled(bool) ) ,  selectMetaContactListBox , SLOT ( setDisabled(bool) ) ) ;

	moveDialog->setMainWidget( w );
	if( moveDialog->exec() == QDialog::Accepted )
	{
		KopeteMetaContact *mc = map[selectMetaContactListBox->currentItem()];
		if(chkCreateNew->isChecked())
		{
			mc=new KopeteMetaContact();
			KopeteContactList::contactList()->addMetaContact(mc);
		}
		if( mc )
		{
			setMetaContact( mc );
		}
	}

	moveDialog->deleteLater();
}

void KopeteContact::setMetaContact( KopeteMetaContact *m )
{
	KopeteMetaContact *old = d->metaContact;
	if(old==m) //that make no sens
		return;

	if( old )
	{
		int result=KMessageBox::No;
		if( old->contacts().count()==1 )
		{ //only one contact, including this one, that mean the contact will be empty efter the move
			result = KMessageBox::questionYesNoCancel( Kopete::UI::Global::mainWidget(), i18n( "You are moving the contact `%1 <%2>' to `%3'.\n"
				"`%4' will be empty afterwards. Do you want to delete this contact?" )
#if QT_VERSION < 0x030200
					.arg(displayName()).arg(contactId()).arg(m ? m->displayName() : QString::null).arg(old->displayName())
#else
					.arg(displayName(), contactId(), m ? m->displayName() : QString::null, old->displayName())
#endif
				, i18n( "Move Contact" ), i18n( "&Delete" ) , i18n( "&Keep" ) , QString::fromLatin1("delete_old_contact_when_move") );

			if(result==KMessageBox::Cancel)
				return;
		}

		old->removeKABC();
		old->removeContact( this );
		disconnect( old, SIGNAL( aboutToSave( KopeteMetaContact * ) ),
			protocol(), SLOT( slotMetaContactAboutToSave( KopeteMetaContact * ) ) );
		old->updateKABC();

		if(result==KMessageBox::Yes)
		{
			//remove the old metacontact.  (this delete the MC)
			KopeteContactList::contactList()->removeMetaContact(old);
		}
		else
		{
			d->metaContact = m; //i am forced to do that now if i want the next line works
			//remove cached data for this protocol which will not be removed since we disconnected
			protocol()->slotMetaContactAboutToSave( old );
		}
	}

	d->metaContact = m;

	if( m )
	{
		m->addContact( this );
		m->insertChild( this );

		connect( d->metaContact, SIGNAL( aboutToSave( KopeteMetaContact * ) ),
		protocol(), SLOT( slotMetaContactAboutToSave( KopeteMetaContact * ) ) );
		m->updateKABC();
	}
	syncGroups();
}

void KopeteContact::serialize( QMap<QString, QString> & /*serializedData */,
	QMap<QString, QString> & /* addressBookData */ )
{
	// Do nothing in the default implementation
}

bool KopeteContact::isReachable()
{
	// The default implementation returns false when offline and true
	// otherwise. Subclass if you need more control over the process.
	return onlineStatus().status() != KopeteOnlineStatus::Offline;
}

void KopeteContact::startChat()
{
	KopeteView *v=manager(true)->view(true, KopeteMessage::Chat );
	if(v)
		v->raise(true);
}

void KopeteContact::sendMessage()
{
	KopeteView *v=manager(true)->view(true, KopeteMessage::Email );
	if(v)
		v->raise(true);
}

void KopeteContact::execute()
{
	// FIXME: After KDE 3.2 remove the isConnected check and move it to isReachable - Martijn
	if ( account()->isConnected() && isReachable() )
	{
		switch ( KopetePrefs::prefs()->interfacePreference() )
		{
		case EMAIL_WINDOW:
			sendMessage();
			break;
		case CHAT_WINDOW:
		default:
			startChat();
			break;
		}
	}
	else
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
			i18n( "This user is not reachable at the moment. Please try a protocol that supports offline sending, or wait "
			"until this user comes online." ), i18n( "User is Not Reachable" ) );
	}
}

/*KopeteMessageManager *KopeteContact::manager( bool )
{
	kdDebug( 14010 ) << "Manager() not implimented for " << protocol()->displayName() << ", crash!" << endl;
	return 0L;
}*/

void KopeteContact::slotDeleteContact()
{
	/* Default implementation simply delete the contact */
	deleteLater();
}

void KopeteContact::slotUserInfo()
{
	/* Default implementation does nothing */
}

bool KopeteContact::isOnline() const
{
	return d->onlineStatus.status() != KopeteOnlineStatus::Offline && d->onlineStatus.status() != KopeteOnlineStatus::Unknown;
}

KopeteMetaContact * KopeteContact::metaContact() const
{
	return d->metaContact;
}

QString KopeteContact::contactId() const
{
	return d->contactId;
}

KopeteProtocol * KopeteContact::protocol() const
{
	return d->account ? d->account->protocol() : 0L;
}

KopeteAccount * KopeteContact::account() const
{
	return d->account;
}

QPtrList<KAction> *KopeteContact::customContextMenuActions()
{
	return 0L;
}

QPtrList<KAction> *KopeteContact::customContextMenuActions( KopeteMessageManager * /* manager */ )
{
	return customContextMenuActions();
}

bool KopeteContact::isFileCapable() const
{
	return d->fileCapable;
}

void KopeteContact::setFileCapable( bool filecap )
{
	d->fileCapable = filecap;
}

bool KopeteContact::canAcceptFiles() const
{
	return isOnline() && d->fileCapable;
}

unsigned long int KopeteContact::idleTime() const
{
	if(d->idleTime==0)
		return 0;

	return d->idleTime+(d->idleTimer.elapsed()/1000);
}

void KopeteContact::setIdleTime(unsigned long int t )
{
	d->idleTime=t;
	if(t)
		d->idleTimer.start();
//	else
//		d->idleTimer.stop();
}

void KopeteContact::syncGroups()
{
	/* Default implementation does nothing */
}

QString& KopeteContact::icon() const
{
	return d->icon;
}

void KopeteContact::setIcon( const QString& icon )
{
	d->icon = icon;
	return;
}

const KopeteContactProperty &KopeteContact::property(const QString &key) const
{
	if(hasProperty(key))
		return d->properties[key];
	else
	{
		//kdDebug(14000) << k_funcinfo << "Property not found, returning null-property" << endl;
		return KopeteContactProperty::null;
	}
}

const QString &KopeteContact::propertyLabel(const QString &key) const
{
	if(hasProperty(key))
		return d->properties[key].label();
	else
		return QString::null;
}

void KopeteContact::setProperty(const QString &key, const QVariant &value)
{
	QString label;
	if(key == QString::fromLatin1("firstName"))
		label = i18n("First Name");
	else if (key == QString::fromLatin1("lastName"))
		label = i18n("Last Name");
	else if (key == QString::fromLatin1("emailAddress"))
		label = i18n("Email");
	else if (key == QString::fromLatin1("privPhoneNum"))
		label = i18n("Private Phone");
	else if (key == QString::fromLatin1("privFaxNum"))
		label = i18n("Private Fax");
	else if (key == QString::fromLatin1("privMobileNum"))
		label = i18n("Private Mobile");
	else if (key == QString::fromLatin1("awayMessage"))
		label = i18n("Away Message");
	else if (key == QString::fromLatin1("onlineSince"))
		label = i18n("Online Since");

	if(!label.isNull())
		setProperty(key, label, value);
	else
		kdDebug(14000) << k_funcinfo << "No known default property with key: " << key << endl;

}

void KopeteContact::setProperty(const QString &key, const QString &label, const QVariant &value)
{
	if(value.isNull())
	{
		removeProperty(key);
		return;
	}

	KopeteContactProperty prop( label, value );
	d->properties.insert(key, prop);
}

void KopeteContact::removeProperty(const QString &key)
{
	//kdDebug(14000) << k_funcinfo << "removing property " << key << endl;
	d->properties.remove(key);
}

QStringList KopeteContact::properties() const
{
	return d->properties.keys();
}

bool KopeteContact::hasProperty(const QString &key) const
{
	//kdDebug(14000) << k_funcinfo << "For key " << key << endl;
	return d->properties.contains(key);
}

QString KopeteContact::toolTip() const
{
	KopeteContactProperty p;
	QString tip;
	QStringList shownProps = KopetePrefs::prefs()->toolTipContents();

	//kdDebug(14000) << k_funcinfo << "Configured Tips: " << shownProps << endl;

	// --------------------------------------------------------------------------
	// Fixed part of tooltip

	QString iconName = QString::fromLatin1("kopete-contact-icon:%1:%2:%3")
	                               .arg( KURL::encode_string( protocol()->pluginId() ),
	                                     KURL::encode_string( account()->accountId() ),
	                                     KURL::encode_string( contactId() ) );

	if ( displayName() == contactId() )
	{
		tip = i18n( "<b>DISPLAY NAME</b><br><img src=\"%2\">&nbsp;CONTACT STATUS",
			"<b>%3</b><br><img src=\"%2\">&nbsp;%1" ).
#if QT_VERSION < 0x030200
			arg( onlineStatus().description() ).
			arg( iconName ).
			arg( QStyleSheet::escape( displayName() ) );
#else
			arg( onlineStatus().description(), iconName, QStyleSheet::escape( displayName() ) );
#endif
	}
	else
	{
		tip = i18n( "<b>DISPLAY NAME</b> (CONTACT ID)<br><img src=\"%2\">&nbsp;CONTACT STATUS",
			"<b>%4</b> (%3)<br><img src=\"%2\">&nbsp;%1" ).
#if QT_VERSION < 0x030200
			arg( onlineStatus().description() ).
			arg( iconName ).
			arg( QStyleSheet::escape( contactId() ) ).
			arg( QStyleSheet::escape( displayName() ) );
#else
			arg( onlineStatus().description(), iconName, QStyleSheet::escape( contactId() ),
				QStyleSheet::escape( displayName() ) );
#endif
	}

	// --------------------------------------------------------------------------
	// Configurable part of tooltip

	for(QStringList::Iterator it=shownProps.begin(); it!=shownProps.end(); ++it)
	{
		if((*it) == QString::fromLatin1("FormattedName"))
		{
			QString name = formattedName();
			if(!name.isEmpty())
				tip += i18n("<br><b>Full Name:</b>&nbsp;FORMATTED NAME",
					"<br><b>Full Name:</b>&nbsp;<nobr>%1</nobr>").arg(name);
		}
		else if ((*it) == QString::fromLatin1("FormattedIdleTime"))
		{
			QString time = formattedIdleTime();
			if(!time.isEmpty())
				tip += i18n("<br><b>Idle:</b>&nbsp;FORMATTED IDLE TIME",
					"<br><b>Idle:</b>&nbsp;<nobr>%1</nobr>").arg(time);
		}
		else if ((*it) == QString::fromLatin1("homePage"))
		{
			QString url = property(*it).value().toString();
			if(!url.isEmpty())
				url += i18n("<br><b>Home Page:</b>&nbsp;FORMATTED URL",
					"<br><b>Home Page:</b>&nbsp;<nobr><a href=\"%1\">%1</a></nobr>").arg(url).arg(url);
		}
		else
		{
			p = property(*it);
			if(!p.isEmpty())
			{
				QVariant val = p.value();
				QString valueText;
				switch(val.type())
				{
					case QVariant::DateTime:
						valueText = KGlobal::locale()->formatDateTime(val.toDateTime());
						break;
					case QVariant::Date:
						valueText = KGlobal::locale()->formatDate(val.toDate());
						break;
					case QVariant::Time:
						valueText = KGlobal::locale()->formatTime(val.toTime());
						break;
					default:
						valueText = val.toString();
				}

				tip += i18n("<br><b>PROPERTY LABEL:</b>&nbsp;PROPERTY VALUE", "<br><nobr><b>%2:</b></nobr>&nbsp;%1")
					.arg(valueText)
					.arg(p.label());
			}
		}
	}

	return tip;
}

QString KopeteContact::formattedName() const
{
	QString ret;
	KopeteContactProperty first, last;

	first = property(QString::fromLatin1("firstName"));
	last = property(QString::fromLatin1("lastName"));
	if(!first.isEmpty())
	{
		if(!last.isEmpty()) // contact has both first and last name
		{
			ret = i18n("firstName lastName", "%2 %1")
				.arg(last.value().toString())
				.arg(first.value().toString());
		}
		else // only first name set
		{
			ret = first.value().toString();
		}
	}
	else if(!last.isEmpty()) // only last name set
	{
		ret = last.value().toString();
	}

	return ret;
}

QString KopeteContact::formattedIdleTime() const
{
	QString ret;
	unsigned long int leftTime = idleTime();

	if ( leftTime > 0 )
	{	// FIXME: duplicated from code in kopetecontactlistview.cpp
		unsigned long int days, hours, mins, secs;

		days = leftTime / ( 60*60*24 );
		leftTime = leftTime % ( 60*60*24 );
		hours = leftTime / ( 60*60 );
		leftTime = leftTime % ( 60*60 );
		mins = leftTime / 60;
		secs = leftTime % 60;

		if ( days != 0 )
		{
			ret = i18n( "<days>d <hours>h <minutes>m <seconds>s",
				"%4d %3h %2m %1s" )
				.arg( secs )
				.arg( mins )
				.arg( hours )
				.arg( days );
		}
		else if ( hours != 0 )
		{
			ret = i18n( "<hours>h <minutes>m <seconds>s", "%3h %2m %1s" )
				.arg( secs )
				.arg( mins )
				.arg( hours );
		}
		else
		{
			ret = i18n( "<minutes>m <seconds>s", "%2m %1s" )
				.arg( secs )
				.arg( mins );
		}
	}
	return ret;
}

#include "kopetecontact.moc"
// vim: set noet ts=4 sts=4 sw=4:
