/*
    Kopete Contactlist Model Private Items

    Copyright     2009      by Roman Jarosz           <kedgedev@gmail.com>

    Kopete    (c) 2009     by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_UI_CONTACTLISTTREEMODEL_P_H
#define KOPETE_UI_CONTACTLISTTREEMODEL_P_H

#include <QPointer>

namespace Kopete {

class Group;
class MetaContact;

namespace UI {

class MetaContactModelItem;
class GroupModelItem;
class ContactListModelItem;

class ContactListModelItem
{
public:
	ContactListModelItem() : mParent( 0 ) {}
	virtual ~ContactListModelItem() {}

	virtual bool isGroup() const
	{
		return false;
	}

	virtual int count() const
	{
		return 0;
	}

	virtual int metaContactCount() const
	{
		return 0;
	}

	virtual bool hasChildren() const
	{
		return false;
	}

	inline GroupModelItem* parent() const
	{
		return mParent;
	}

	int index() const;

	bool remove();

	virtual void sort( bool (*lessThan)(const ContactListModelItem*, const ContactListModelItem*) )
	{
		Q_UNUSED( lessThan );
	}

protected:
	friend class GroupModelItem;
	GroupModelItem* mParent;
};

class MetaContactModelItem : public ContactListModelItem
{
public:
	MetaContactModelItem( Kopete::MetaContact* metaContact )
		: ContactListModelItem(), mMetaContact( metaContact )
	{}

	inline Kopete::MetaContact* metaContact() const
	{
		return mMetaContact;
	}

private:
	QPointer <Kopete::MetaContact> mMetaContact;
};

class GroupModelItem : public ContactListModelItem
{
public:
	GroupModelItem( Kopete::Group* group )
		: ContactListModelItem(), mGroup( group )
	{}

	virtual ~GroupModelItem()
	{
		qDeleteAll( mItems );
	}

	virtual bool isGroup() const
	{
		return true;
	}

	virtual int count() const
	{
		return mItems.count();
	}

	virtual int metaContactCount() const;

	virtual bool hasChildren() const
	{
		return !mItems.isEmpty();
	}

	inline Kopete::Group* group() const
	{
		return mGroup;
	}

	inline void append( ContactListModelItem* item )
	{
		item->mParent = this;
		mItems.append( item );
	}

	inline void insert( int i, ContactListModelItem* item )
	{
		item->mParent = this;
		mItems.insert( i, item );
	}

	inline ContactListModelItem* at( int i ) const
	{
		return mItems.at( i );
	}

	inline QList<ContactListModelItem*> items() const
	{
		return mItems;
	}

	virtual void sort( bool (*lessThan)(const ContactListModelItem*, const ContactListModelItem*) );

protected:
	friend class ContactListModelItem;
	int indexOf( const ContactListModelItem* item ) const;

	inline void removeAt( int i )
	{
		mItems.removeAt( i );
	}

private:
	QPointer <Kopete::Group> mGroup;
	QList<ContactListModelItem*> mItems;
};

}

}

#endif
//kate: tab-width 4
