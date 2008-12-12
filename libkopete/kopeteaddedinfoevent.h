/*
    kopeteaddedinfoevent.h - Kopete Added Info Event

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETEADDEDINFOEVENT_H
#define KOPETEADDEDINFOEVENT_H

#include "kopeteinfoevent.h"

namespace Kopete {

class MetaContact;
class Account;
/**
 * @brief Event which is shown when a contact added you into the contact list or requested authorization.
 *
 * This event allows the user to give authorization for the addition to the
 * person who added the user and also allows the user to add the person into
 * the user's contact list.
 *
 * The @p title() and @p text() will be filled with predefined text.
 * If you want to add additional information use @p setAdditionalText()
 *
 * The @p actionActivated(uint) signal can be emitted more than once.
 * All AddedInfoEvent object will be closed and deleted automatically.
 *
 * example of usage
 * @code
	Kopete::AddedInfoEvent* event = new Kopete::AddedInfoEvent( contactId, account );
	QObject::connect( event, SIGNAL(actionActivated(uint)), this, SLOT(addedInfoEventActionActivated(uint)) );
	event->sendEvent();
 * @endcode
 *
 * and in your addedInfoEventActionActivated slot
 * @code
	Kopete::AddedInfoEvent *event = dynamic_cast<Kopete::AddedInfoEvent *>(sender());
	if ( !event )
		return;
	
	switch ( actionId )
	{
	case Kopete::AddedInfoEvent::AddContactAction:
		event->addContact();
		break;
	case Kopete::AddedInfoEvent::AuthorizeAction:
		socket->authorize( event->contactId() );
		break;
	case Kopete::AddedInfoEvent::InfoAction:
		showInfo();
		break;
	}
 * @endcode
 *
 * @author Roman Jarosz <kedgedev@centrum.cz>
 */
class KOPETE_EXPORT AddedInfoEvent : public InfoEvent
{
	Q_OBJECT
public:
	/**
	 * All actions that may be shown and emitted.
	 */
	enum ShowAction
	{
		AddAction = 0x001, /** Add action was activated. The default implementation shows
		                       ContactAddedNotifyDialog if activate(uint) isn't replaced */
		AuthorizeAction = 0x002, /** You should authorize the contact */
		BlockAction = 0x004, /** You should block this and future requests */
		InfoAction = 0x008, /** You should show info about contact */

		AddContactAction = 0x100, /** You should add contact to Kopete contact list with @p addContact()
		                              this is only emitted if activate(uint) isn't replaced */
		AllActions = 0x00F
	};
	Q_DECLARE_FLAGS(ShowActionOptions, ShowAction)

	/**
	 * @brief Constructor
	 *
	 * @param contactId the contactId of the contact which has added you
	 * @param account the account which has generated this event
	 */
	AddedInfoEvent( const QString& contactId, Kopete::Account *account );

	~AddedInfoEvent();

	/**
	 * Return the contactId of a contact which has added you.
	 */
	QString contactId() const;

	/**
	 * Return the account that has generated this event.
	 */
	Kopete::Account* account() const;

	/**
	 * Set which actions should be shown.
	 *
	 * @param actions a bitmask of ShowAction used to show specific actions.
	 * @note by default everything is shown.
	 */
	void showActions( ShowActionOptions actions );

	/**
	 * Set contact nickname
	 *
	 * @param nickname the nickname of the contact.
	 */
	void setContactNickname( const QString& nickname );

	/**
	 * @brief create a metacontact.
	 *
	 * This function only works if the AddContactAction action was activated, otherwise
	 * it will return 0L.
	 *
	 * it uses the Account::addContact function to add the contact
	 *
	 * @return the new metacontact created, or 0L if the operation failed.
	 */
	MetaContact* addContact() const;

public Q_SLOTS:
	/**
	 * Activate the action specified action
	 */
	virtual void activate( uint actionId );

	/**
	 * Emit the event.
	 */
	virtual void sendEvent();

private Q_SLOTS:
	void addDialogOk();
	void addDialogInfo();
	void addDialogFinished();

private:
	class Private;
	Private * const d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS( AddedInfoEvent::ShowActionOptions )

}

#endif
