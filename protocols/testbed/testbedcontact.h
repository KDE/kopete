/*
    testbedcontact.h - Kopete Testbed Protocol

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

#ifndef TESTBEDCONTACT_H
#define TESTBEDCONTACT_H

#include <QMap>
#include <QList>

#include <QAction>
  
#include "kopetecontact.h"
#include "kopetemessage.h"

class QAction;
class KActionCollection;
namespace Kopete { class Account; }
namespace Kopete { class ChatSession; }
namespace Kopete { class MetaContact; }

/**
@author Will Stephenson
*/
class TestbedContact : public Kopete::Contact
{
	Q_OBJECT
public:
	/**
	 * The range of possible contact types
	 */
	enum Type { Null, Echo, Group };

	TestbedContact( Kopete::Account* _account, const QString &uniqueName, 
			Kopete::MetaContact *parent );

    ~TestbedContact();

    bool isReachable() Q_DECL_OVERRIDE;
	/**
	 * Serialize the contact's data into a key-value map
	 * suitable for writing to a file
	 */
    void serialize(QMap< QString, QString >& serializedData,
			QMap< QString, QString >& addressBookData) Q_DECL_OVERRIDE;
	/**
	 * Return the actions for this contact
	 */
	//virtual QList<QAction *> *customContextMenuActions();
	using Kopete::Contact::customContextMenuActions;
	/**
	 * Returns a Kopete::ChatSession associated with this contact
	 */
	Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate ) Q_DECL_OVERRIDE;

	/**
	 * Set the Type of this contact
	 */
	void setType( Type type );
public Q_SLOTS:
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

protected Q_SLOTS:
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
	QAction* m_actionPrefs;
};

#endif
