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

#ifndef TESTBEDACCOUNT_H
#define TESTBEDACCOUNT_H

#include "kopetepasswordedaccount.h"
#include "qqwebcamdialog.h"

class KActionMenu;
namespace Kopete 
{ 
	class Contact;
	class MetaContact;
	class StatusMessage;
}

class QQContact;
class QQProtocol;
class QQNotifySocket;

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
	virtual KActionMenu* actionMenu();

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
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status , const Kopete::StatusMessage &reason = Kopete::StatusMessage() );
	virtual void setStatusMessage(const Kopete::StatusMessage& statusMessage);

	/**
	 * Connect/Disconnect 
	 */
	virtual void connectWithPassword( const QString &password );
	virtual void disconnect();

	/**
	 * Returns the address of the QQ server
	 */
	QString serverName();

	/**
	 * Returns the address of the QQ server port
	 */
	uint serverPort();

	QQNotifySocket* notifySocket();

public slots:
	/**
	 * Called by the server when it has a message for us.
	 * This identifies the sending Kopete::Contact and passes it a Kopete::Message
	 */
	void receivedMessage( const QString &message );
	void slotStatusChanged( const Kopete::OnlineStatus &status );

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


private:
	QQNotifySocket *m_notifySocket; // stub now.
	// status which will be using for connecting
	Kopete::OnlineStatus m_connectstatus;
	QString m_password;


};

#endif
