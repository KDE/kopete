/*
    kopeteonlinestatus.h - Kopete Online Status

    Copyright (c) 2003 by Martijn Klingens <klingens@kde.org>
    Copyright (c) 2003 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2003 by Will Stephenson <lists@stevello.free-online.co.uk>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef __kopeteonlinestatus_h__
#define __kopeteonlinestatus_h__

#include <qobject.h>

class QString;
class QPixmap;
class QColor;
class KopeteContact;
class KopeteAccount;

class KopeteProtocol;

namespace Kopete
{
	class OnlineStatusIconCache;
}

struct KopeteOnlineStatusPrivate;

/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Will Stephenson (icon generating code)
 *
 * KopeteOnlineStatus is a class that encapsulates all information about the
 * various online states that a protocol can be in in a single class. The
 * online status consists of both a 'global' status as it's known to libkopete
 * and used for going online or away, which non-protocol plugins can use,
 * and the 'private' status, which is simply an unsigned int and is only
 * useful for the actual protocol plugin that uses the status.
 *
 * This class is passed around by value, but is refcounted to cut down on the
 * amount of overhead. All in all it should be more than fast enough for
 * general use.
 *
 * Note that ONLY the constructor can set the data, the object is considered
 * to be const after creation as there really shouldn't be a need to change
 * a status' characteristics during runtime!
 */
class KopeteOnlineStatus
{
public:
	/**
	 * The available global states. It is possible that multiple internal
	 * states map to the same global states. For example ICQ's 'Do not disturb'
	 * is handled just like 'Away' by libkopete. Only ICQ itself makes (and
	 * should make) a distinction.
	 * The order is important and is used in the < or > operator
	 */
	enum OnlineStatus
	{
		/**
		 * Refers to protocols where state cannot be determined. This
		 * applies to SMS contacts (text messages via mobile phones),
		 * since there's no presence information over SMS, but also
		 * to e.g. MSN contacts that are not on your contact list,
		 * since MSN only allows a user to query online state for
		 * users that are formally on the contact list. Lastly, libkopete
		 * itself uses the Unknown state in @ref KopeteMetaContact for
		 * meta contacts that have no child contacts at all.
		 */
		Unknown=0,
		/**
		 * State where you really cannot be contacted. Although
		 * Kopete doesn't oppose any technical limitations it really
		 * doesn't make sense to have more than one status per protocol
		 * that maps to 'Offline', since you're supposed to be
		 * disconnected from the network in this state.
		 */
		Offline=1,
		/**
		 * State where the user is not available on the network yet
		 * but trying to get onto. Most useful to yourself contact, because
		 * this state means not visible but with network access
		 */
		Connecting=2,
		/**
		 * Refers to a state where you can be technically reached, but
		 * for one reason or another it is often not useful to do so.
		 * This can be because you really aren't behind the computer
		 * ('Away' or 'Idle') or because you have other things to do
		 * and don't want to get involved in messaging ('Busy' or 'Do
		 * not Disturb' for example).
		 */
		Away=3,
		/**
		 * Refers to a true online state, i.e. you can be contacted by
		 * others both technically and practically. This also applies
		 * to e.g. ICQ's 'Free for Chat' status.
		 */
		Online=4
	};
	// note than Unknown is first, because the metacontact algorithm to detect
	// the metacontact status from the contact status starts from Unknown, and
	// takes a contact only if its status is greater

	/**
	 * Constructor.
	 *
	 * Creates an empty KopeteOnlineStatus object. Since you cannot change
	 * KopeteOnlineStatus objects that are already created other than by their
	 * assignment operator, this constructor is only a convenience method
	 * for use in e.g. class members and local variables.
	 */
	KopeteOnlineStatus();

	/**
	 * Constructor.
	 *
	 * Creates a new KopeteOnlineStatus object. All fields are mandatory; there
	 * are no default values. Also, you cannot change the object after creation.
	 *
	 * @param status is the global online status as used by libkopete
	 * @param weight is the 'weight' of this status. The contact list is
	 * sorted by status, and by weight within a status. It's not possible to
	 * 'promote' an Away item to a level above Online, since the status field
	 * always takes precedence. Weight is used when the same status is used
	 * more than once. Weight is also used for picking the most important
	 * 'Away' status for a protocol when going Away.
	 * @param protocol is a pointer to the protocol used. This is used when
	 * comparing two states using @ref operator=().
	 * @param internalStatus is the status as used internally by the protocol.
	 * This status is usually a lot more fine-grained than the status as used
	 * by libkopete and should be unique per protocol.
	 * @param overlayIcon is a string returning the name of the status icon to be
	 * used by the KDE icon loader. (Online and Offline don't have icon to overlay
	 * so you should QString::null as icon string)
	 * @param caption is the description of the status in menus and on buttons.
	 * @param description is a description in e.g. tooltips.
	 */
	KopeteOnlineStatus( OnlineStatus status, unsigned weight, KopeteProtocol *protocol,
		unsigned internalStatus, const QString &overlayIcon, const QString &caption, const QString &description );

	/**
	 * Constructor.
	 *
	 * Creates a libkopete builtin status object. Weight, protocol and internal
	 * status are set to zero, the strings and icons are set to the meta contact
	 * strings.
	 */
	KopeteOnlineStatus( OnlineStatus status );

	/**
	 * Copy constructor.
	 *
	 * Just adds a reference to the refcount. Used to copy around the status
	 * objects with very little overhead.
	 */
	KopeteOnlineStatus( const KopeteOnlineStatus &other );

	/**
	 * Destructor.
	 */
	~KopeteOnlineStatus();

	/**
	 * Return the status
	 */
	OnlineStatus status() const;

	/**
	 * Return the internal status
	 */
	unsigned internalStatus() const;

	/**
	 * Return the weight
	 */
	unsigned weight() const;

	/**
	 * Return the icon
	 */
	QString overlayIcon() const;

	/**
	 * Return the description
	 */
	QString description() const;

	/**
	 * Return the caption
	 */
	QString caption() const;

	/**
	 * Return the protocol this applies to
	 */
	KopeteProtocol* protocol() const;

	/**
	 * Return a status icon generated for the given KopeteContact
	 * This will draw an overlay representing the online status
	 * of the contact the KopeteOnlineStatus applies to
	 * over the base icon.
	 * A cache is employed to reduce CPU and memory usage.
	 * @param contact is the contact the icon should apply to.
	 * @param size is the size we the icon should be scaled to - 16 is default and so costs nothing
	 */
	QPixmap iconFor( const KopeteContact *contact, int size = 16 ) const;

	/**
	 * Return a status icon generated for the given KopeteAccount
	 * This will draw an overlay representing the online status
	 * of the account the KopeteOnlineStatus applies to
	 * over the base icon.
	 * A cache is employed to reduce CPU and memory usage.
	 * @param account is the account the icon should apply to.
	 * The account's color causes tinting, if it's plain QColor(), no tinting takes place.
	 * @param size is the size we the icon should be scaled to - 16 is default and so costs nothing
	 */
	QPixmap iconFor( const KopeteAccount *account, int size = 16 ) const;

	/**
	 * Returns the status icon for the protocol.
	 * A cache is employed to reduce CPU and memory usage.
	 */
	QPixmap protocolIcon() const;

	/**
	 * Assignment operator
	 */
	KopeteOnlineStatus & operator=( const KopeteOnlineStatus &other );

	/**
	 * Comparison operator
	 *
	 * Returns true if both the protocol and the internal status are
	 * identical.
	 */
	bool operator==( const KopeteOnlineStatus &other ) const;

	/**
	 * Comparison operator
	 *
	 * This operator works exactly opposite of @ref operator==()
	 */
	bool operator!=( const KopeteOnlineStatus &other ) const;

	/**
	 * Comparison operator
	 *
	 * Returns true if the status() of this contact is of higher value than the other
	 * contact or if both statuses are equal and weight() is higher for this contact.
	 */
	bool operator>( const KopeteOnlineStatus &other ) const;

	/**
	 * Comparison operator
	 *
	 * This operator works exactly opposite of @ref operator>()
	 */
	bool operator<( const KopeteOnlineStatus &other ) const;

private:
	KopeteOnlineStatusPrivate *d;
	QPixmap cacheLookup( const QString& icon, int size, QColor color, bool idle = false) const;
	friend class Kopete::OnlineStatusIconCache;
};

namespace Kopete
{

namespace Global
{
	OnlineStatusIconCache *onlineStatusIconCache();
}

class OnlineStatusIconCache : public QObject
{
	Q_OBJECT
	OnlineStatusIconCache();
public:
	~OnlineStatusIconCache();
	QPixmap cacheLookup( const KopeteOnlineStatus &statusFor, const QString& icon, int size, QColor color, bool idle = false);

signals:
	void iconsChanged();

private slots:
	void slotIconsChanged();

private:
	QPixmap* renderIcon( const KopeteOnlineStatus &statusFor, const QString& baseicon, int size, QColor color, bool idle = false) const;
	
	friend OnlineStatusIconCache *Global::onlineStatusIconCache();
	class Private;
	Private *d;
};

}

#endif

// vim: set noet ts=4 sts=4 sw=4:

