/*
    kopetemetacontact.h - Kopete Meta Contact

    Copyright (c) 2002 by Martijn Klingens       <klingens@kde.org>
    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __kopetemetacontact_h__
#define __kopetemetacontact_h__

#include <qobject.h>

#include "kopetecontact.h"

/**
 * @author Martijn Klingens <klingens@kde.org>
 *
 */
class KopeteMetaContact : public QObject
{
	Q_OBJECT

public:
	KopeteMetaContact();
	~KopeteMetaContact();

	/**
	 * Retrieve the list of contacts that are part of the meta contact
	 */
	QPtrList<KopeteContact> contacts() const { return m_contacts; }

	/**
	 * Add contact to the meta contact
	 */
	void addContact( KopeteContact *c, const QStringList &groups );

	/**
	 * Find the KopeteContact to a given contact. If contact
	 * is not found, a null pointer is returned.
	 * For now, just compare the ID field.
	 * FIXME: Also take protocol and identity into account!
	 */
	KopeteContact *findContact( const QString &contactId );

	/**
	 * The name of the icon associated with the contact's status
	 */
	virtual QString statusIcon() const;

	/**
	 * Returns whether this contact can be reached online for at least one
	 * protocol. Protocols are processed in loading order.
	 * FIXME: Make that user preference order!
	 * FIXME: Make that an enum, because status can be unknown for certain
	 *        protocols
	 */
	bool isOnline() const;

	/**
	 * Contact's status
	 */
	enum OnlineStatus { Online, Away, Offline };

	/**
	 * Return more fine-grained status.
	 * Online means at least one sub-contact is online, away means at least
	 * one is away, but nobody is online and offline speaks for itself
	 */
	OnlineStatus status() const;

	/**
	 * Like isOnline, but returns true even if the contact is not online, but
	 * can be reached trough offline-messages.
	 * FIXME: Here too, use preference order, not append order!
	 * FIXME: Here too an enum.
	 */
	bool isReachable() const;

	/**
	 * Get/set the display name
	 */
	QString displayName() const;
	void setDisplayName( const QString &name );

public slots:
	/**
	 * Contact another user.
	 * Depending on the config settings, call sendMessage() or
	 * startChat()
	 */
	void execute();

	/**
	 * Send a single message, classic ICQ style.
	 * The actual sending is done by the KopeteContact, but the meta contact
	 * does the GUI side of things.
	 * This is a slot to allow being called easily from e.g. a GUI.
	 */
	void sendMessage();

	/**
	 * Like sendMessage, but this time a full-blown chat will be opened.
	 * Most protocols can't distinguish between the two and are either
	 * completely session based like MSN or completely message based like
	 * ICQ the only true difference is the GUI shown to the user.
	 */
	void startChat();

signals:
	/**
	 * The contact's online status changed
	 */
	void onlineStatusChanged( KopeteMetaContact *contact,
		KopeteMetaContact::OnlineStatus status );

	/**
	 * The meta contact's display name changed
	 */
	void displayNameChanged( KopeteMetaContact *c, const QString &name );

private slots:
	/**
	 * One of the child contact's online status changed
	 */
	void slotContactStatusChanged( KopeteContact *c,
		KopeteContact::ContactStatus s );

	/**
	 * One of the child contact's display names changed
	 * FIXME: Add a KopeteContact * to this method and the associated signal
	 *        in KopeteContact!
	 */
	void slotContactNameChanged( const QString &name );

private:
	QPtrList<KopeteContact> m_contacts;

	/**
	 * Display name as shown
	 */
	QString m_displayName;

	/**
	 * When true, track changes in child contact's display name and update
	 * the meta contact's display name too
	 */
	bool m_trackChildNameChanges;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

