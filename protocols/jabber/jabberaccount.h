/***************************************************************************
                          jabberaccount.h  -  description
                             -------------------
    begin                : Sat Mar 8 2003
    copyright            : (C) 2003 by Till Gerken (till@tantalo.net)
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERACCOUNT_H
#define JABBERACCOUNT_H

#include <qwidget.h>
#include <kopeteaccount.h>
#include "jabbercontact.h"

/**
  *@author Till Gerken
  */

class JabberAccount : public KopeteAccount
{
	Q_OBJECT

public:
	/****************************************
	 * KopeteAccount reimplementation start *
	 ****************************************/

	JabberAccount(KopeteProtocol *parent, const QString& accountID, const char *name=0L);
	~JabberAccount();

	/**
	 * this will be called if main-kopete wants
	 * the plugin to set the user's mode to away
	 */
	virtual void setAway(bool);

	/**
	 * Function has to be reimplemented in every single protocol
	 * and return the KopeteContact associated with the 'home' user.
	 * the myself contact MUST be created in the account constructor!
	 *
	 * @return contact associated with the currently logged in user
	 */
	virtual KopeteContact* myself() const;

	/**
	 * return the menu for this account
	 */
	virtual KActionMenu* actionMenu();

	/****************************************
	 * KopeteAccount reimplementation end   *
	 ****************************************/

protected:
	/**
	 * Create a new contact in the specified metacontact
	 * You shouldn't call yourself this method, for adding contact see @ref addContact()
	 *
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayname of the contact (may equal userId for some protocols
	 * @param parentContact The metacontact to add this contact to
	 */
	virtual bool addContactToMetaContact( const QString &contactId, const QString &displayName,
		 KopeteMetaContact *parentContact );

private:
	/**
	 * JabberContact for this identity
	 */
	JabberContact *myContact;

	/**
	 * Psi backend for this identity
	 */
	Jabber::Client *client;

	/* Set up our actions for the status menu. */
	void initActions();
	
	/* Actions for the menu. */
        KAction *actionGoOnline;
        KAction *actionGoChatty;
        KAction *actionGoAway;
        KAction *actionGoXA;
        KAction *actionGoDND;
        KAction *actionGoInvisible;
        KAction *actionGoOffline;
        KAction *actionJoinChat;
        KAction *actionServices;
        KAction *actionSendRaw;
        KAction *actionEditVCard;
        KAction *actionEmptyMail;
        KActionMenu *actionStatusMenu;

        dlgJabberStatus *reasonDialog;
        dlgJabberSendRaw *sendRawDialog;
};

#endif
