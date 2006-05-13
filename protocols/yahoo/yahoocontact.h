/*
    yahoocontact.h - Yahoo Contact

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

#ifndef YAHOOCONTACT_H
#define YAHOOCONTACT_H

/* Kopete Includes */
#include "kopetecontact.h"

class KAction;
class KTempFile;

namespace Kopete { class ChatSession; }
namespace Kopete { class MetaContact; }
namespace Kopete { class OnlineStatus; }
namespace Kopete { class Message; }
class YahooProtocol;
class YahooAccount;
class YahooWebcamDialog;
class YahooChatSession;
class YABEntry;
struct KURL;

class YahooContact : public Kopete::Contact
{
	Q_OBJECT
public:
	YahooContact( YahooAccount *account, const QString &userId, const QString &fullName, Kopete::MetaContact *metaContact );
	~YahooContact();

	/** Base Class Reimplementations **/
	virtual bool isOnline() const;
	virtual bool isReachable();
	virtual QPtrList<KAction> *customContextMenuActions();
	virtual Kopete::ChatSession *manager( Kopete::Contact::CanCreateFlags canCreate= Kopete::Contact::CanCreate );
	virtual void serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData );

	void setOnlineStatus(const Kopete::OnlineStatus &status);
	void setYahooStatus( const Kopete::OnlineStatus& );
	void setStealthed( bool );
	bool stealthed();


	/** The group name getter and setter methods**/
	QString group() const;
	void setGroup( const QString& );

	/** The userId getter method**/
	QString userId() const;

	void receivedWebcamImage( const QPixmap& );
	void webcamClosed( int );
	void webcamPaused();
	
	const YABEntry *yabEntry();

	static QString prepareMessage( const QString &messageText );

public slots:
	virtual void slotUserInfo();
	virtual void slotSendFile( const KURL &file );
	virtual void deleteContact();
	virtual void sendFile( const KURL &sourceURL = KURL(), const QString &fileName = QString::null, uint fileSize = 0L );
	void slotUserProfile();
	void stealthContact();
	void requestWebcam();
	void inviteWebcam();
	void buzzContact();
	void setDisplayPicture(KTempFile *f, int checksum);
	void sendBuddyIconInfo( const QString &url, int checksum );
	void sendBuddyIconUpdate( int type );
	void sendBuddyIconChecksum( int checksum );
	void setYABEntry( YABEntry *, bool show = false );

	/**
	 * Must be called after the contact list has been received
	 * or it doesn't work well!
	 */
	void syncToServer();

	void sync(unsigned int flags);

signals:
	void signalReceivedWebcamImage( const QPixmap &pic );
	void signalWebcamClosed( int reason );
	void signalWebcamPaused();
	void displayPictureChanged();

private slots:
	void slotChatSessionDestroyed();
	void slotSendMessage( Kopete::Message& );
	void slotTyping( bool );
	void slotEmitDisplayPictureChanged();

	void closeWebcamDialog();
	void initWebcamViewer();
	void inviteConference();

	void writeYABEntry();
	void readYABEntry();

private:
	QString m_userId; 
	QString m_groupName;
	YABEntry *m_YABEntry;
	YahooChatSession *m_manager;
	YahooWebcamDialog* m_webcamDialog;
	YahooAccount* m_account;
	bool m_stealthed;
	bool m_receivingWebcam;
	bool m_sessionActive;
	
	KAction* m_stealthAction;
	KAction* m_profileAction;
	KAction* m_webcamAction;
	KAction* m_inviteWebcamAction;
	KAction* m_buzzAction;	
	KAction* m_inviteConferenceAction;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

