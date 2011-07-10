/*
    kopeteonlinestatus.h - Kopete Online Status

    Copyright (c) 2003      by Martijn Klingens <klingens@kde.org>
    Copyright (c) 2003      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2003      by Will Stephenson <wstephenson@kde.org>
    Copyright (c) 2004      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef kopeteonlinestatus_h
#define kopeteonlinestatus_h

#include "kopete_export.h"

#include <kdemacros.h>
#include <ksharedptr.h>
#include <kiconloader.h>

#include <QtCore/QObject>
#include <QtCore/QFlags>
#include <QtGui/QIcon>

#include "kopeteonlinestatusmanager.h"

class QString;
class QPixmap;
class QColor;

namespace Kopete
{
	class OnlineStatusManager;

	class Identity;
	class Protocol;
	class Account;
	class Contact;

/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Will Stephenson (icon generating code)
 *
 * OnlineStatus is a class that encapsulates all information about the
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
class KOPETE_EXPORT OnlineStatus
{
public:
	/**
	 * The available global states. It is possible that multiple internal
	 * states map to the same global states. For example ICQ's 'Do not disturb'
	 * is handled just like 'Away' by libkopete. Only ICQ itself makes (and
	 * should make) a distinction.
	 * The order is important and is used in the < or > operator
	 */
	enum StatusType
	{
		/**
		 * Refers to protocols where state cannot be determined. This
		 * applies to SMS contacts (text messages via mobile phones),
		 * since there's no presence information over SMS, but also
		 * to e.g. MSN contacts that are not on your contact list,
		 * since MSN only allows a user to query online state for
		 * users that are formally on the contact list. Lastly, libkopete
		 * itself uses the Unknown state in @ref MetaContact for
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
		Offline=10,
		/**
		 * State where the user is not available on the network yet
		 * but trying to get onto. Most useful to yourself contact, because
		 * this state means not visible but with network access
		 */
		Connecting=20,
		/**
		 * State where you are online but none of your contacts can
		 * see that you're online. Useful for all the protocols that support
		 * being invisible.
		 */
		Invisible=30,
		/**
		 * Refers to a state where you can be technically reached, but
		 * for one reason or another it is often not useful to do so.
		 * This will be because you are not in front of the computer
		 * or because the client detected you don't interact.
		 */
		Away=40,
		/**
		 * Means that you have other things to do
		 * and don't want to get involved in messaging ('Busy' or 'Do
		 * not Disturb' for example).
		 */
		Busy=45,
		/**
		 * Refers to a true online state, i.e. you can be contacted by
		 * others both technically and practically. This also applies
		 * to e.g. ICQ's 'Free for Chat' status.
		 */
		Online=50
	};
	// note than Unknown is first, because the metacontact algorithm to detect
	// the metacontact status from the contact status starts from Unknown, and
	// takes a contact only if its status is greater

	/**
	 * Reserved internal status values
	 *
	 * Any internal status value > 0x80000000 is reserved for internal
	 * libkopete use. This enumeration lists the currently known values.
	 */
	enum ReservedInternalStatus
	{
		/**
		 * The account this contact belongs to is offline. Used with
		 * the Unknown StatusType.
		 */
		AccountOffline = 0x80000001
	};


	/**
	 * Constructor.
	 *
	 * Creates an empty OnlineStatus object. Since you cannot change
	 * OnlineStatus objects that are already created other than by their
	 * assignment operator, this constructor is only a convenience method
	 * for use in e.g. class members and local variables.
	 */
	OnlineStatus();


	/**
	 * Constructor.
	 *
	 * Creates a new OnlineStatus object. All fields are mandatory; there
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
	 * comparing two online status objects.
	 * @param internalStatus is the status as used internally by the protocol.
	 * This status is usually a lot more fine-grained than the status as used
	 * by libkopete and should be unique per protocol.
	 * @param overlayIcons is a list of QStrings which are the name of status
	 * icons to be used by the KDE icon loader. (Statuses which don't have icons
	 * to overlay like Online and Offline should use QString() as icon
	 * name ).  NOTE if the string is a movie ( *.mng ) it must be the first string in the list.
	 * TODO: KDE4 sort out movies and overlay icons.
	 * @param description is a description in e.g. tooltips.
	 */
	OnlineStatus( StatusType status, unsigned weight, Protocol *protocol,
		unsigned internalStatus, const QStringList &overlayIcons, const QString &description );

	/**
	 * Constructor.
	 *
	 * @p Creates a new OnlineStatus object and registers it with the @ref Kopete::OnlineStatusManager.
	 * Registration allows you to generate a KActionMenu filled with KActions for changing to this OnlineStatus,
	 * using Kopete::Account::accountMenu().
	 *
	 * @p Note that weight has an additional significance for registered protocols when used for menu generation.
	 *
	 * All fields are mandatory; there
	 * are no default values. Also, you cannot change the object after creation.
	 *
	 * @param status is the global online status as used by libkopete
	 * @param weight is the 'weight' of this status. The contact list is
	 * sorted by status, and by weight within a status. It's not possible to
	 * 'promote' an Away item to a level above Online, since the status field
	 * always takes precedence. Weight is used when the same status is used
	 * more than once. Weight is also used for picking the most important
	 * 'Away' status for a protocol when going Away. Additionally, Weight determinesis also
	 * @param protocol is a pointer to the protocol used. This is used when
	 * comparing two online status objects.
	 * @param internalStatus is the status as used internally by the protocol.
	 * This status is usually a lot more fine-grained than the status as used
	 * by libkopete and should be unique per protocol.
	 * @param overlayIcon is a string returning the name of the status icon to be
	 * used by the KDE icon loader. (Status whiwh doesn't have icon to overlay like
	 * Online and Offline should use QString() as icon string)
	 * @param description is a description in e.g. tooltips.
	 * @param caption is the text of the action in the menu
	 * @param categories the categories this online status is in
	 * @param options the options of this online status
	 * @see Kopete::OnlineStatusManager for more info about the categories and options parameters
	 *
	 * You can set the status to be in the predefined categories.
	 * Ideally, each category should own one status.
	 * A status may be in several categories, or in none.
	 * There shouldn't be more than one status per protocol per categories.
	 */
	OnlineStatus( StatusType status, unsigned weight, Protocol *protocol, unsigned internalStatus, const QStringList &overlayIcon,
		const QString &description, const QString& caption, OnlineStatusManager::Categories categories = 0x0 , OnlineStatusManager::Options options = 0x0 );


	/**
	 * Constructor.
	 *
	 * Creates a libkopete builtin status object. Weight, protocol and internal
	 * status are set to zero, the strings and icons are set to the meta contact
	 * strings.
	 */
	OnlineStatus( StatusType status ); /* implicit */

	/**
	 * Copy constructor.
	 *
	 * Just adds a reference to the refcount. Used to copy around the status
	 * objects with very little overhead.
	 */
	OnlineStatus( const OnlineStatus &other );

	/**
	 * Destructor.
	 */
	~OnlineStatus();

	/**
	 * \brief Return the status
	 */
	StatusType status() const;

	/**
	 * \brief Return the internal status
	 */
	unsigned internalStatus() const;

	/**
	 * \brief Return the weight
	 */
	unsigned weight() const;

	/**
	 * \brief Return the list of overlay icons
	 */
	QStringList overlayIcons() const;

	/**
	 * \brief Return the description
	 */
	QString description() const;

	/**
	 * \brief Return the protocol this applies to
	 */
	Protocol* protocol() const;

	/**
	 * \brief Return the text for the action in the menu
	 */
	QString caption() const;

	/**
	 * \brief Return the categories this online status is in
	 *
	 * @see Kopete::OnlineStatusManager for more info about the categories
	 */
	OnlineStatusManager::Categories categories() const;

	/**
	 * \brief Return the options of this online status
	 *
	 * @see Kopete::OnlineStatusManager for more info about the options parameters
	 */
	OnlineStatusManager::Options options() const;

	/**
	 * @return @c true if this a contact with this status is definitely online,
	 *         @c false if the contact is Offline, Connecting or Unknown.
	 */
	bool isDefinitelyOnline() const;


	

	/**
	 * \brief Return a status icon generated for the given Contact
	 *
	 * This will draw an overlay representing the online status
	 * of the contact the OnlineStatus applies to
	 * over the base icon.
	 * A cache is employed to reduce CPU and memory usage.
	 * @param contact is the contact the icon should apply to.
	 */
	QIcon iconFor( const Contact *contact ) const;

	/**
	 * \brief Return a status icon generated for the given Contact
	 * \overload 
	 * \deprecated  Use the one that return a QIcon
	 * @param contact is the contact the icon should apply to.
	 * @param size is the size we the icon should be scaled to
	 */
	KDE_DEPRECATED QPixmap iconFor( const Contact *contact, int size ) const
	{ return iconFor(contact).pixmap(size); }

	/**
	 * \brief Return the mime source for a status icon generated for the given Contact
	 *
	 * This behaves essentially like the method above, except for that
	 * it returns a mime source string that can be used to render the
	 * image in richtext components and the like. The returned key
	 * is only valid until the cache is cleared for the next time,
	 * so no assumptions should be made about long-time availability
	 * of the referenced data.
	 * @param contact is the contact the icon should apply to.
	 * @param size is the size we the icon should be scaled to - 16 is default and so costs nothing
	 */
	QString mimeSourceFor( const Contact *contact, int size = 16 ) const;

	/**
	 * \brief Return a status icon generated for the given Account
	 *
	 * This will draw an overlay representing the online status
	 * of the account the OnlineStatus applies to
	 * over the base icon.
	 * A cache is employed to reduce CPU and memory usage.
	 * @param account is the account the icon should apply to.
	 * The account's color causes tinting, if it's plain QColor(), no tinting takes place.
	 * @param size is the size we the icon should be scaled to - 16 is default and so costs nothing
	 */
	QIcon iconFor( const Account *account ) const;

	/**
	 * \brief Return a status icon generated for the given Account
	 * \overload
	 * \deprecated  Use the varient which return a QIcon
	 * 
	 * @param account is the account the icon should apply to.
	 * The account's color causes tinting, if it's plain QColor(), no tinting takes place.
	 * @param size is the size we the icon should be scaled to - 16 is default and so costs nothing
	 */
	KDE_DEPRECATED QPixmap iconFor( const Account *account, int size  ) const
	{ return iconFor(account).pixmap(size); }

	/**
	 * \brief Return the mime source for a status icon generated for the given Account
	 *
	 * This behaves essentially like the method above, except for that
	 * it returns a mime source string that can be used to render the
	 * image in richtext components and the like. The returned key
	 * is only valid until the cache is cleared for the next time,
	 * so no assumptions should be made about long-time availability
	 * of the referenced data.
	 * @param account is the account the icon should apply to.
	 * The account's color causes tinting, if it's plain QColor(), no tinting takes place.
	 * @param size is the size we the icon should be scaled to - 16 is default and so costs nothing
	 */
	QString mimeSourceFor( const Account *account, int size = 16 ) const;

	/**
	 * \brief Return a previously rendered status icon for a mime source key
	 *
	 * You can access icons with this method that have previously been rendered
	 * using mimeSourceFor(). Note that only a cache lookup will be done, so
	 * if the cache has been invalidated due to a change of icon sets between
	 * requesting the key (thus rendering the icon) and trying to access the
	 * icon by key, an invalid pixmap will be returned.
	 */
	QPixmap iconFor( const QString &mimeSource ) const;

	/**
	 * \brief Returns the status icon for the protocol.
	 *
	 * A cache is employed to reduce CPU and memory usage.
	 */
	QPixmap protocolIcon(const KIconLoader::StdSizes size) const;

	/**
	 * \brief Returns the status icon for the protocol.
	 *
	 * A cache is employed to reduce CPU and memory usage.
	 */
	KDE_DEPRECATED QPixmap protocolIcon() const;

	/**
	 * Assignment operator
	 */
	OnlineStatus & operator=( const OnlineStatus &other );

	/**
	 * Comparison operator
	 *
	 * Returns true if both the protocol and the internal status are
	 * identical.
	 */
	bool operator==( const OnlineStatus &other ) const;

	/**
	 * Comparison operator
	 *
	 * This operator works exactly opposite of @ref operator==()
	 */
	bool operator!=( const OnlineStatus &other ) const;

	/**
	 * Comparison operator
	 *
	 * Returns true if the status() of this contact is of higher value than the other
	 * contact or if both statuses are equal and weight() is higher for this contact.
	 */
	bool operator>( const OnlineStatus &other ) const;

	/**
	 * Comparison operator
	 *
	 * This operator works exactly opposite of @ref operator>()
	 */
	bool operator<( const OnlineStatus &other ) const;

	/**
	 * \brief returns a QString from a StatusType
	 *
	 * Static method to convert a Kopete::OnlineStatus::StatusType to a string to avoid
	 * many issues when saving StatusType to disk
	 */
	 static QString statusTypeToString(OnlineStatus::StatusType status);

	 /**
	 * \brief returns a StatusType from a QString
	 *
	 * Static method to convert a QString representing a StatusType to a StatusType to avoid
	 * many issues when saving StatusType to disk
	 */
	 static OnlineStatus::StatusType statusStringToType(const QString& string);



private:

	class Private;
	KSharedPtr<Private> d;

	QString mimeSource( const QString& icon, int size, QColor color, bool idle) const;


};

}  //END namespace Kopete

#endif


