/*
    kopetecontact.h - Kopete Contact

	Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
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

#ifndef KOPETECONTACT_H
#define KOPETECONTACT_H

#include "contactlist.h"

#include <qobject.h>

class QString;
class QPixmap;

class KopeteEvent;

/**
 * This class abstracts a generic contact/buddie.
 * Use it for inserting contacts in the contact list for example.
 */
class KopeteContact : public QObject
{
	Q_OBJECT

public:
	/**
	 * Contact's status
	 */
	enum ContactStatus { Online, Away, Offline };

	/**
	 * Return whether this contact is online or not.
	 * FIXME: Make the return value an enum, because this value might
	 *        be Unknown or NotApplicable!
	 * FIXME: When all plugins support this, make this pure virtual!
	 */
	bool isOnline() const { return status() != Offline; }

	/**
	 * The groups in which the user is physically located.
	 * The logical groups are stored in the Meta Contact. Physical groups
	 * can be different from the logical groups!
	 * Not the whole API supports multi-group membership yet, be careful
	 * relying on this for now!
	 * FIXME: When all protocols support the group API, make these methods
	 *        pure virtual or have different defaults!
	 */
	virtual QStringList groups();

	/**
	 * Add a contact to a physical group. If the protocol doesn't support
	 * multi-group memberships this method can do nothing. The group name
	 * passed is the logical group. Protocols with server-side contact lists
	 * can use this to keep the local and remote lists in sync.
	 */
	virtual void addToGroup( const QString &group );

	/**
	 * Remove a contact from a physical group.
	 * If the logical group passed is different from the physical group, or
	 * if this kind of changes is not supported this method may do nothing.
	 */
	virtual void removeFromGroup( const QString &group );

	/**
	 * Move a contact from one group to another. Again, this method may do
	 * nothing if there's no support for this in the protocol.
	 */
	virtual void moveToGroup( const QString &from, const QString &to );

		/**
		* Usually a contact is owned by a protocol plugin
		**/
		KopeteContact(QObject *parent);
		~KopeteContact();

		/**
		* set name of an KopeteContact
		* This is the string that gets drawn in the listview
		**/
		virtual void setName( const QString &name );
		/**
		* Display name, known as nick
		**/
		virtual QString name() const;
		/**
		* return status of an KopeteContact
		**/
		virtual ContactStatus status() const;
		/**
		* The text describing the contact's status
		* The default implement does what you'd expect,
		* but you might want to reimplement it for more
		* fine-grained reporting of status
		**/
		virtual QString statusText() const;
		/**
		* The name of the icon associated with the contact's status
		**/
		virtual QString statusIcon() const;
		/**
		* This method provides an scaled version of the status icon.
		* useful for metacontacts, and it uses a primitive cache so
		* we dont have to scale at every repaint, it asumes square.
		**/
		virtual QPixmap scaledStatusIcon(int size);
		/**
		* The "importance" of this contact, used for sorting
		* This is almost always related to the contact's status
		* Here is how ICQ does it:
		* 25 = Free For Chat
		* 20 = Online
		* 15 = Away (temporary away)
		* 10 = Not Available (extended away)
		* 5 = Invisible
		* 0 = Offline
		*
		* The default implementation returns 20 for Online,
		* 10 for away, and 0 for offline
		* Please make your importance values between 0 and 25,
		* and try to follow ICQ's scheme somewhat
		**/
		virtual int importance() const;
		/**
		* This should typically pop up a KopeteChatWindow
		**/
		virtual void execute() {}
		/**
		* Show a context menu of actions pertaining to this contact
		* I hate having the group parameter, but its used for when
		* a contact can be in multiple groups and you have to move
		* a specific instance from one group to another.
		**/
		virtual void showContextMenu( QPoint /* p */, QString /* group */ ) {}

		/**
		 * Return the unique id that identifies a contact. Id is required
		 * to be unique per protocol and per identity. Across those boundaries
		 * ids may occur multiple times.
		 * The id is solely for comparing items safely (using pointers is
		 * more crash-prone). DO NOT assume anything regarding the id's
		 * value! Even if it may look like an ICQ UIN or an MSN passport,
		 * this is undefined and may change at any time!
		 */
		QString id() const { return m_id; }
		void setId( const QString &id ) { m_id = id; }
		
		/**
		 * Return the protocol id that identifies a contact. Id is required
		 * to be unique per protocol and per identity. Across those boundaries
		 * ids may occur multiple times.
		 * The id is solely for comparing items safely (using pointers is
		 * more crash-prone). DO NOT assume anything regarding the id's
		 * value! Even if it may look like an ICQ UIN or an MSN passport,
		 * this is undefined and may change at any time!
		 */
		QString protocol() const { return m_protocolId; }
		void setProtocol( const QString &id ) { m_protocolId = id; }

		/**
		 * Return the protocol specific serialized data
		 * that a plugin may want to store here.
		 */
		QString data() const { return m_data; }
		void setData( const QString &data ) { m_data = data; }

		/**
		 * Return a XML representation of the contact
		 */
		QString toXML();

signals:
	/**
	 * The contact's online status changed
	 */
	void statusChanged( KopeteContact *contact,
		KopeteContact::ContactStatus status );

	/**
	 * Deprecated, old signal! Use the above one instead
	 */
	void statusChanged();

		/**
		* Connect to this signal to know when the contact
		* changed its name/nick
		**/
		void nameChanged(const QString &name);
		/**
		* This gets emitted usually when you receive a message from
		* this contact.
		**/
		void incomingEvent(KopeteEvent *);

	private:
		QString m_name;

		QString m_id;
		QString m_protocolId;
		QString m_data;

		/**
		* Scaled icon cache
		*/
		QPixmap m_cachedScaledIcon;
		int m_cachedSize;
		ContactStatus m_cachedOldStatus;
};
#endif
