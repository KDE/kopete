/*
    ligcontact.h - Kopete Lig Protocol

    Copyright (c) 2007      by Cláudio da Silveira Pinheiro	<taupter@gmail.com>
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

#ifndef LIGCONTACT_H
#define LIGCONTACT_H

#include <qmap.h>
#include "kopetecontact.h"
#include "kopetemessage.h"

class KAction;
class KActionCollection;
namespace Kopete { class Account; }
namespace Kopete { class ChatSession; }
namespace Kopete { class MetaContact; }

/**
@author Will Stephenson
*/
class LigContact : public Kopete::Contact
{
	Q_OBJECT
public:
	/**
	 * The range of possible contact types
	 */
	enum LigContactType { Null, Echo };

	LigContact( Kopete::Account* _account, const QString &uniqueName, 
			const LigContact::LigContactType type, const QString &displayName, 
			Kopete::MetaContact *parent );

    ~LigContact();

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
	virtual QPtrList<KAction> *customContextMenuActions();
	/**
	 * Returns a Kopete::ChatSession associated with this contact
	 */
	virtual Kopete::ChatSession *manager( CanCreateFlags canCreate = CannotCreate );

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
	LigContactType m_type;
	KAction* m_actionPrefs;
};

#endif
