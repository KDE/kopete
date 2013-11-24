
/*
    yahoocontact.cpp - Yahoo Contact

    Copyright (c) 2003-2004 by Matt Rogers <matt.rogers@kdemail.net>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Portions based on code by Bruno Rodrigues <bruno.rodrigues@litux.org>

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "yahoocontact.h"

#include "kopetegroup.h"
#include "kopetechatsession.h"
#include "kopeteonlinestatus.h"
#include "kopetemetacontact.h"
#include "kopetechatsessionmanager.h"
#include "kopeteuiglobal.h"
#include "kopeteview.h"
#include "kopetetransfermanager.h"
#include "kopeteavatarmanager.h"

// Local Includes
#include "yahooaccount.h"
#include "client.h"
#include "yahoowebcamdialog.h"
#include "ui_yahoostealthsetting.h"
#include "yahoochatsession.h"
#include "yabentry.h"
#include "yahoouserinfodialog.h"
#include "sendfiletask.h"

// QT Includes
#include <qregexp.h>
#include <qfile.h>
#include <qradiobutton.h>
#include <QPointer>

// KDE Includes
#include <kdebug.h>
#include <kaction.h>
#include <KActionCollection>
#include <kdialog.h>
#include <klocale.h>
#include <krun.h>
#include <kshortcut.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kimageio.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <ktoolinvocation.h>
#include <kicon.h>

YahooContact::YahooContact( YahooAccount *account, const QString &userId, const QString &fullName, Kopete::MetaContact *metaContact )
	: Kopete::Contact( account, userId, metaContact )
{
	//kDebug(YAHOO_GEN_DEBUG) ;

	m_userId = userId;
	if ( metaContact )
		m_groupName = metaContact->groups().first()->displayName();
	m_manager = 0L;
	m_account = account;
	m_YABEntry = 0L;
	m_receivingWebcam = false;
	m_sessionActive = false;

	// Update ContactList
	setNickName( fullName );
	setOnlineStatus( static_cast<YahooProtocol*>( m_account->protocol() )->Offline );
	setFileCapable( true );

	if ( m_account->haveContactList() )
		syncToServer();

	m_webcamDialog = 0L;
	m_webcamAction = 0L;
	m_stealthAction = 0L;
	m_inviteWebcamAction = 0L;
	m_inviteConferenceAction = 0L;
	m_profileAction = 0L;

	m_buzzAction = 0L;
}

YahooContact::~YahooContact()
{
	delete m_YABEntry;
	m_YABEntry = 0L;
}

QString YahooContact::userId() const
{
	return m_userId;
}

void YahooContact::setOnlineStatus(const Kopete::OnlineStatus &status)
{
	bool isStealthed = stealthed();
	if( isStealthed && status.internalStatus() <= 999)	// Not Stealted -> Stealthed
	{
		Contact::setOnlineStatus(
			Kopete::OnlineStatus(status.status() ,
			(status.weight()==0) ? 0 : (status.weight() -1)  ,
			protocol() ,
			status.internalStatus()+1000 ,
			status.overlayIcons() + QStringList("yahoo_stealthed") ,
			i18n("%1|Stealthed", status.description() ) ) );
	}
	else if( !isStealthed && status.internalStatus() > 999 )// Stealthed -> Not Stealthed
		Contact::setOnlineStatus( static_cast< YahooProtocol *>( protocol() )->statusFromYahoo( status.internalStatus() - 1000 ) );
	else
		Contact::setOnlineStatus( status );

	if( status.status() == Kopete::OnlineStatus::Offline )
		setStatusMessage( Kopete::StatusMessage() );
}

void YahooContact::updateStealthed()
{
	setOnlineStatus( onlineStatus() );
}

bool YahooContact::stealthed() const
{
	return (m_account->yahooSession()->stealthStatus( m_userId ) == Yahoo::StealthActive );
}

void YahooContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData)
{
	//kDebug(YAHOO_GEN_DEBUG) ;

	Kopete::Contact::serialize(serializedData, addressBookData);
}

void YahooContact::syncToServer()
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if(!m_account->isConnected()) return;

	if ( !m_account->isOnServer(m_userId) && !metaContact()->isTemporary() )
	{	kDebug(YAHOO_GEN_DEBUG) << "Contact " << m_userId << " doesn't exist on server-side. Adding...";

		Kopete::GroupList groupList = metaContact()->groups();
		foreach(Kopete::Group *g, groupList)
			m_account->yahooSession()->addBuddy(m_userId, g->displayName() );
	}
}

void YahooContact::sync(unsigned int flags)
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if ( !m_account->isConnected() )
		return;

	if ( !m_account->isOnServer( contactId() ) )
	{
		//TODO: Share this code with the above function
		kDebug(YAHOO_GEN_DEBUG) << "Contact isn't on the server. Adding...";
		Kopete::GroupList groupList = metaContact()->groups();
		foreach(Kopete::Group *g, groupList)
			m_account->yahooSession()->addBuddy(m_userId, g->displayName() );
	}
	else
	{
		QString newGroup = metaContact()->groups().first()->displayName();
		if ( flags & Kopete::Contact::MovedBetweenGroup )
		{
			kDebug(YAHOO_GEN_DEBUG) << "contact changed groups. moving on server";
			m_account->yahooSession()->moveBuddy( contactId(), m_groupName, newGroup );
			m_groupName = newGroup;
		}
	}
}


bool YahooContact::isOnline() const
{
	//kDebug(YAHOO_GEN_DEBUG) ;
	return onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown;
}

bool YahooContact::isReachable()
{
	//kDebug(YAHOO_GEN_DEBUG) ;
	if ( m_account->isConnected() )
		return true;
	else
		return false;
}

Kopete::ChatSession *YahooContact::manager( Kopete::Contact::CanCreateFlags canCreate )
{
	if( !m_manager && canCreate)
	{
		Kopete::ContactPtrList m_them;
		m_them.append( this );
		m_manager = new YahooChatSession( protocol(), account()->myself(), m_them  );
		connect( m_manager, SIGNAL(destroyed()), this, SLOT(slotChatSessionDestroyed()) );
		connect( m_manager, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)), this, SLOT(slotSendMessage(Kopete::Message&)) );
		connect( m_manager, SIGNAL(myselfTyping(bool)), this, SLOT(slotTyping(bool)) );
		connect( m_account, SIGNAL(receivedTypingMsg(QString,bool)), m_manager, SLOT(receivedTypingMsg(QString,bool)) );
		connect( this, SIGNAL(displayPictureChanged()), m_manager, SLOT(slotDisplayPictureChanged()));
	}

	return m_manager;
}

QString YahooContact::prepareMessage( const QString &messageText )
{
	// Yahoo does not understand XML/HTML message data, so send plain text
	// instead.  (Yahoo has its own format for "rich text".)
	QString newMsg( messageText );
	QRegExp regExp;
	int pos = 0;
	regExp.setMinimal( true );
	// find and replace <p>-formattings
	regExp.setPattern( "<p style=\"([^\"]*)\">(.*)</p>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("\\2" ) );
		}
	}


	// find and replace Bold-formattings
	regExp.setPattern( "<span([^>]*)font-weight:600([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("<span\\1font-weight:600\\2>\033[1m\\3\033[x1m</span>" ) );
		}
	}

	// find and replace Underline-formattings
	regExp.setPattern( "<span([^>]*)text-decoration:underline([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("<span\\1text-decoration:underline\\2>\033[4m\\3\033[x4m</span>" ) );
		}
	}

	// find and replace Italic-formattings
	regExp.setPattern( "<span([^>]*)font-style:italic([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("<span\\1font-style:italic\\2>\033[2m\\3\033[x2m</span>" ) );
		}
	}

	// find and remove background color formattings (not supported)
	regExp.setPattern( "<span([^>]*)background-color:#([0-9a-zA-Z]*)([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("<span\\1\\3>\\4</span>" ) );
		}
	}

	// find and replace Color-formattings
	regExp.setPattern( "<span([^>]*)color:#([0-9a-zA-Z]*)([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("<span\\1\\3>\033[#\\2m\\4\033[#000000m</span>" ) );
		}
	}

	// find and replace Font-formattings
	regExp.setPattern( "<span([^>]*)font-family:([^;\"]*)([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("<span\\1\\3><font face=\"\\2\">\\4</span>" ) );
		}
	}

	// find and replace Size-formattings
	regExp.setPattern( "<span([^>]*)font-size:([0-9]*)pt([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("<span\\1\\3><font size=\"\\2\">\\4</span>" ) );
		}
	}

	// remove span-tags
	regExp.setPattern( "<span([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("\\2") );
		}
	}

	// remove p-tags
	regExp.setPattern( "<p([^>]*)>(.*)</p>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.indexIn( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			newMsg.replace( regExp, QLatin1String("\\2") );
		}
	}

	// convert escaped chars
	newMsg.replace( QLatin1String( "&gt;" ), QLatin1String( ">" ) );
	newMsg.replace( QLatin1String( "&lt;" ), QLatin1String( "<" ) );
	newMsg.replace( QLatin1String( "&quot;" ), QLatin1String( "\"" ) );
	newMsg.replace( QLatin1String( "&nbsp;" ), QLatin1String( " " ) );
	newMsg.replace( QLatin1String( "&amp;" ), QLatin1String( "&" ) );
	newMsg.replace( QRegExp( QString::fromLatin1("<br[ /]*>") ), QLatin1String( "\r" ) );

	return newMsg;
}

void YahooContact::slotSendMessage( Kopete::Message &message )
{
	kDebug(YAHOO_GEN_DEBUG) ;

	QString messageText = message.escapedBody();
	kDebug(YAHOO_GEN_DEBUG) << "Original message: " << messageText;
	messageText = prepareMessage( messageText );
	kDebug(YAHOO_GEN_DEBUG) << "Converted message: " << messageText;

	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
	Kopete::Contact *target = m_them.first();

	if( !m_sessionActive )			// Register a new chatsession
	{
		m_account->yahooSession()->setChatSessionState( m_userId, false );
		m_sessionActive = true;
	}

	m_account->yahooSession()->sendMessage( static_cast<YahooContact *>(target)->m_userId, messageText );

	// append message to window
	manager(Kopete::Contact::CanCreate)->appendMessage(message);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void YahooContact::sendFile( const KUrl &sourceURL, const QString &fileName, uint fileSize )
{
	Kopete::TransferManager::transferManager()->sendFile( sourceURL, fileName, fileSize,
			false, this, SLOT(slotSendFile(KUrl)) );
}

void YahooContact::slotTyping(bool isTyping_ )
{
	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
	Kopete::Contact *target = m_them.first();


	m_account->yahooSession()->sendTyping( static_cast<YahooContact*>(target)->m_userId, isTyping_ );
}

void YahooContact::slotChatSessionDestroyed()
{
	m_manager = 0L;
	m_account->yahooSession()->setChatSessionState( m_userId, true );	// Unregister chatsession
	m_sessionActive = false;
}

QList<KAction*> *YahooContact::customContextMenuActions()
{
	QList<KAction*> *actions = new QList<KAction*>();
	if ( !m_webcamAction )
	{
		m_webcamAction = new KAction( KIcon("webcamreceive"), i18n( "View &Webcam" ), this );
		connect( m_webcamAction, SIGNAL(triggered(bool)), this, SLOT(requestWebcam()) );
	}
	if ( isReachable() )
		m_webcamAction->setEnabled( true );
	else
		m_webcamAction->setEnabled( false );
	//actions->addAction( "view_webcam", m_webcamAction );
        actions->append( m_webcamAction );

	if( !m_inviteWebcamAction )
	{
		m_inviteWebcamAction = new KAction( KIcon("webcamsend"), i18n( "Invite to view your Webcam" ), this );
		connect( m_inviteWebcamAction, SIGNAL(triggered(bool)), this, SLOT(inviteWebcam()) );
	}
	if ( isReachable() )
		m_inviteWebcamAction->setEnabled( true );
	else
		m_inviteWebcamAction->setEnabled( false );
	//actions->addAction( "invite_webcam", m_inviteWebcamAction );
        actions->append( m_inviteWebcamAction );

	if ( !m_buzzAction )
	{
		m_buzzAction = new KAction( KIcon("bell"), i18n( "&Buzz Contact" ), this);
		connect( m_buzzAction, SIGNAL(triggered(bool)), this, SLOT(buzzContact()) );
	}
	if ( isReachable() )
		m_buzzAction->setEnabled( true );
	else
		m_buzzAction->setEnabled( false );
	//actions->addAction( "buzz_contact", m_buzzAction );
        actions->append( m_buzzAction );

	if ( !m_stealthAction )
	{
		m_stealthAction = new KAction( KIcon("yahoo_stealthed"), i18n( "&Stealth Setting" ), this );
		connect( m_stealthAction, SIGNAL(triggered(bool)), this, SLOT(stealthContact()) );
	}
	if ( isReachable() )
		m_stealthAction->setEnabled( true );
	else
		m_stealthAction->setEnabled( false );
	//actions->addAction( "stealth_contact", m_stealthAction );
        actions->append( m_stealthAction );

	if ( !m_inviteConferenceAction )
	{
		m_inviteConferenceAction = new KAction( KIcon("x-office-contact"), i18n( "&Invite to Conference" ), this ); // icon should probably be "contact-invite", but that doesn't exist... please request an icon on http://techbase.kde.org/index.php?title=Projects/Oxygen/Missing_Icons
		connect( m_inviteConferenceAction, SIGNAL(triggered(bool)), this, SLOT(inviteConference()) );
	}
	if ( isReachable() )
		m_inviteConferenceAction->setEnabled( true );
	else
		m_inviteConferenceAction->setEnabled( false );
	//actions->addAction( "invite_conference", m_inviteConferenceAction );
        actions->append( m_inviteConferenceAction );

	if ( !m_profileAction )
	{
		m_profileAction = new KAction( KIcon("document-preview"), i18n( "&View Yahoo Profile" ), this );
		connect( m_profileAction, SIGNAL(triggered(bool)), this, SLOT(slotUserProfile()) );
	}
	m_profileAction->setEnabled( true );
	//actions->addAction( "profile_contact", m_profileAction );
        actions->append( m_profileAction );

	// temporary action collection, used to apply Kiosk policy to the actions
	KActionCollection tempCollection((QObject*)0);
	tempCollection.addAction(QLatin1String("contactViewWebcam"), m_webcamAction);
	tempCollection.addAction(QLatin1String("contactInviteToViewWebcam"), m_inviteWebcamAction);
	tempCollection.addAction(QLatin1String("contactBuzz"), m_buzzAction);
	tempCollection.addAction(QLatin1String("yahooContactStealth"), m_stealthAction);
	tempCollection.addAction(QLatin1String("yahooContactInviteConference"), m_inviteConferenceAction);
	tempCollection.addAction(QLatin1String("contactViewProfile"), m_profileAction);
	return actions;
}

void YahooContact::slotUserInfo()
{
	kDebug(YAHOO_GEN_DEBUG) ;
	if( !m_YABEntry )
	{
		readYABEntry();	// No YABEntry was set, so read the one from contactlist.xml
	}

	YahooUserInfoDialog *dlg = new YahooUserInfoDialog( this, Kopete::UI::Global::mainWidget() );
	dlg->setData( *m_YABEntry );
	dlg->setAccountConnected( m_account->isConnected() );
	dlg->show();
	QObject::connect( dlg, SIGNAL(saveYABEntry(YABEntry&)), m_account, SLOT(slotSaveYABEntry(YABEntry&)));
}

void YahooContact::slotUserProfile()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	QString profileSiteString = QLatin1String("http://profiles.yahoo.com/") + userId();
	KToolInvocation::invokeBrowser(  profileSiteString );
}

void YahooContact::slotSendFile( const KUrl &url)
{
	kDebug(YAHOO_GEN_DEBUG) ;
	m_account->sendFile( this, url );
}

void YahooContact::stealthContact()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	QPointer <KDialog> stealthSettingDialog = new KDialog( Kopete::UI::Global::mainWidget() );
	stealthSettingDialog->setCaption( i18n("Stealth Setting") );
	stealthSettingDialog->setButtons( KDialog::Ok | KDialog::Cancel );
	stealthSettingDialog->setDefaultButton(KDialog::Ok);
	stealthSettingDialog->showButtonSeparator(true);

	QWidget* w = new QWidget( stealthSettingDialog );
	Ui::YahooStealthSetting stealthWidget = Ui::YahooStealthSetting();
	stealthWidget.setupUi( w );
	stealthSettingDialog->setMainWidget( w );

	// Prepare dialog
	if( m_account->myself()->onlineStatus() == YahooProtocol::protocol()->Invisible )
	{
		stealthWidget.radioOffline->setEnabled( true );
		stealthWidget.radioOffline->setChecked( true );
	}
	if( stealthed() )
		stealthWidget.radioPermOffline->setChecked( true );


	// Show dialog
	if ( stealthSettingDialog->exec() == QDialog::Rejected || ! stealthSettingDialog )
	{
		delete stealthSettingDialog;
		return;
	}

	// Apply permanent setting
	if( stealthed() && !stealthWidget.radioPermOffline->isChecked() )
		m_account->yahooSession()->stealthContact( m_userId, Yahoo::StealthPermOffline, Yahoo::StealthNotActive );
	else if( !stealthed() && stealthWidget.radioPermOffline->isChecked() )
		m_account->yahooSession()->stealthContact( m_userId, Yahoo::StealthPermOffline, Yahoo::StealthActive );

	// Apply temporary setting
	if( m_account->myself()->onlineStatus() == YahooProtocol::protocol()->Invisible )
	{
		if( stealthWidget.radioOnline->isChecked() )
		{
			m_account->yahooSession()->stealthContact( m_userId, Yahoo::StealthOnline, Yahoo::StealthActive );
		}
		else if( stealthWidget.radioOffline->isChecked() )
		{
			m_account->yahooSession()->stealthContact( m_userId, Yahoo::StealthOffline, Yahoo::StealthActive );
		}
	}

	stealthSettingDialog->deleteLater();
}

void YahooContact::buzzContact()
{
	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
	Kopete::Contact *target = m_them.first();

	m_account->yahooSession()->sendBuzz( static_cast<YahooContact*>(target)->m_userId );

	KopeteView *view = manager(Kopete::Contact::CannotCreate)->view(false);
	if ( view )
	{
		Kopete::Message msg = Kopete::Message( manager(Kopete::Contact::CannotCreate)->myself() ,
					manager(Kopete::Contact::CannotCreate)->members() );
		msg.setPlainBody( i18n("Buzz") );
		msg.setDirection( Kopete::Message::Outbound );
		msg.setType( Kopete::Message::TypeAction );
		
		view->appendMessage( msg );
	}
}

void YahooContact::setDisplayPicture(const QByteArray &data, int checksum)
{
	kDebug(YAHOO_GEN_DEBUG) << data.size();

	setProperty( YahooProtocol::protocol()->iconCheckSum, checksum );

	Kopete::AvatarManager::AvatarEntry entry;
	entry.name = contactId();
	entry.category = Kopete::AvatarManager::Contact;
	entry.contact = this;
	entry.image = QImage::fromData(data);
	entry = Kopete::AvatarManager::self()->add(entry);

	if (!entry.dataPath.isNull())
	{
		setProperty( Kopete::Global::Properties::self()->photo(), QString() );
		setProperty( Kopete::Global::Properties::self()->photo() , entry.dataPath );
		emit displayPictureChanged();
	}
}


void YahooContact::setYABEntry( YABEntry *entry, bool show )
{
	kDebug(YAHOO_GEN_DEBUG) << userId();
	delete m_YABEntry;

	m_YABEntry = entry;
	writeYABEntry();	// Store data in Contact

	if( show )
		slotUserInfo();
}
const YABEntry *YahooContact::yabEntry()
{
	if( !m_YABEntry )
		readYABEntry();
	return m_YABEntry;
}

void YahooContact::inviteConference()
{
	m_account->prepareConference( m_userId );
}

void YahooContact::inviteWebcam()
{
	// TODO: some message if code for local video devices is not present
	m_account->yahooSession()->sendWebcamInvite( m_userId );
}

void YahooContact::receivedWebcamImage( const QPixmap& image )
{
	if( !m_webcamDialog )
		initWebcamViewer();
	m_receivingWebcam = true;
	emit signalReceivedWebcamImage( image );
}

void YahooContact::webcamClosed( int reason )
{
	m_receivingWebcam = false;
	emit signalWebcamClosed( reason );
}

void YahooContact::webcamPaused()
{
	emit signalWebcamPaused();
}

void YahooContact::initWebcamViewer()
{
	//

	if ( !m_webcamDialog )
	{
		m_webcamDialog = new YahooWebcamDialog( userId(), Kopete::UI::Global::mainWidget() );
// 		QObject::connect( m_webcamDialog, SIGNAL(closeClicked()), this, SLOT(closeWebcamDialog()) );

		QObject::connect( this, SIGNAL(signalWebcamClosed(int)),
		                  m_webcamDialog, SLOT(webcamClosed(int)) );

		QObject::connect( this, SIGNAL(signalWebcamPaused()),
		                  m_webcamDialog, SLOT(webcamPaused()) );

		QObject::connect( this, SIGNAL (signalReceivedWebcamImage(QPixmap)),
				m_webcamDialog, SLOT(newImage(QPixmap)) );

		QObject::connect( m_webcamDialog, SIGNAL (closingWebcamDialog()),
				this, SLOT (closeWebcamDialog()) );
	}
	m_webcamDialog->show();
}

void YahooContact::requestWebcam()
{
	if( !m_webcamDialog )
		initWebcamViewer();
	m_account->yahooSession()->requestWebcam( contactId() );
}

void YahooContact::closeWebcamDialog()
{
	QObject::disconnect( this, SIGNAL(signalWebcamClosed(int)),
	                  m_webcamDialog, SLOT(webcamClosed(int)) );

	QObject::disconnect( this, SIGNAL(signalWebcamPaused()),
	                  m_webcamDialog, SLOT(webcamPaused()) );

	QObject::disconnect( this, SIGNAL (signalReceivedWebcamImage(QPixmap)),
	                  m_webcamDialog, SLOT(newImage(QPixmap)) );

	QObject::disconnect( m_webcamDialog, SIGNAL (closingWebcamDialog()),
	                  this, SLOT (closeWebcamDialog()) );
	if( m_receivingWebcam )
		m_account->yahooSession()->closeWebcam( contactId() );
	m_webcamDialog->delayedDestruct();
	m_webcamDialog = 0L;
}

void YahooContact::deleteContact()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	if( !m_account->isOnServer( contactId() ) )
	{
		kDebug(YAHOO_GEN_DEBUG) << "Contact does not exist on server-side. Not removing...";
	}
	else
	{
		kDebug(YAHOO_GEN_DEBUG) << "Contact is getting remove from server side contact list....";
		// Delete from YAB first
		if( !m_YABEntry )
			readYABEntry();
		if( m_YABEntry->YABId )
			m_account->yahooSession()->deleteYABEntry( *m_YABEntry );

		// Now remove from the contact list
		m_account->yahooSession()->removeBuddy( contactId(), m_groupName );
	}
}

void YahooContact::writeYABEntry()
{
	kDebug(YAHOO_GEN_DEBUG) ;

	// Personal
	setProperty( YahooProtocol::protocol()->propfirstName, m_YABEntry->firstName );
	setProperty( YahooProtocol::protocol()->propSecondName, m_YABEntry->secondName );
	setProperty( YahooProtocol::protocol()->propLastName, m_YABEntry->lastName );
	setProperty( YahooProtocol::protocol()->propNickName, m_YABEntry->nickName );
	setProperty( YahooProtocol::protocol()->propTitle, m_YABEntry->title );

	// Primary Information
	setProperty( YahooProtocol::protocol()->propPhoneMobile, m_YABEntry->phoneMobile );
	setProperty( YahooProtocol::protocol()->propEmail, m_YABEntry->email );
	setProperty( YahooProtocol::protocol()->propYABId, m_YABEntry->YABId );

		// Additional Information
	setProperty( YahooProtocol::protocol()->propPager, m_YABEntry->pager );
	setProperty( YahooProtocol::protocol()->propFax, m_YABEntry->fax );
	setProperty( YahooProtocol::protocol()->propAdditionalNumber, m_YABEntry->additionalNumber );
	setProperty( YahooProtocol::protocol()->propAltEmail1, m_YABEntry->altEmail1 );
	setProperty( YahooProtocol::protocol()->propAltEmail2, m_YABEntry->altEmail2 );
	setProperty( YahooProtocol::protocol()->propImAIM, m_YABEntry->imAIM );
	setProperty( YahooProtocol::protocol()->propImICQ, m_YABEntry->imICQ );
	setProperty( YahooProtocol::protocol()->propImMSN, m_YABEntry->imMSN );
	setProperty( YahooProtocol::protocol()->propImGoogleTalk, m_YABEntry->imGoogleTalk );
	setProperty( YahooProtocol::protocol()->propImSkype, m_YABEntry->imSkype );
	setProperty( YahooProtocol::protocol()->propImIRC, m_YABEntry->imIRC );
	setProperty( YahooProtocol::protocol()->propImQQ, m_YABEntry->imQQ );

		// Private Information
	setProperty( YahooProtocol::protocol()->propPrivateAddress, m_YABEntry->privateAdress );
	setProperty( YahooProtocol::protocol()->propPrivateCity, m_YABEntry->privateCity );
	setProperty( YahooProtocol::protocol()->propPrivateState, m_YABEntry->privateState );
	setProperty( YahooProtocol::protocol()->propPrivateZIP, m_YABEntry->privateZIP );
	setProperty( YahooProtocol::protocol()->propPrivateCountry, m_YABEntry->privateCountry );
	setProperty( YahooProtocol::protocol()->propPrivatePhone, m_YABEntry->privatePhone );
	setProperty( YahooProtocol::protocol()->propPrivateURL, m_YABEntry->privateURL );

		// Work Information
	setProperty( YahooProtocol::protocol()->propCorporation, m_YABEntry->corporation );
	setProperty( YahooProtocol::protocol()->propWorkAddress, m_YABEntry->workAdress );
	setProperty( YahooProtocol::protocol()->propWorkCity, m_YABEntry->workCity );
	setProperty( YahooProtocol::protocol()->propWorkState, m_YABEntry->workState );
	setProperty( YahooProtocol::protocol()->propWorkZIP, m_YABEntry->workZIP );
	setProperty( YahooProtocol::protocol()->propWorkCountry, m_YABEntry->workCountry );
	setProperty( YahooProtocol::protocol()->propWorkPhone, m_YABEntry->workPhone );
	setProperty( YahooProtocol::protocol()->propWorkURL, m_YABEntry->workURL );

		// Miscellaneous
	setProperty( YahooProtocol::protocol()->propBirthday, m_YABEntry->birthday.toString( Qt::ISODate ) );
	setProperty( YahooProtocol::protocol()->propAnniversary, m_YABEntry->anniversary.toString( Qt::ISODate ) );
	setProperty( YahooProtocol::protocol()->propNotes, m_YABEntry->notes );
	setProperty( YahooProtocol::protocol()->propAdditional1, m_YABEntry->additional1 );
	setProperty( YahooProtocol::protocol()->propAdditional2, m_YABEntry->additional2 );
	setProperty( YahooProtocol::protocol()->propAdditional3, m_YABEntry->additional3 );
	setProperty( YahooProtocol::protocol()->propAdditional4, m_YABEntry->additional4 );
}

void YahooContact::readYABEntry()
{
	kDebug(YAHOO_GEN_DEBUG) ;
	delete m_YABEntry;

	m_YABEntry = new YABEntry;
	m_YABEntry->yahooId = userId();
	// Personal
	m_YABEntry->firstName = property( YahooProtocol::protocol()->propfirstName ).value().toString();
	m_YABEntry->secondName = property( YahooProtocol::protocol()->propSecondName ).value().toString();
	m_YABEntry->lastName = property( YahooProtocol::protocol()->propLastName ).value().toString();
	m_YABEntry->nickName = property( YahooProtocol::protocol()->propNickName ).value().toString();
	m_YABEntry->title = property( YahooProtocol::protocol()->propTitle ).value().toString();

	// Primary Information
	m_YABEntry->phoneMobile = property( YahooProtocol::protocol()->propPhoneMobile ).value().toString();
	m_YABEntry->email = property( YahooProtocol::protocol()->propEmail ).value().toString();
	m_YABEntry->YABId = property( YahooProtocol::protocol()->propYABId ).value().toInt();

	// Additional Information
	m_YABEntry->pager = property( YahooProtocol::protocol()->propPager ).value().toString();
	m_YABEntry->fax = property( YahooProtocol::protocol()->propFax ).value().toString();
	m_YABEntry->additionalNumber = property( YahooProtocol::protocol()->propAdditionalNumber ).value().toString();
	m_YABEntry->altEmail1 = property( YahooProtocol::protocol()->propAltEmail1 ).value().toString();
	m_YABEntry->altEmail2 = property( YahooProtocol::protocol()->propAltEmail2 ).value().toString();
	m_YABEntry->imAIM = property( YahooProtocol::protocol()->propImAIM ).value().toString();
	m_YABEntry->imICQ = property( YahooProtocol::protocol()->propImICQ ).value().toString();
	m_YABEntry->imMSN = property( YahooProtocol::protocol()->propImMSN ).value().toString();
	m_YABEntry->imGoogleTalk = property( YahooProtocol::protocol()->propImGoogleTalk ).value().toString();
	m_YABEntry->imSkype = property( YahooProtocol::protocol()->propImSkype ).value().toString();
	m_YABEntry->imIRC = property( YahooProtocol::protocol()->propImIRC ).value().toString();
	m_YABEntry->imQQ = property( YahooProtocol::protocol()->propImQQ ).value().toString();

	// Private Information
	m_YABEntry->privateAdress = property( YahooProtocol::protocol()->propPrivateAddress ).value().toString();
	m_YABEntry->privateCity = property( YahooProtocol::protocol()->propPrivateCity ).value().toString();
	m_YABEntry->privateState = property( YahooProtocol::protocol()->propPrivateState ).value().toString();
	m_YABEntry->privateZIP = property( YahooProtocol::protocol()->propPrivateZIP ).value().toString();
	m_YABEntry->privateCountry = property( YahooProtocol::protocol()->propPrivateCountry ).value().toString();
	m_YABEntry->privatePhone = property( YahooProtocol::protocol()->propPrivatePhone ).value().toString();
	m_YABEntry->privateURL = property( YahooProtocol::protocol()->propPrivateURL ).value().toString();

	// Work Information
	m_YABEntry->corporation = property( YahooProtocol::protocol()->propCorporation ).value().toString();
	m_YABEntry->workAdress = property( YahooProtocol::protocol()->propWorkAddress ).value().toString();
	m_YABEntry->workCity = property( YahooProtocol::protocol()->propWorkCity ).value().toString();
	m_YABEntry->workState = property( YahooProtocol::protocol()->propWorkState ).value().toString();
	m_YABEntry->workZIP = property( YahooProtocol::protocol()->propWorkZIP ).value().toString();
	m_YABEntry->workCountry = property( YahooProtocol::protocol()->propWorkCountry ).value().toString();
	m_YABEntry->workPhone = property( YahooProtocol::protocol()->propWorkPhone ).value().toString();
	m_YABEntry->workURL = property( YahooProtocol::protocol()->propWorkURL ).value().toString();

	// Miscellaneous
	m_YABEntry->birthday = QDate::fromString( property( YahooProtocol::protocol()->propBirthday ).value().toString(), Qt::ISODate );
	m_YABEntry->anniversary = QDate::fromString( property( YahooProtocol::protocol()->propAnniversary ).value().toString(), Qt::ISODate );
	m_YABEntry->notes = property( YahooProtocol::protocol()->propNotes ).value().toString();
	m_YABEntry->additional1 = property( YahooProtocol::protocol()->propAdditional1 ).value().toString();
	m_YABEntry->additional2 = property( YahooProtocol::protocol()->propAdditional2 ).value().toString();
	m_YABEntry->additional3 = property( YahooProtocol::protocol()->propAdditional3 ).value().toString();
	m_YABEntry->additional4 = property( YahooProtocol::protocol()->propAdditional4 ).value().toString();
}

#include "yahoocontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
//kate: space-indent off; replace-tabs off; indent-mode csands;

