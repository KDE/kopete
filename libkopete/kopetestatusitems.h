/*
    kopetestatusitems.h - Kopete Status Items

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
#ifndef KOPETESTATUSITEMS_H
#define KOPETESTATUSITEMS_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

#include "kopete_export.h"
#include "kopeteonlinestatusmanager.h"

namespace Kopete {

namespace Status {

class StatusGroup;

/**
 * StatusItem is a base class for all status items. The items store
 * values that are needed to build status menu.
 *
 * The items are stored in StatusManager.
 *
 * IdentityManager is a singleton, you may uses it with @ref IdentityManager::self()
 *
 *@author Roman Jarosz <kedgedev@centrum.cz>
 */
class KOPETE_EXPORT StatusItem : public QObject
{
	Q_OBJECT
public:
	/**
	 * StatusItem constructor
	 **/
	StatusItem();
	StatusItem( const QString& uid );

	/**
	 * Sets category
	 **/
	void setCategory( OnlineStatusManager::Categories category );

	/**
	 * Returns category
	 **/
	OnlineStatusManager::Categories category() const { return mCategory; }

	/**
	 * Sets title
	 **/
	void setTitle( const QString& title );

	/**
	 * Returns title
	 **/
	QString title() const { return mTitle; }

	/**
	 * Returns unique identifier
	 **/
	QString uid() const { return mUid; }

	/**
	 * Returns true if StatusItem is group
	 **/
	virtual bool isGroup() const = 0;
	
	/**
	 * Returns number of childes
	 **/
	virtual int childCount() const = 0;

	/**
	 * Returns a StatusItem at given @p index or 0 if there is no item at given @p index
	 **/
	virtual StatusItem *child( int /*index*/ ) const = 0;

	/**
	 * Returns index of this Item in parent group
	 **/
	int index() const;

	/**
	 * Returns StatusGroup this Item belongs to
	 **/
	StatusGroup *parentGroup() const;

	/**
	 * Creates a copy of StatusItem
	 *
	 * @note this copies also uid so it should only be used when we know
	 * that the original or copy object will be destroyed
	 **/
	virtual StatusItem* copy() const = 0;

Q_SIGNALS:
	/**
	 * This signal is emitted whenever the item's content changes
	 **/
	void changed();

private:
	OnlineStatusManager::Categories mCategory;
	QString mTitle;
	QString mUid;

	StatusGroup *mParentItem;
	Q_DISABLE_COPY(StatusItem)
};

/**
 * StatusGroup represents a group that can contain other StatusItems
 *
 *@author Roman Jarosz <kedgedev@centrum.cz>
 */
class KOPETE_EXPORT StatusGroup : public StatusItem
{
	Q_OBJECT
public:
	/**
	 * StatusGroup constructor
	 **/
	StatusGroup();
	StatusGroup( const QString& uid );

	/**
	 * Returns true if StatusItem is group
	 * @note for StatusGroup it always returns true;
	 **/
	virtual bool isGroup() const { return true; }
	
	/**
	 * Returns number of childes
	 **/
	virtual int childCount() const { return mChildItems.count(); }

	/**
	 * Returns a StatusItem at given @p index or 0 if there is no item at given @p index
	 **/
	virtual StatusItem *child( int index ) const { return mChildItems.value( index, 0 ); }

	/**
	 * Returns list of all childes
	 **/
	QList<StatusItem*> childList() const { return mChildItems; }

	/**
	 * Returns index for given StatusItem
	 **/
	int indexOf( StatusItem *child ) const { return mChildItems.indexOf( child ); }

	/**
	 * Inserts @p child at given @p index
	 **/
	void insertChild( int index, StatusItem *child );

	/**
	 * Inserts @p child at the end
	 **/
	void appendChild( Kopete::Status::StatusItem *child );

	/**
	 * Removes @p child
	 **/
	void removeChild( Kopete::Status::StatusItem *child );

	/**
	 * Creates a copy of this object
	 *
	 * @note this copies also uid so it should only be used when we know
	 * that the original or copy object will be destroyed
	 **/
	virtual StatusItem* copy() const;
Q_SIGNALS:
	/**
	 * This signal is emitted after new child was inserted is inserted at position @p index
	 **/
	void childInserted( int index, Kopete::Status::StatusItem *child );

	/**
	 * This signal is emitted after child was removed
	 **/
	void childRemoved( Kopete::Status::StatusItem *child );

private Q_SLOTS:
	void childDestroyed( QObject *object );
	
private:
	QList<StatusItem*> mChildItems;
};

/**
 * Status represents a status which has title, message and category.
 * Values from this class are used to create status action with which user can change status.
 *
 *@author Roman Jarosz <kedgedev@centrum.cz>
 */
class KOPETE_EXPORT Status : public StatusItem
{
	Q_OBJECT
public:
	/**
	 * Status constructor
	 **/
	Status();
	Status( const QString& uid );

	/**
	 * Returns true if the item is group
	 * @note for Status it always returns false;
	 **/
	virtual bool isGroup() const { return false; }

	/**
	 * Returns number of childes
	 * @note for Status it always returns 0;
	 **/
	virtual int childCount() const { return 0; }

	/**
	 * Returns the item at given @p index or 0 if there is no item at given @p index
	 * @note for Status it always returns 0;
	 **/
	virtual StatusItem *child( int ) const { return 0; }

	/**
	 * Set message
	 **/
	void setMessage( const QString& message );

	/**
	 * Returns message
	 **/
	QString message() const { return mMessage; }

	/**
	 * Creates a copy of this object
	 *
	 * @note this copies also uid so it should only be used when we know
	 * that the original or copy object will be destroyed
	 **/
	virtual StatusItem* copy() const;

private:
	QString mMessage;
};

}

}

#endif
