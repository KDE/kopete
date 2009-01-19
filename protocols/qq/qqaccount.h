/*
    qqaccount.h - Kopete QQ Protocol

    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef QQACCOUNT_H
#define QQACCOUNT_H

#include <kopetechatsessionmanager.h>
#include <kopetecontact.h>

#include "kopetepasswordedaccount.h"
#include "qqwebcamdialog.h"

class KActionMenu;
namespace Kopete 
{ 
	class Contact;
	class MetaContact;
	class StatusMessage;
}

namespace Eva {
	struct ContactInfo;
	struct ContactStatus;
	struct MessageHeader;
	class ByteArray;
}

class QQContact;
class QQProtocol;
class QQNotifySocket;
class QQChatSession;
class QTextCodec;

/**
 * This represents an account connected to the qq
 * @author Hui Jin
*/
class QQAccount : public Kopete::PasswordedAccount
{
	Q_OBJECT
public:
	QQAccount( QQProtocol *parent, const QString& accountID );

	/**
	 * Construct the context menu used for the status bar icon
	 */
	virtual void fillActionMenu( KActionMenu *actionMenu );

	/** FIXME: connect is used only for testing, once the kwalletd fixed.
	 * all the code is going to move to connectWithPassword
	 */
	virtual void connect( const Kopete::OnlineStatus& /* initialStatus */ );

	/**
	 * Creates a protocol specific Kopete::Contact subclass and adds it to the supplie
	 * Kopete::MetaContact
	 */
	virtual bool createContact(const QString& contactId, Kopete::MetaContact* parentContact);

	/**
	 * Called when Kopete status is changed globally
	 */
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                             const OnlineStatusOptions& options = None);
	virtual void setStatusMessage(const Kopete::StatusMessage& statusMessage);

	/**
	 * Connect/Disconnect 
	 */
	virtual void connectWithPassword( const QString &password );
	virtual void disconnect();

	/**
	 * Returns the address of the QQ server
	 */
	QString serverName() 
	{ return configGroup()->readEntry(  "serverName" , "tcpconn.tencent.com" ); }

	/**
	 * Returns the address of the QQ server port
	 */
	uint serverPort()
	{ return configGroup()->readEntry(  "serverPort" , 80 ); }


	/**
	 * Returns the online status from Eva status
	 */
	Kopete::OnlineStatus fromEvaStatus( char es );

	QQChatSession * chatSession( Kopete::ContactPtrList others, const QString& guid, Kopete::Contact::CanCreateFlags canCreate );

	void sendInvitation(const QString& guid, const QString& id, const QString& message );
	void sendMessage(const QString& guid, Kopete::Message& message );
	void getVCard( QQContact* contact );

public slots:
	/**
	 * Called by the server when it has a message for us.
	 * This identifies the sending Kopete::Contact and passes it a Kopete::Message
	 */
	void slotStatusChanged( const Kopete::OnlineStatus &status );
	void slotNewContactList();
	void slotContactListed( const Eva::ContactInfo& ci );
	void slotGroupNamesListed(const QStringList& ql );
	void slotContactInGroup(const int qqId, const char type, const int groupId );
	void slotContactStatusChanged(const Eva::ContactStatus& cs);
	void slotMessageReceived( const Eva::MessageHeader& header, const Eva::ByteArray& message );
	void slotContactDetailReceived( const QString& id, const QMap<const char*, QByteArray>& map) ;

protected:
	/**
	 * This simulates contacts going on and offline in sync with the account's status changes
	 */
	void updateContactStatus();

protected slots:
	/**
	 * Show webcam.  Called by KActions and internally.
	 */
	void slotShowVideo();

private:
	void createNotificationServer( const QString &host, uint port );
	QQChatSession * findChatSessionByGuid( const QString& guid );



private:
	QQNotifySocket *m_notifySocket;

	// status which will be using for connecting
	Kopete::OnlineStatus m_connectstatus;
	QString m_password;

	// server data
	QStringList m_groupNames;

	bool m_newContactList;

	Kopete::MetaContact *m_addWizard_metaContact;
	QMap< QString, QStringList > tmp_addToNewGroup;
	QMap< QString, QStringList > tmp_addNewContactToGroup;

	QString m_pictureObj; //a cache of the <msnobj>
	QString m_pictureFilename; // the picture filename.

	//this is the translation between old to new groups id when syncing from server.
	QMap<QString, Kopete::Group*> m_oldGroupList;
	QList<Kopete::Group*> m_groupList;

	/**
	 * Cliend ID is a bitfield that contains supported features for a MSN contact.
	 */
	uint m_clientId;

	QList<QQChatSession*> m_chatSessions;

	QTextCodec* m_codec;
};

#endif
