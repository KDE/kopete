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
#include "kopetegroup.h"
#include "kopetechatsession.h"
#include "kopeteonlinestatus.h"
#include "kopetemetacontact.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include "kopeteview.h"

// Local Includes
#include "yahoocontact.h"
#include "yahooaccount.h"
#include "yahoowebcamdialog.h"
#include "yahoostealthsetting.h"
#include "yahoochatsession.h"

// QT Includes
#include <qregexp.h>
#include <qfile.h>
#include <qradiobutton.h>

// KDE Includes
#include <kdebug.h>
#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <krun.h>
#include <kshortcut.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kio/global.h>
#include <kio/job.h>
#include <kurl.h>
#include <kio/jobclasses.h>
//#include <kimageio.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

YahooContact::YahooContact( YahooAccount *account, const QString &userId, const QString &fullName, Kopete::MetaContact *metaContact )
	: Kopete::Contact( account, userId, metaContact )
{
	//kdDebug(14180) << k_funcinfo << endl;

	m_userId = userId;
	if ( metaContact )
		m_groupName = metaContact->groups().getFirst()->displayName();
	m_manager = 0L;
	m_account = account;

	// Update ContactList
	setNickName( fullName );
	setOnlineStatus( static_cast<YahooProtocol*>( m_account->protocol() )->Offline );
	setFileCapable( true );
	
	if ( m_account->haveContactList() )
		syncToServer();
	
	m_webcamDialog = 0L;
	m_webcamAction = 0L;
	m_stealthAction = 0L;

	m_buzzAction = 0L;
}

YahooContact::~YahooContact()
{
}

void YahooContact::setOnlineStatus(const Kopete::OnlineStatus &status)
{
	Contact::setOnlineStatus( status );
	if( status.status() == Kopete::OnlineStatus::Offline ) 
		removeProperty( ((YahooProtocol*)(m_account->protocol()))->awayMessage);
}

void YahooContact::serialize(QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData)
{
	//kdDebug(14180) << k_funcinfo << endl;

	Kopete::Contact::serialize(serializedData, addressBookData);
}

void YahooContact::syncToServer()
{
	kdDebug(14180) << k_funcinfo  << endl;
	if(!m_account->isConnected()) return;

	if ( !m_account->isOnServer(m_userId) && !metaContact()->isTemporary() )
	{	kdDebug(14180) << "Contact " << m_userId << " doesn't exist on server-side. Adding..." << endl;

		Kopete::GroupList groupList = metaContact()->groups();
		for( Kopete::Group *g = groupList.first(); g; g = groupList.next() )
			m_account->yahooSession()->addBuddy(m_userId, g->displayName() );
	}
}

void YahooContact::sync(unsigned int flags)
{
	if ( !m_account->isConnected() )
		return;

	if ( !m_account->isOnServer( contactId() ) )
	{
		//TODO: Share this code with the above function
		kdDebug(14180) << k_funcinfo << "Contact isn't on the server. Adding..." << endl;
		Kopete::GroupList groupList = metaContact()->groups();
		for ( Kopete::Group *g = groupList.first(); g; g = groupList.next() )
			m_account->yahooSession()->addBuddy(m_userId, g->displayName() );
	}
	else
	{
		QString newGroup = metaContact()->groups().first()->displayName();
		if ( flags & Kopete::Contact::MovedBetweenGroup )
		{
			kdDebug(14180) << k_funcinfo << "contact changed groups. moving on server" << endl;
			m_account->yahooSession()->changeBuddyGroup( contactId(), m_groupName, newGroup );
			m_groupName = newGroup;
		}
	}
}


bool YahooContact::isOnline() const
{
	//kdDebug(14180) << k_funcinfo << endl;
	return onlineStatus().status() != Kopete::OnlineStatus::Offline && onlineStatus().status() != Kopete::OnlineStatus::Unknown;
}

bool YahooContact::isReachable()
{
	//kdDebug(14180) << k_funcinfo << endl;
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
		connect( m_manager, SIGNAL( destroyed() ), this, SLOT( slotChatSessionDestroyed() ) );
		connect( m_manager, SIGNAL( messageSent ( Kopete::Message&, Kopete::ChatSession* ) ), this, SLOT( slotSendMessage( Kopete::Message& ) ) );
		connect( m_manager, SIGNAL( myselfTyping( bool) ), this, SLOT( slotTyping( bool ) ) );
		connect( m_account, SIGNAL( receivedTypingMsg( const QString &, bool ) ), m_manager, SLOT( receivedTypingMsg( const QString&, bool ) ) );
		connect( this, SIGNAL( signalWebcamInviteAccepted() ), this, SLOT( requestWebcam() ) );
		connect( this, SIGNAL(displayPictureChanged()), m_manager, SLOT(slotDisplayPictureChanged()));
	}

	return m_manager;
}

void YahooContact::slotSendMessage( Kopete::Message &message )
{
	kdDebug(14180) << k_funcinfo << endl;
	
	// Yahoo does not understand XML/HTML message data, so send plain text
	// instead.  (Yahoo has its own format for "rich text".)
	QRegExp regExp;
	int pos = 0;
	regExp.setMinimal( true );
	
	QString messageText = message.escapedBody();
	kdDebug(14180) << "Original message: " << messageText << endl;

	// find and replace Bold-formattings
	regExp.setPattern( "<span([^>]*)font-weight:600([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.search( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
		messageText.replace( regExp, QString::fromLatin1("<span\\1font-weight:600\\2>\033[1m\\3\033[x1m</span>" ) );
		}
	}
	
	// find and replace Underline-formattings
	regExp.setPattern( "<span([^>]*)text-decoration:underline([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.search( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			messageText.replace( regExp, QString::fromLatin1("<span\\1text-decoration:underline\\2>\033[4m\\3\033[x4m</span>" ) );
		}
	}
	
	// find and replace Italic-formattings
	regExp.setPattern( "<span([^>]*)font-style:italic([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.search( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			messageText.replace( regExp, QString::fromLatin1("<span\\1font-style:italic\\2>\033[2m\\3\033[x2m</span>" ) );
		}
	}
	
	// find and replace Color-formattings
	regExp.setPattern( "<span([^>]*)color:#([0-9a-zA-Z]*)([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.search( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			messageText.replace( regExp, QString::fromLatin1("<span\\1\\3>\033[#\\2m\\4\033[#000000m</span>" ) );
		}
	}
	
	// remove span-tags
	regExp.setPattern( "<span([^>]*)>(.*)</span>" );
	pos = 0;
	while ( pos >= 0 ) {
		pos = regExp.search( messageText, pos );
		if ( pos >= 0 ) {
			pos += regExp.matchedLength();
			messageText.replace( regExp, QString::fromLatin1("\\2") );
		}
	}
	
	// convert escaped chars
	messageText.replace( QString::fromLatin1( "&gt;" ), QString::fromLatin1( ">" ) );
	messageText.replace( QString::fromLatin1( "&lt;" ), QString::fromLatin1( "<" ) );
	messageText.replace( QString::fromLatin1( "&quot;" ), QString::fromLatin1( "\"" ) );
	messageText.replace( QString::fromLatin1( "&nbsp;" ), QString::fromLatin1( " " ) );
	messageText.replace( QString::fromLatin1( "&amp;" ), QString::fromLatin1( "&" ) );
	
	kdDebug(14180) << "Converted message: " << messageText << endl;
	
	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
	Kopete::Contact *target = m_them.first();

	m_account->yahooSession()->sendIm( static_cast<YahooContact*>(m_account->myself())->m_userId,
							static_cast<YahooContact *>(target)->m_userId, messageText, m_account->pictureFlag());

	// append message to window
	manager(Kopete::Contact::CanCreate)->appendMessage(message);
	manager(Kopete::Contact::CanCreate)->messageSucceeded();
}

void YahooContact::sendFile( const KURL &sourceURL, const QString &/*fileName*/, uint fileSize )
{
	QString file;
	if( sourceURL.isValid() )
		file = sourceURL.path();
	else
	{
		file = KFileDialog::getOpenFileName( QString::null, "*", 0, i18n("Kopete File Transfer") );
		if( !file.isEmpty() )
		{
			fileSize = QFile( file ).size();
		}
		else
			return;
	}
	
	m_account->yahooSession()->sendFile( m_userId, QString(), file, fileSize );
}

void YahooContact::slotTyping(bool isTyping_ )
{
	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
	Kopete::Contact *target = m_them.first();


	m_account->yahooSession()->sendTyping( static_cast<YahooContact*>(m_account->myself())->m_userId,
		static_cast<YahooContact*>(target)->m_userId, isTyping_ );
}

void YahooContact::slotChatSessionDestroyed()
{
	m_manager = 0L;
}

QPtrList<KAction> *YahooContact::customContextMenuActions()
{
	QPtrList<KAction> *actionCollection = new QPtrList<KAction>();
	if ( !m_webcamAction )
	{
		m_webcamAction = new KAction( i18n( "View &Webcam" ), "camera_unmount", KShortcut(),
		                              this, SLOT( requestWebcam() ), this, "view_webcam" );
	}
	if ( isReachable() )
		m_webcamAction->setEnabled( true );
	else
		m_webcamAction->setEnabled( false );
	actionCollection->append( m_webcamAction );
	
	if ( !m_buzzAction )
	{
		m_buzzAction = new KAction( i18n( "&Buzz Contact" ), KShortcut(), this, SLOT( buzzContact() ), this, "buzz_contact");
	}
	if ( isReachable() )
		m_buzzAction->setEnabled( true );
	else
		m_buzzAction->setEnabled( false );
	actionCollection->append( m_buzzAction );

	if ( !m_stealthAction )
	{
		m_stealthAction = new KAction( i18n( "&Stealth Setting" ), KShortcut(), this, SLOT( stealthContact() ), this, "stealth_contact");
	}
	if ( isReachable() )
		m_stealthAction->setEnabled( true );
	else
		m_stealthAction->setEnabled( false );
	actionCollection->append( m_stealthAction );
	
	return actionCollection;
	
	//return 0L;
}

void YahooContact::slotUserInfo()
{
	kdDebug(14180) << k_funcinfo << endl;
	if( m_account->yahooSession() )
		m_account->yahooSession()->getUserInfo( m_userId );
	else
		KMessageBox::information( Kopete::UI::Global::mainWidget(), i18n("You need to connect to the service in order to use this feature."),
		                         i18n("Not connected") );
}

void YahooContact::slotSendFile()
{
	kdDebug(14180) << k_funcinfo << endl;
}

void YahooContact::stealthContact()
{
	kdDebug(14180) << k_funcinfo << endl;

	KDialogBase *stealthSettingDialog = new KDialogBase( Kopete::UI::Global::mainWidget(), "stealthSettingDialog", "true",
				i18n("Stealth Setting"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, true );
	YahooStealthSetting *stealthWidget = new YahooStealthSetting( stealthSettingDialog, "stealthSettingWidget" );
	stealthSettingDialog->setMainWidget( stealthWidget );

	if ( stealthSettingDialog->exec() == QDialog::Rejected )
		return;
	
	if ( stealthWidget->radioOnline->isChecked() )
		m_account->yahooSession()->stealthContact( m_userId, 1 );
	else
		m_account->yahooSession()->stealthContact( m_userId, 0 );

	stealthSettingDialog->delayedDestruct();
}

void YahooContact::buzzContact()
{
	Kopete::ContactPtrList m_them = manager(Kopete::Contact::CanCreate)->members();
	Kopete::Contact *target = m_them.first();
	
	m_account->yahooSession()->buzzContact(	static_cast<YahooContact*>(m_account->myself())->m_userId, static_cast<YahooContact*>(target)->m_userId, m_account->pictureFlag() );

	KopeteView *view = manager(Kopete::Contact::CannotCreate)->view(false);
	if ( view )
	{
		Kopete::Message msg = Kopete::Message( manager(Kopete::Contact::CannotCreate)->myself() ,
									manager(Kopete::Contact::CannotCreate)->members(), i18n("Buzzz!!!"),
									Kopete::Message::Internal, Kopete::Message::PlainText );
		view->appendMessage( msg );
	}
}

void YahooContact::gotWebcamInvite()
{
	// emit signalReceivedWebcamInvite();
	
	if( KMessageBox::Yes == KMessageBox::questionYesNo( Kopete::UI::Global::mainWidget(), i18n("%1 has invited you to view his/her webcam. Accept?").arg(nickName()), QString::null, i18n("Accept"), i18n("Ignore") ) )
		
	{
		emit signalWebcamInviteAccepted ( );
	}
	else
	{
		// libyahoo2 doesn't do anything for rejecting invites
	}
	
}

void YahooContact::sendBuddyIconChecksum( int checksum )
{
	kdDebug(14180) << k_funcinfo << endl;
	m_account->yahooSession()->sendBuddyIconChecksum( checksum, m_userId );
	
}

void YahooContact::sendBuddyIconInfo( const QString &url, int checksum )
{
	kdDebug(14180) << k_funcinfo << endl;
	m_account->yahooSession()->sendBuddyIconInfo( m_userId, url, checksum );
}

void YahooContact::sendBuddyIconUpdate( int type )
{
	kdDebug(14180) << k_funcinfo << endl;
	m_account->yahooSession()->sendBuddyIconUpdate( m_userId, type );
}

void YahooContact::setDisplayPicture(KTempFile *f, int checksum)
{
	kdDebug(14180) << k_funcinfo << endl;
	if( !f )
		return;
	// stolen from msncontact.cpp ;)
	QString newlocation=locateLocal( "appdata", "yahoopictures/"+ contactId().lower().replace(QRegExp("[./~]"),"-")  +".png"  ) ;
	setProperty( YahooProtocol::protocol()->iconCheckSum, checksum );
	
	KIO::Job *j=KIO::file_move( KURL::fromPathOrURL( f->name() ) , KURL::fromPathOrURL( newlocation ) , -1, true /*overwrite*/ , false /*resume*/ , false /*showProgressInfo*/ );
	
	f->setAutoDelete(false);
	delete f;
	
	//let the time to KIO to copy the file
	connect(j, SIGNAL(result(KIO::Job *)) , this, SLOT(slotEmitDisplayPictureChanged() ));
}

void YahooContact::slotEmitDisplayPictureChanged()
{
	kdDebug(14180) << k_funcinfo << endl;
	QString newlocation=locateLocal( "appdata", "yahoopictures/"+ contactId().lower().replace(QRegExp("[./~]"),"-")  +".png"  ) ;
	setProperty( Kopete::Global::Properties::self()->photo(), QString::null );
	setProperty( Kopete::Global::Properties::self()->photo() , newlocation );
	emit displayPictureChanged();
}

void YahooContact::receivedWebcamImage( const QPixmap& image )
{
	emit signalReceivedWebcamImage( image );
}

void YahooContact::webcamClosed( int reason )
{
	emit signalWebcamClosed( reason );
}

void YahooContact::requestWebcam()
{
	if ( !KStandardDirs::findExe("jasper") )
	{
		KMessageBox::queuedMessageBox(                            		Kopete::UI::Global::mainWidget(),
			KMessageBox::Error, i18n("I cannot find the jasper image convert program.\njasper is required to render the yahoo webcam images.\nPlease go to http://kopete.kde.org/yahoo/jasper.html for instructions.")
                );
		return;
	}

	// uncomment this when kdelibs supports jpc
	// KImageIO::registerFormats();
	
	delete m_webcamDialog;
	m_webcamDialog = NULL;
	if ( !m_webcamDialog )
	{
		m_webcamDialog = new YahooWebcamDialog( this, Kopete::UI::Global::mainWidget() );
		QObject::connect( m_webcamDialog, SIGNAL( closeClicked() ), this, SLOT( closeWebcamDialog() ) );
	}
	
	QObject::connect( this, SIGNAL( signalWebcamClosed( int ) ),
	                  m_webcamDialog, SLOT( webcamClosed( int ) ) );
	
	QObject::connect( this, SIGNAL ( signalReceivedWebcamImage( const QPixmap& ) ),
	                  m_webcamDialog, SLOT( newImage( const QPixmap& ) ) );

	QObject::connect( m_webcamDialog, SIGNAL ( closingWebcamDialog ( ) ),
	                  this, SLOT ( closeWebcamDialog ( ) ) );
	
	m_account->yahooSession()->requestWebcam( contactId() );
}

void YahooContact::closeWebcamDialog()
{
	QObject::disconnect( this, SIGNAL( signalWebcamClosed( int ) ),
	                  m_webcamDialog, SLOT( webcamClosed( int ) ) );
	
	QObject::disconnect( this, SIGNAL ( signalReceivedWebcamImage( const QPixmap& ) ),
	                  m_webcamDialog, SLOT( newImage( const QPixmap& ) ) );
	
	QObject::disconnect( m_webcamDialog, SIGNAL ( closingWebcamDialog ( ) ),
	                  this, SLOT ( closeWebcamDialog ( ) ) );
	
	m_account->yahooSession()->closeWebcam( contactId() );
	//m_webcamDialog->delayedDestruct();
}

void YahooContact::deleteContact()
{
	kdDebug(14180) << k_funcinfo << endl;
	//my ugliest hack yet. how many levels of indirection do I want? ;)
	if ( m_account->isConnected() )
		m_account->yahooSession()->removeBuddy(m_userId, m_groupName);

	Kopete::Contact::deleteContact();
}
#include "yahoocontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
//kate: space-indent off; replace-tabs off; indent-mode csands;

