/*
    bonjourcontact.h - Kopete Bonjour Protocol

    Copyright (c) 2007      by Tejas Dinkar	<tejas@gja.in>
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

#ifndef BONJOURCONTACT_H
#define BONJOURCONTACT_H

#include <qmap.h>
#include <QList>
#include <QHostAddress>

#include <kopetecontact.h>
#include <kopetemessage.h>

#include "bonjourcontactconnection.h"

class KAction;
class KActionCollection;
namespace Kopete { class Account; }
namespace Kopete { class ChatSession; }
namespace Kopete { class MetaContact; }

/**
 * @brief This Represents a Bonjour Contact
 *
 * Bonjour Contacts are Added only by an Account Object, and not manually.
 *
 * @author Tejas Dinkar <tejas\@gja.in>
 */
class BonjourContact : public Kopete::Contact
{
	Q_OBJECT

	/**
	 * These are the Datatypes that define how to contact the user
	 * This includes the Hostname and the Port.
	 * We also add a TCPSocket through which we can talk to the user
	 */
	BonjourContactConnection *connection;
	QString remoteHostName;
	QHostAddress remoteAddress;
	short int remotePort;

	QString username;
	QMap <QString, QByteArray> textdata;

public:
	BonjourContact( Kopete::Account* _account, const QString &uniqueName, 
			Kopete::MetaContact *parent );

	~BonjourContact();

	virtual bool isReachable();
	/**
	 * Serialize the contact's data into a key-value map
	 * suitable for writing to a file
	 *
	 * @todo FIXME: As we don't save any contacts, this function is useless
	 */
	virtual void serialize(QMap< QString, QString >& serializedData,
			QMap< QString, QString >& addressBookData);

	/**
	 * @brief Returns a Kopete::ChatSession associated with this contact
	 *
	 * @param canCreate If @c true, then a new manager may be created
	 * @return The Contats's ChatSession Manager
	 */
	virtual Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate );

	/**
	 * The Following Properties are For saving Each Contact's IP address, hostname and
	 * port. This Way, when a connection is made, we know who it is from
	 */
	Q_PROPERTY(QString remoteHostName READ getremoteHostName WRITE setremoteHostName)
	Q_PROPERTY(QHostAddress remoteAddress READ getremoteAddress WRITE setremoteAddress)
	Q_PROPERTY(short int remotePort READ getremotePort WRITE setremotePort)
	Q_PROPERTY(QString username READ getusername WRITE setusername)

	void setremoteHostName(const QString &nremoteHostName);
	void setremoteAddress(const QHostAddress &nremoteAddress);
	void setremotePort(const short int &nremotePort);
	void setusername(const QString &nusername);
	void settextdata(const QMap <QString, QByteArray> &ntextdata);

	const QString getremoteHostName() const;
	const QHostAddress getremoteAddress() const;
	short int getremotePort() const;
	const QString getusername() const;
	const QMap <QString, QByteArray> gettextdata() const;

	/*
	 * @brief Checks if the contact is at a given address
	 *
	 * This Function Checks if a contact has the given remote address
	 *
	 * @param host The Host To Be Compared With
	 * @return @c true if it is the same, @false otherwise
	 */
	bool isRemoteAddress(const QHostAddress &host) const;

	/*
	 * @brief This Sets the @ref connection
	 *
	 * A Connection is a wrapper around a socket which is a TCP connection with a remote user
	 * The Connection is reparented so that the contact is the new parent
	 *
	 * @param conn The Connection
	 */
	void setConnection(BonjourContactConnection *conn );

public slots:
	/**
	 * @brief Send an Outgoing message
	 *
	 * Transmits an outgoing message to the @ref connection
	 * Called when the chat window send button has been pressed
	 * (in response to the relevant Kopete::ChatSession signal)
	 *
	 * @param message The Message To Be Sent
	 */
	void sendMessage( Kopete::Message &message );

	/**
	 * @brief Receive an Incoming Messaed
	 *
	 * Called when an incoming message arrived
	 * This displays it in the chatwindow
	 *
	 * @param message The Message Received
	 */
	void receivedMessage( Kopete::Message message );

	/**
	 * @brief The Connection Was Disconnected
	 *
	 * Call This Function when the connection is disconnected
	 * This Cleans up (ensures the entire connection is deleted)
	 *
	 * @param conn The Connection
	 */
	void connectionDisconnected(BonjourContactConnection *conn);

protected slots:
	/**
	 * @brief Show the settings dialog
	 *
	 * @todo FIXME: This is implemented from the Testbed protocol
	 */
	void showContactSettings();

	/**
	 * @brief The Chat Session was destroyed
	 *
	 * Notify the contact that its current Kopete::ChatSession was
	 * destroyed - probably by the chatwindow being closed
	 */
	void slotChatSessionDestroyed();
	
protected:
	Kopete::ChatSession* m_msgManager;
};

#endif
