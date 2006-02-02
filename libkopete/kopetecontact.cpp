/*
    kopetecontact.cpp - Kopete Contact

    Copyright (c) 2002-2004 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart @tiscalinet.be>

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

#include <qapplication.h>

#include <kdebug.h>

#include <kdeversion.h>
#include <kinputdialog.h>

#include <kabcpersistence.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <klistviewsearchline.h>

#include "kopetecontactlist.h"
#include "kopeteglobal.h"
#include "kopeteuiglobal.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetestdaction.h"
#include "kopetechatsession.h"
#include "kopeteview.h"
#include "kopetemetacontact.h"
#include "kopeteprefs.h"
#include "metacontactselectorwidget.h"
#include "kopeteemoticons.h"

//For the moving to another metacontact dialog
#include <qlabel.h>
#include <qimage.h>
#include <qmime.h>
#include <qvbox.h>
#include <klistview.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qstylesheet.h>

namespace Kopete {

struct Contact::Private
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

	Kopete::ContactProperty::Map properties;

};

Contact::Contact( Account *account, const QString &contactId,
	MetaContact *parent, const QString &icon )
	: QObject( parent )
{
	d = new Private;

	//kdDebug( 14010 ) << k_funcinfo << "Creating contact with id " << contactId << endl;

	d->contactId = contactId;
	d->metaContact = parent;
	d->fileCapable = false;
	d->account = account;
	d->idleTime = 0;
	d->icon = icon;

	// If can happend that a MetaContact may be used without a account
	// (ex: for unit tests or chat window style preview)
	if ( account )
	{
		account->registerContact( this );
		connect( account, SIGNAL( isConnectedChanged() ), SLOT( slotAccountIsConnectedChanged() ) );
	}

	// Need to check this because myself() may have no parent
	// Maybe too the metaContact doesn't have a valid protocol() 
	// (ex: for unit tests or chat window style preview)
	if( parent && protocol() )
	{
		connect( parent, SIGNAL( aboutToSave( Kopete::MetaContact * ) ),
			protocol(), SLOT( slotMetaContactAboutToSave( Kopete::MetaContact * ) ) );

		parent->addContact( this );
	}

	
}

Contact::~Contact()
{
	//kdDebug(14010) << k_funcinfo << endl;
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

	OnlineStatus oldStatus = d->onlineStatus;
	d->onlineStatus = status;

	Kopete::Global::Properties *globalProps = Kopete::Global::Properties::self();

	// Contact changed from Offline to another status
	if( oldStatus.status() == OnlineStatus::Offline &&
		status.status() != OnlineStatus::Offline )
	{
		setProperty( globalProps->onlineSince(), QDateTime::currentDateTime() );
		/*kdDebug(14010) << k_funcinfo << "REMOVING lastSeen property for " <<
			d->displayName << endl;*/
		removeProperty( globalProps->lastSeen() );
	}
	else if( oldStatus.status() != OnlineStatus::Offline &&
		oldStatus.status() != OnlineStatus::Unknown &&
		status.status() == OnlineStatus::Offline ) // Contact went back offline
	{
		removeProperty( globalProps->onlineSince() );
		/*kdDebug(14010) << k_funcinfo << "SETTING lastSeen property for " <<
			d->displayName << endl;*/
		setProperty( globalProps->lastSeen(), QDateTime::currentDateTime() );
	}

	if ( this == account()->myself() || account()->isConnected() )
		emit onlineStatusChanged( this, status, oldStatus );
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


void Contact::sendFile( const KURL &, const QString &, uint )
{
	kdWarning( 14010 ) << k_funcinfo << "Plugin "
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

KPopupMenu* Contact::popupMenu( ChatSession *manager )
{
	// Build the menu
	KPopupMenu *menu = new KPopupMenu();

	// insert title
	QString titleText;
	QString nick = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if( nick.isEmpty() )
		titleText = QString::fromLatin1( "%1 (%2)" ).arg( contactId(), onlineStatus().description() );
	else
		titleText = QString::fromLatin1( "%1 <%2> (%3)" ).arg( nick, contactId(), onlineStatus().description() );
	menu->insertTitle( titleText );

	if( metaContact() && metaContact()->isTemporary() && contactId() != account()->myself()->contactId() )
	{
		KAction *actionAddContact = new KAction( i18n( "&Add to Your Contact List" ), QString::fromLatin1( "add_user" ),
		                                         0, this, SLOT( slotAddContact() ), menu, "actionAddContact" );
		actionAddContact->plug( menu );
		menu->insertSeparator();
	}

	// FIXME: After KDE 3.2 we should make isReachable do the isConnected call so it can be removed here - Martijn
	bool reach = account()->isConnected() && isReachable();
	bool myself = (this == account()->myself());

	KAction *actionSendMessage = KopeteStdAction::sendMessage( this, SLOT( sendMessage() ), menu, "actionSendMessage" );
	actionSendMessage->setEnabled( reach && !myself );
	actionSendMessage->plug( menu );

	KAction *actionChat = KopeteStdAction::chat( this, SLOT( startChat() ), menu, "actionChat" );
	actionChat->setEnabled( reach && !myself );
	actionChat->plug( menu );

	KAction *actionSendFile = KopeteStdAction::sendFile( this, SLOT( sendFile() ), menu, "actionSendFile" );
	actionSendFile->setEnabled( reach && d->fileCapable && !myself );
	actionSendFile->plug( menu );

	// Protocol specific options will go below this separator
	// through the use of the customContextMenuActions() function

	// Get the custom actions from the protocols ( pure virtual function )
	QPtrList<KAction> *customActions = customContextMenuActions( manager );
	if( customActions && !customActions->isEmpty() )
	{
		menu->insertSeparator();

		for( KAction *a = customActions->first(); a; a = customActions->next() )
			a->plug( menu );
	}
	delete customActions;

	menu->insertSeparator();

	if( metaContact() && !metaContact()->isTemporary() )
		KopeteStdAction::changeMetaContact( this, SLOT( changeMetaContact() ), menu, "actionChangeMetaContact" )->plug( menu );

	KopeteStdAction::contactInfo( this, SLOT( slotUserInfo() ), menu, "actionUserInfo" )->plug( menu );

#if 0 //this is not fully implemented yet (and doesn't work).  disable for now   - Olivier 2005-01-11
	if ( account()->isBlocked( d->contactId ) )
		KopeteStdAction::unblockContact( this, SLOT( slotUnblock() ), menu, "actionUnblockContact" )->plug( menu );
	else
		KopeteStdAction::blockContact( this, SLOT( slotBlock() ), menu, "actionBlockContact" )->plug( menu );
#endif

	if( metaContact() && !metaContact()->isTemporary() )
		KopeteStdAction::deleteContact( this, SLOT( slotDelete() ), menu, "actionDeleteContact" )->plug( menu );

	return menu;
}

void Contact::changeMetaContact()
{
	KDialogBase *moveDialog = new KDialogBase( Kopete::UI::Global::mainWidget(), "moveDialog", true, i18n( "Move Contact" ),
		KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );

	QVBox *w = new QVBox( moveDialog );
	w->setSpacing( KDialog::spacingHint() );
	Kopete::UI::MetaContactSelectorWidget *selector = new Kopete::UI::MetaContactSelectorWidget(w);
	selector->setLabelMessage(i18n( "Select the meta contact to which you want to move this contact:" ));
	// exclude this metacontact as a target metacontact for the move
	selector->excludeMetaContact( metaContact() );
	QCheckBox *chkCreateNew = new QCheckBox( i18n( "Create a new metacontact for this contact" ), w );
	QWhatsThis::add( chkCreateNew , i18n( "If you select this option, a new metacontact will be created in the top-level group "
		"with the name of this contact and the contact will be moved to it." ) );
	QObject::connect( chkCreateNew , SIGNAL( toggled(bool) ) ,  selector , SLOT ( setDisabled(bool) ) ) ;

	moveDialog->setMainWidget(w);
	if( moveDialog->exec() == QDialog::Accepted )
	{
		Kopete::MetaContact *mc = selector->metaContact();
		if(chkCreateNew->isChecked())
		{
			mc=new Kopete::MetaContact();
			Kopete::ContactList::self()->addMetaContact(mc);
		}
		if( mc )
		{
			setMetaContact( mc );
		}
	}

	moveDialog->deleteLater();
}

void Contact::setMetaContact( MetaContact *m )
{
	MetaContact *old = d->metaContact;
	if(old==m) //that make no sens
		return;

	if( old )
	{
		int result=KMessageBox::No;
		if( old->isTemporary() )
			result=KMessageBox::Yes;
		else if( old->contacts().count()==1 )
		{ //only one contact, including this one, that mean the contact will be empty efter the move
			result = KMessageBox::questionYesNoCancel( Kopete::UI::Global::mainWidget(), i18n( "You are moving the contact `%1' to the meta contact `%2'.\n"
				"`%3' will be empty afterwards. Do you want to delete this contact?" )
					.arg(contactId(), m ? m->displayName() : QString::null, old->displayName())
				, i18n( "Move Contact" ), KStdGuiItem::del(), i18n( "&Keep" ) , QString::fromLatin1("delete_old_contact_when_move") );
			if(result==KMessageBox::Cancel)
				return;
		}
		old->removeContact( this );
		disconnect( old, SIGNAL( aboutToSave( Kopete::MetaContact * ) ),
			protocol(), SLOT( slotMetaContactAboutToSave( Kopete::MetaContact * ) ) );

		if(result==KMessageBox::Yes)
		{
			//remove the old metacontact.  (this delete the MC)
			ContactList::self()->removeMetaContact(old);
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
		// it is necessary to call this write here, because MetaContact::addContact() does not differentiate
		// between adding completely new contacts (which should be written to kabc) and restoring upon restart
		// (where no write is needed).
		KABCPersistence::self()->write( m );
		connect( d->metaContact, SIGNAL( aboutToSave( Kopete::MetaContact * ) ),
		protocol(), SLOT( slotMetaContactAboutToSave( Kopete::MetaContact * ) ) );
	}
	sync();
}

void Contact::serialize( QMap<QString, QString> &/*serializedData*/,
	QMap<QString, QString> & /* addressBookData */ )
{
}


void Contact::serializeProperties(QMap<QString, QString> &serializedData)
{

	Kopete::ContactProperty::Map::ConstIterator it;// = d->properties.ConstIterator;
	for (it=d->properties.begin(); it != d->properties.end(); ++it)
	{
		if (!it.data().tmpl().persistent())
			continue;

		QVariant val = it.data().value();
		QString key = QString::fromLatin1("prop_%1_%2").arg(QString::fromLatin1(val.typeName()), it.key());

		serializedData[key] = val.toString();

	} // end for()
} // end serializeProperties()

void Contact::deserializeProperties(
	QMap<QString, QString> &serializedData )
{
	QMap<QString, QString>::ConstIterator it;
	for ( it=serializedData.begin(); it != serializedData.end(); ++it )
	{
		QString key = it.key();

		if ( !key.startsWith( QString::fromLatin1("prop_") ) ) // avoid parsing other serialized data
			continue;

		QStringList keyList = QStringList::split( QChar('_'), key, false );
		if( keyList.count() < 3 ) // invalid key, not enough parts in string "prop_X_Y"
			continue;

		key = keyList[2]; // overwrite key var with the real key name this property has
		QString type( keyList[1] ); // needed for QVariant casting

		QVariant variant( it.data() );
		if( !variant.cast(QVariant::nameToType(type.latin1())) )
		{
			kdDebug(14010) << k_funcinfo <<
				"Casting QVariant to needed type FAILED" <<
				"key=" << key << ", type=" << type << endl;
			continue;
		}

		Kopete::ContactPropertyTmpl tmpl = Kopete::Global::Properties::self()->tmpl(key);
		if( tmpl.isNull() )
		{
			kdDebug( 14010 ) << k_funcinfo << "no ContactPropertyTmpl defined for" \
				" key " << key << ", cannot restore persistent property" << endl;
			continue;
		}

		setProperty(tmpl, variant);
	} // end for()
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
		KopeteView *v=manager( CanCreate )->view(true, KopetePrefs::prefs()->interfacePreference() );
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
		i18n( "Are you sure you want to remove the contact  '%1' from your contact list?" ).
		arg( d->contactId ), i18n( "Remove Contact" ), KGuiItem(i18n("Remove"), QString::fromLatin1("delete_user") ),
		QString::fromLatin1("askRemoveContact"), KMessageBox::Notify | KMessageBox::Dangerous )
		== KMessageBox::Continue )
	{
		deleteContact();
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

QPtrList<KAction> *Contact::customContextMenuActions()
{
	return 0L;
}

QPtrList<KAction> *Contact::customContextMenuActions( ChatSession * /* manager */ )
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
	d->fileCapable = filecap;
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


QStringList Contact::properties() const
{
	return d->properties.keys();
}

bool Contact::hasProperty(const QString &key) const
{
	return d->properties.contains(key);
}

const ContactProperty &Contact::property(const QString &key) const
{
	if(hasProperty(key))
		return d->properties[key];
	else
		return Kopete::ContactProperty::null;
}

const Kopete::ContactProperty &Contact::property(
	const Kopete::ContactPropertyTmpl &tmpl) const
{
	if(hasProperty(tmpl.key()))
		return d->properties[tmpl.key()];
	else
		return Kopete::ContactProperty::null;
}


void Contact::setProperty(const Kopete::ContactPropertyTmpl &tmpl,
	const QVariant &value)
{
	if(tmpl.isNull() || tmpl.key().isEmpty())
	{
		kdDebug(14000) << k_funcinfo <<
			"No valid template for property passed!" << endl;
		return;
	}

	if(value.isNull() || value.canCast(QVariant::String) && value.toString().isEmpty())
	{
		removeProperty(tmpl);
	}
	else
	{
		QVariant oldValue = property(tmpl.key()).value();

		if(oldValue != value)
		{
			Kopete::ContactProperty prop(tmpl, value);
			d->properties.insert(tmpl.key(), prop, true);

			emit propertyChanged(this, tmpl.key(), oldValue, value);
		}
	}
}

void Contact::removeProperty(const Kopete::ContactPropertyTmpl &tmpl)
{
	if(!tmpl.isNull() && !tmpl.key().isEmpty())
	{

		QVariant oldValue = property(tmpl.key()).value();
		d->properties.remove(tmpl.key());
		emit propertyChanged(this, tmpl.key(), oldValue, QVariant());
	}
}


QString Contact::toolTip() const
{
	Kopete::ContactProperty p;
	QString tip;
	QStringList shownProps = KopetePrefs::prefs()->toolTipContents();

	// --------------------------------------------------------------------------
	// Fixed part of tooltip

	QString iconName = QString::fromLatin1("kopete-contact-icon:%1:%2:%3")
		.arg( KURL::encode_string( protocol()->pluginId() ),
				KURL::encode_string( account()->accountId() ),
				KURL::encode_string( contactId() ) );

	// TODO:  the nickname should be a configurable properties, like others. -Olivier
	QString nick = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if ( nick.isEmpty() )
	{
		tip = i18n( "<b>DISPLAY NAME</b><br><img src=\"%2\">&nbsp;CONTACT STATUS",
			"<b><nobr>%3</nobr></b><br><img src=\"%2\">&nbsp;%1" ).
			arg( Kopete::Message::escape( onlineStatus().description() ), iconName,
				Kopete::Message::escape( d->contactId ) );
	}
	else
	{
		tip = i18n( "<b>DISPLAY NAME</b> (CONTACT ID)<br><img src=\"%2\">&nbsp;CONTACT STATUS",
			"<nobr><b>%4</b> (%3)</nobr><br><img src=\"%2\">&nbsp;%1" ).
				arg( Kopete::Message::escape( onlineStatus().description() ), iconName,
					Kopete::Message::escape( contactId() ),
					Kopete::Emoticons::parseEmoticons( Kopete::Message::escape( nick ) ) );
	}

	// --------------------------------------------------------------------------
	// Configurable part of tooltip

	for(QStringList::Iterator it=shownProps.begin(); it!=shownProps.end(); ++it)
	{
		if((*it) == QString::fromLatin1("FormattedName"))
		{
			QString name = formattedName();
			if(!name.isEmpty())
			{
				tip += i18n("<br><b>Full Name:</b>&nbsp;FORMATTED NAME",
							"<br><b>Full Name:</b>&nbsp;<nobr>%1</nobr>").arg(QStyleSheet::escape(name));
			}
		}
		else if ((*it) == QString::fromLatin1("FormattedIdleTime"))
		{
			QString time = formattedIdleTime();
			if(!time.isEmpty())
			{
				tip += i18n("<br><b>Idle:</b>&nbsp;FORMATTED IDLE TIME",
					"<br><b>Idle:</b>&nbsp;<nobr>%1</nobr>").arg(time);
			}
		}
		else if ((*it) == QString::fromLatin1("homePage"))
		{
			QString url = property(*it).value().toString();
			if(!url.isEmpty())
			{
				tip += i18n("<br><b>Home Page:</b>&nbsp;FORMATTED URL",
					"<br><b>Home Page:</b>&nbsp;<a href=\"%1\"><nobr>%2</nobr></a>").
						arg( KURL::encode_string( url ), Kopete::Message::escape( QStyleSheet::escape(url) ) );
			}
		}
		else if ((*it) == QString::fromLatin1("awayMessage"))
		{
			QString awaymsg = property(*it).value().toString();
			if(!awaymsg.isEmpty())
			{
				tip += i18n("<br><b>Away Message:</b>&nbsp;FORMATTED AWAY MESSAGE",
							"<br><b>Away&nbsp;Message:</b>&nbsp;%1").arg ( Kopete::Emoticons::parseEmoticons( Kopete::Message::escape(awaymsg) ) );
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

				tip += i18n("<br><b>PROPERTY LABEL:</b>&nbsp;PROPERTY VALUE",
					"<br><nobr><b>%2:</b></nobr>&nbsp;%1").
						arg( valueText, QStyleSheet::escape(p.tmpl().label()) );
			}
		}
	}

	return tip;
}

QString Kopete::Contact::formattedName() const
{
	if( hasProperty(QString::fromLatin1("FormattedName")) )
		return property(QString::fromLatin1("FormattedName")).value().toString();

	QString ret;
	Kopete::ContactProperty first, last;

	first = property(QString::fromLatin1("firstName"));
	last = property(QString::fromLatin1("lastName"));
	if(!first.isNull())
	{
		if(!last.isNull()) // contact has both first and last name
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
	QString nick = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if( !nick.isEmpty() )
		return nick;

	return contactId();
}

void Contact::virtual_hook( uint , void * )
{ }


} //END namespace Kopete


#include "kopetecontact.moc"


