/*
    gwcontact.h - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
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

#include <qmap.h>
#include "kopetecontact.h"
#include "kopetemessage.h"

class KAction;
class KActionCollection;
class KopeteAccount;
class KopeteMessageManager;
class KopeteMetaContact;

/**
@author Will Stephenson
*/
class GroupWiseContact : public KopeteContact
{
	Q_OBJECT
public:
	/**
	 * The range of possible contact types
	 */
	enum GroupWiseContactType { Null, Echo };

	GroupWiseContact( KopeteAccount* _account, const QString &uniqueName, 
			const GroupWiseContact::GroupWiseContactType type, const QString &displayName, 
			KopeteMetaContact *parent );

    ~GroupWiseContact();

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
	 * Returns a KopeteMessageManager associated with this contact
	 */
	virtual KopeteMessageManager *manager( bool canCreate = false );

public slots:
	/**
	 * Transmits an outgoing message to the server 
	 * Called when the chat window send button has been pressed
	 * (in response to the relevant KopeteMessageManager signal)
	 */
	void sendMessage( KopeteMessage &message );
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
	 * Notify the contact that its current KopeteMessageManager was
	 * destroyed - probably by the chatwindow being closed
	 */
	void slotMessageManagerDestroyed();
	
protected:
	KopeteMessageManager* m_msgManager;
	KActionCollection* m_actionCollection;
	GroupWiseContactType m_type;
	KAction* m_actionPrefs;
};

#endif
