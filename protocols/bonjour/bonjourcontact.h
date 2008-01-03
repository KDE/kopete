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
//Added by qt3to4:
#include <QList>
#include <QTcpSocket>
#include <QHostAddress>

#include "kopetecontact.h"
#include "kopetemessage.h"

#include "bonjourcontactconnection.h"

class KAction;
class KActionCollection;
namespace Kopete { class Account; }
namespace Kopete { class ChatSession; }
namespace Kopete { class MetaContact; }

/**
@author Will Stephenson
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

public:
	/**
	 * The range of possible contact types
	 */
	enum Type { Null, Echo, Group };

	BonjourContact( Kopete::Account* _account, const QString &uniqueName, 
			const QString &displayName, 
			Kopete::MetaContact *parent );

    ~BonjourContact();

    virtual bool isReachable();
	/**
	 * Serialize the contact's data into a key-value map
	 * suitable for writing to a file
	 */
    virtual void serialize(QMap< QString, QString >& serializedData,
			QMap< QString, QString >& addressBookData);
	/**
	 * Return the actions for this contact
	 */
	virtual QList<KAction *> *customContextMenuActions();
	/**
	 * Returns a Kopete::ChatSession associated with this contact
	 */
	virtual Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate );

	/**
	 * Set the Type of this contact
	 */
	void setType( Type type );

	/**
	 * The Following Properties are For saving Each Contact's IP address, hostname and
	 * port. This Way, when a connection is made, we know who it is from
	 */
	Q_PROPERTY(QString remoteHostName READ getremoteHostName WRITE setremoteHostName)
	Q_PROPERTY(QHostAddress remoteAddress READ getremoteAddress)
	Q_PROPERTY(short int remotePort READ getremotePort WRITE setremotePort)

	void setremoteHostName(const QString &nremoteHostName);
	void setremotePort(const short int &nremotePort);

	const QString getremoteHostName() const;
	const QHostAddress getremoteAddress() const;
	const short int getremotePort() const;

	/*
	 * This Function Checks if a contact has the given remote address and port
	 * and Returns True if they both match
	 */
	const bool isRemoteAddress(const QHostAddress &host) const;

	/*
	 * This Sets the Connection
	 */
	void setConnection(BonjourContactConnection *);

public slots:
	/**
	 * Transmits an outgoing message to the server 
	 * Called when the chat window send button has been pressed
	 * (in response to the relevant Kopete::ChatSession signal)
	 */
	void sendMessage( Kopete::Message &message );
	/**
	 * Called when an incoming message arrived
	 * This displays it in the chatwindow
	 */
	void receivedMessage( const QString &message );
	void receivedMessage( Kopete::Message *message );

	/**
	 * Call This Function when the connection is deleted
	 */
	void connectionDisconnected(BonjourContactConnection *);

protected slots:
	/**
	 * Show the settings dialog
	 */
	void showContactSettings();
	/**
	 * Notify the contact that its current Kopete::ChatSession was
	 * destroyed - probably by the chatwindow being closed
	 */
	void slotChatSessionDestroyed();
	
protected:
	Kopete::ChatSession* m_msgManager;
	KActionCollection* m_actionCollection;
	Type m_type;
	KAction* m_actionPrefs;
};

#endif
