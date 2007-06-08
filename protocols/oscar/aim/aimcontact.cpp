/*
  aimcontact.cpp  -  Oscar Protocol Plugin

  Copyright (c) 2003 by Will Stephenson
  Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include <time.h>

#include <qimage.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtextcodec.h>

#include <kapplication.h>
#include <kactionclasses.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kopeteaway.h"
#include "kopetechatsession.h"
#include "kopeteuiglobal.h"
#include "kopetemetacontact.h"

//liboscar
#include "client.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "ssimanager.h"

#include "aimprotocol.h"
#include "aimuserinfo.h"
#include "aimcontact.h"
#include "aimaccount.h"

AIMContact::AIMContact( Kopete::Account* account, const QString& name, Kopete::MetaContact* parent,
                        const QString& icon, const Oscar::SSI& ssiItem )
: OscarContact(account, name, parent, icon, ssiItem )
{
	mProtocol=static_cast<AIMProtocol *>(protocol());
	setOnlineStatus( mProtocol->statusOffline );

	m_infoDialog = 0L;
	m_warnUserAction = 0L;
	mUserProfile="";
	m_haveAwayMessage = false;
	m_mobile = false;
	// Set the last autoresponse time to the current time yesterday
	m_lastAutoresponseTime = QDateTime::currentDateTime().addDays(-1);

	QObject::connect( mAccount->engine(), SIGNAL( receivedUserInfo( const QString&, const UserDetails& ) ),
	                  this, SLOT( userInfoUpdated( const QString&, const UserDetails& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( userIsOffline( const QString& ) ),
	                  this, SLOT( userOffline( const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( receivedAwayMessage( const QString&, const QString& ) ),
	                  this, SLOT( updateAwayMessage( const QString&, const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( receivedProfile( const QString&, const QString& ) ),
	                  this, SLOT( updateProfile( const QString&, const QString& ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( userWarned( const QString&, Q_UINT16, Q_UINT16 ) ),
	                  this, SLOT( gotWarning( const QString&, Q_UINT16, Q_UINT16 ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( haveIconForContact( const QString&, QByteArray ) ),
	                  this, SLOT( haveIcon( const QString&, QByteArray ) ) );
	QObject::connect( mAccount->engine(), SIGNAL( iconServerConnected() ),
	                  this, SLOT( requestBuddyIcon() ) );
	QObject::connect( this, SIGNAL( featuresUpdated() ), this, SLOT( updateFeatures() ) );
}

AIMContact::~AIMContact()
{
}

bool AIMContact::isReachable()
{
	return true;
}

QPtrList<KAction> *AIMContact::customContextMenuActions()
{

	QPtrList<KAction> *actionCollection = new QPtrList<KAction>();
	if ( !m_warnUserAction )
	{
		m_warnUserAction = new KAction( i18n( "&Warn User" ), 0, this, SLOT( warnUser() ), this, "warnAction" );
	}
	m_actionVisibleTo = new KToggleAction(i18n("Always &Visible To"), "", 0,
	                                      this, SLOT(slotVisibleTo()), this, "actionVisibleTo");
	m_actionInvisibleTo = new KToggleAction(i18n("Always &Invisible To"), "", 0,
	                                        this, SLOT(slotInvisibleTo()), this, "actionInvisibleTo");
	
	bool on = account()->isConnected();

	m_warnUserAction->setEnabled( on );

	m_actionVisibleTo->setEnabled(on);
	m_actionInvisibleTo->setEnabled(on);

	SSIManager* ssi = account()->engine()->ssiManager();
	m_actionVisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_VISIBLE ));
	m_actionInvisibleTo->setChecked( ssi->findItem( m_ssiItem.name(), ROSTER_INVISIBLE ));

	actionCollection->append( m_warnUserAction );

	actionCollection->append(m_actionVisibleTo);
	actionCollection->append(m_actionInvisibleTo);


	return actionCollection;
}

const QString AIMContact::awayMessage()
{
	return property(mProtocol->awayMessage).value().toString();
}

void AIMContact::setAwayMessage(const QString &message)
{
	kdDebug(14152) << k_funcinfo <<
		"Called for '" << contactId() << "', away msg='" << message << "'" << endl;
	QString filteredMessage = message;
	filteredMessage.replace(
		QRegExp(QString::fromLatin1("<[hH][tT][mM][lL].*>(.*)</[hH][tT][mM][lL]>")),
		QString::fromLatin1("\\1"));
	filteredMessage.replace(
		QRegExp(QString::fromLatin1("<[bB][oO][dD][yY].*>(.*)</[bB][oO][dD][yY]>")),
		QString::fromLatin1("\\1") );
	QRegExp fontRemover( QString::fromLatin1("<[fF][oO][nN][tT].*>(.*)</[fF][oO][nN][tT]>") );
	fontRemover.setMinimal(true);
	while ( filteredMessage.find( fontRemover ) != -1 )
		filteredMessage.replace( fontRemover, QString::fromLatin1("\\1") );
	setProperty(mProtocol->awayMessage, filteredMessage);
}

int AIMContact::warningLevel() const
{
	return m_warningLevel;
}

void AIMContact::updateSSIItem()
{
	if ( m_ssiItem.type() != 0xFFFF && m_ssiItem.waitingAuth() == false &&
	     onlineStatus() == Kopete::OnlineStatus::Unknown )
	{
		//make sure they're offline
		setOnlineStatus( static_cast<AIMProtocol*>( protocol() )->statusOffline );
	}
}

void AIMContact::slotUserInfo()
{
	if ( !m_infoDialog)
	{
		m_infoDialog = new AIMUserInfoDialog( this, static_cast<AIMAccount*>( account() ), false, Kopete::UI::Global::mainWidget(), 0 );
		if( !m_infoDialog )
			return;
		connect( m_infoDialog, SIGNAL( finished() ), this, SLOT( closeUserInfoDialog() ) );
		m_infoDialog->show();
		if ( mAccount->isConnected() )
		{
			mAccount->engine()->requestAIMProfile( contactId() );
			mAccount->engine()->requestAIMAwayMessage( contactId() );
		}
	}
	else
		m_infoDialog->raise();
}

void AIMContact::userInfoUpdated( const QString& contact, const UserDetails& details )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << contact << endl;

	//if they don't have an SSI alias, make sure we use the capitalization from the
	//server so their contact id looks all pretty.
	QString nickname = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	if ( nickname.isEmpty() || Oscar::normalize( nickname ) == Oscar::normalize( contact ) )
		setNickName( contact );

	( details.userClass() & CLASS_WIRELESS ) ? m_mobile = true : m_mobile = false;

	if ( ( details.userClass() & CLASS_AWAY ) == STATUS_ONLINE )
	{
		if ( m_mobile ) 
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Contact: " << contact << " is mobile-online." << endl;
			setOnlineStatus( mProtocol->statusWirelessOnline );
    	}
		else 
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Contact: " << contact << " is online." << endl;
			setOnlineStatus( mProtocol->statusOnline ); //we're online
		}
		removeProperty( mProtocol->awayMessage );
		m_haveAwayMessage = false;
	}
	else if ( ( details.userClass() & CLASS_AWAY ) ) // STATUS_AWAY
	{
		if ( m_mobile ) 
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Contact: " << contact << " is mobile-away." << endl;
			setOnlineStatus( mProtocol->statusWirelessOnline );
		}
		else 
		{
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Contact: " << contact << " is away." << endl;
			setOnlineStatus( mProtocol->statusAway ); //we're away
		}
		if ( !m_haveAwayMessage ) //prevent cyclic away message requests
		{
			mAccount->engine()->requestAIMAwayMessage( contactId() );
			m_haveAwayMessage = true;
		}
	}
	else
	{
        kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Contact: " << contact << " class " << details.userClass() << " is unhandled... defaulting to away." << endl;
		setOnlineStatus( mProtocol->statusAway ); //we're away
		if ( !m_haveAwayMessage ) //prevent cyclic away message requests
		{
			mAccount->engine()->requestAIMAwayMessage( contactId() );
			m_haveAwayMessage = true;
		}
	}

	if ( details.buddyIconHash().size() > 0 && details.buddyIconHash() != m_details.buddyIconHash() )
	{
        if ( !mAccount->engine()->hasIconConnection() )
            mAccount->engine()->requestServerRedirect( 0x0010 );

		int time = ( KApplication::random() % 10 ) * 1000;
		kdDebug(OSCAR_ICQ_DEBUG) << k_funcinfo << "updating buddy icon in " << time/1000 << " seconds" << endl;
		QTimer::singleShot( time, this, SLOT( requestBuddyIcon() ) );
	}

	OscarContact::userInfoUpdated( contact, details );
}

void AIMContact::userOnline( const QString& userId )
{
	if ( Oscar::normalize( userId ) == Oscar::normalize( contactId() ) )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Getting more contact info" << endl;
		setOnlineStatus( mProtocol->statusOnline );
	}
}

void AIMContact::userOffline( const QString& userId )
{
	if ( Oscar::normalize( userId ) == Oscar::normalize( contactId() ) )
	{
		setOnlineStatus( mProtocol->statusOffline );
		removeProperty( mProtocol->awayMessage );
	}
}

void AIMContact::updateAwayMessage( const QString& contact, const QString& message )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;
	else
	{
		if ( message.isEmpty() )
		{
			removeProperty( mProtocol->awayMessage );
			if ( !m_mobile )
				setOnlineStatus( mProtocol->statusOnline );
			else
				setOnlineStatus( mProtocol->statusWirelessOnline );
			m_haveAwayMessage = false;
		}
		else
		{
			m_haveAwayMessage = true;
			setAwayMessage( message );
			if ( !m_mobile )
				setOnlineStatus( mProtocol->statusAway );
			else
				setOnlineStatus( mProtocol->statusWirelessAway );
		}
	}

	emit updatedProfile();
}

void AIMContact::updateProfile( const QString& contact, const QString& profile )
{
	if ( Oscar::normalize( contact ) != Oscar::normalize( contactId() ) )
		return;

	setProperty( mProtocol->clientProfile, profile );
	emit updatedProfile();
}

void AIMContact::gotWarning( const QString& contact, Q_UINT16 increase, Q_UINT16 newLevel )
{
	//somebody just got bitchslapped! :O
	Q_UNUSED( increase );
	if ( Oscar::normalize( contact ) == Oscar::normalize( contactId() ) )
		m_warningLevel = newLevel;

	//TODO add a KNotify event after merge to HEAD
}

void AIMContact::requestBuddyIcon()
{
	kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Updating buddy icon for " << contactId() << endl;
	if ( m_details.buddyIconHash().size() > 0 )
	{
		account()->engine()->requestBuddyIcon( contactId(), m_details.buddyIconHash(),
		                                       m_details.iconCheckSumType() );
	}
}

void AIMContact::haveIcon( const QString& user, QByteArray icon )
{
	if ( Oscar::normalize( user ) != Oscar::normalize( contactId() ) )
		return;

	kdDebug(OSCAR_AIM_DEBUG) << k_funcinfo << "Updating icon for " << contactId() << endl;
	QImage buddyIcon( icon );
	if ( buddyIcon.isNull() )
	{
		kdWarning(OSCAR_AIM_DEBUG) << k_funcinfo << "Failed to convert buddy icon to QImage" << endl;
		return;
	}

	setProperty( Kopete::Global::Properties::self()->photo(), buddyIcon );
}

void AIMContact::closeUserInfoDialog()
{
	m_infoDialog->delayedDestruct();
	m_infoDialog = 0L;
}

void AIMContact::warnUser()
{
	QString nick = property( Kopete::Global::Properties::self()->nickName() ).value().toString();
	QString message = i18n( "<qt>Would you like to warn %1 anonymously or with your name?<br>" \
	                        "(Warning a user on AIM will result in a \"Warning Level\"" \
	                        " increasing for the user you warn. Once this level has reached a" \
	                        " certain point, they will not be able to sign on. Please do not abuse" \
	                        " this function, it is meant for legitimate practices.)</qt>" ).arg( nick );


	int result = KMessageBox::questionYesNoCancel( Kopete::UI::Global::mainWidget(), message,
	                                               i18n( "Warn User %1?" ).arg( nick ),
	                                               i18n( "Warn Anonymously" ), i18n( "Warn" ) );

	if ( result == KMessageBox::Yes )
		mAccount->engine()->sendWarning( contactId(), true);
	else if ( result == KMessageBox::No )
		mAccount->engine()->sendWarning( contactId(), false);
}

void AIMContact::slotVisibleTo()
{
	account()->engine()->setVisibleTo( contactId(), m_actionVisibleTo->isChecked() );
}

void AIMContact::slotInvisibleTo()
{
	account()->engine()->setInvisibleTo( contactId(), m_actionInvisibleTo->isChecked() );
}

void AIMContact::slotSendMsg(Kopete::Message& message, Kopete::ChatSession *)
{
	Oscar::Message msg;
	QString s;

	if (message.plainBody().isEmpty()) // no text, do nothing
		return;
	//okay, now we need to change the message.escapedBody from real HTML to aimhtml.
	//looking right now for docs on that "format".
	//looks like everything except for alignment codes comes in the format of spans

	//font-style:italic -> <i>
	//font-weight:600 -> <b> (anything > 400 should be <b>, 400 is not bold)
	//text-decoration:underline -> <u>
	//font-family: -> <font face="">
	//font-size:xxpt -> <font ptsize=xx>

	s=message.escapedBody();
	s.replace ( QRegExp( QString::fromLatin1("<span style=\"([^\"]*)\">([^<]*)</span>")),
			QString::fromLatin1("<style>\\1;\"\\2</style>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-style:italic;([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<i><style>\\1\\2\"\\3</style></i>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-weight:600;([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<b><style>\\1\\2\"\\3</style></b>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)text-decoration:underline;([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<u><style>\\1\\2\"\\3</style></u>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-family:([^;]*);([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<font face=\"\\2\"><style>\\1\\3\"\\4</style></font>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)font-size:([^p]*)pt;([^\"]*)\"([^<]*)</style>")),
				QString::fromLatin1("<font ptsize=\"\\2\"><style>\\1\\3\"\\4</style></font>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)color:([^;]*);([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("<font color=\"\\2\"><style>\\1\\3\"\\4</style></font>"));

	s.replace ( QRegExp( QString::fromLatin1("<style>([^\"]*)\"([^<]*)</style>")),
	            QString::fromLatin1("\\2"));

	//okay now change the <font ptsize="xx"> to <font size="xx">

	//0-9 are size 1
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"\\d\">")),
	            QString::fromLatin1("<font size=\"1\">"));
	//10-11 are size 2
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"1[01]\">")),
	            QString::fromLatin1("<font size=\"2\">"));
	//12-13 are size 3
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"1[23]\">")),
	            QString::fromLatin1("<font size=\"3\">"));
	//14-16 are size 4
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"1[456]\">")),
	            QString::fromLatin1("<font size=\"4\">"));
	//17-22 are size 5
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"(?:1[789]|2[012])\">")),
	            QString::fromLatin1("<font size=\"5\">"));
	//23-29 are size 6
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"2[3456789]\">")),QString::fromLatin1("<font size=\"6\">"));
	//30- (and any I missed) are size 7
	s.replace ( QRegExp ( QString::fromLatin1("<font ptsize=\"[^\"]*\">")),QString::fromLatin1("<font size=\"7\">"));

	s.replace ( QRegExp ( QString::fromLatin1("<br[ /]*>")), QString::fromLatin1("<br>") );

	// strip left over line break
	s.remove( QRegExp( QString::fromLatin1( "<br>$" ) ) );

	kdDebug(14190) << k_funcinfo << "sending "
		<< s << endl;

	// XXX Need to check for message size?

	if ( m_details.hasCap( CAP_UTF8 ) )
		msg.setText( Oscar::Message::UCS2, s );
	else
		msg.setText( Oscar::Message::UserDefined, s, contactCodec() );

	msg.setReceiver(mName);
	msg.setTimestamp(message.timestamp());
	msg.setType(0x01);

	mAccount->engine()->sendMessage(msg);

	// Show the message we just sent in the chat window
	manager(Kopete::Contact::CanCreate)->appendMessage(message);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void AIMContact::updateFeatures()
{
	setProperty( static_cast<AIMProtocol*>(protocol())->clientFeatures, m_clientFeatures );
}

void AIMContact::sendAutoResponse(Kopete::Message& msg)
{
	// The target time is 2 minutes later than the last message
	int delta = m_lastAutoresponseTime.secsTo( QDateTime::currentDateTime() );
	kdDebug(14152) << k_funcinfo << "Last autoresponse time: " << m_lastAutoresponseTime << endl;
	kdDebug(14152) << k_funcinfo << "Current time: " << QDateTime::currentDateTime() << endl;
	kdDebug(14152) << k_funcinfo << "Difference: " << delta << endl;
	// Check to see if we're past that time
	if(delta > 120)
	{
		kdDebug(14152) << k_funcinfo << "Sending auto response" << endl;

		// This code was yoinked straight from OscarContact::slotSendMsg()
		// If only that slot wasn't private, but I'm not gonna change it right now.
		Oscar::Message message;

		if ( m_details.hasCap( CAP_UTF8 ) )
		{
			message.setText( Oscar::Message::UCS2, msg.plainBody() );
		}
		else
		{
			QTextCodec* codec = contactCodec();
			message.setText( Oscar::Message::UserDefined, msg.plainBody(), codec );
		}

		message.setTimestamp( msg.timestamp() );
		message.setSender( mAccount->accountId() );
		message.setReceiver( mName );
		message.setType( 0x01 );

		// isAuto defaults to false
		mAccount->engine()->sendMessage( message, true);
		kdDebug(14152) << k_funcinfo << "Sent auto response" << endl;
		manager(Kopete::Contact::CanCreate)->appendMessage(msg);
		manager(Kopete::Contact::CanCreate)->messageSucceeded();
		// Update the last autoresponse time
		m_lastAutoresponseTime = QDateTime::currentDateTime();
	}
	else
	{
		kdDebug(14152) << k_funcinfo << "Not enough time since last autoresponse, NOT sending" << endl;
	}
}
#include "aimcontact.moc"
//kate: tab-width 4; indent-mode csands;
