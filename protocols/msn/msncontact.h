/*
    msncontact.h - MSN Contact

    Copyright (c) 2002 Duncan Mac-Vicar Prett <duncan@kde.org>
              (c) 2002 Ryan Cumming           <bodnar42@phalynx.dhs.org>
              (c) 2002 Martijn Klingens       <klingens@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNCONTACT_H
#define MSNCONTACT_H

#include "kopetecontact.h"
#include "msnprotocol.h"
#include <kurl.h>

class QListView;
class QListViewItem;
class QPixmap;
class QTimer;

class MSNMessageManager;
class KAction;
class KActionCollection;
class KListAction;

class MSNContact : public KopeteContact
{
	Q_OBJECT

public:
	MSNContact( KopeteIdentity *identity, const QString &id,
		const QString &displayName, KopeteMetaContact *parent );

	~MSNContact();

	QString statusText() const;
	QString statusIcon() const;
	int importance() const;

	/**
	 * Indicate whether this contact is blocked
	 */
	bool isBlocked() const;
	void setBlocked( bool b );

	/**
	 * Indicate whether this contact is deleted
	 */
/*	bool isDeleted() const;
	void setDeleted( bool d );*/

	/**
	 * Indicate whether this contact is allowed
	 */
	bool isAllowed() const;
	void setAllowed( bool d );

	/**
	 * Indicate whether this contact is on the reversed list
	 */
	bool isReversed() const;
	void setReversed( bool d );

	/**
	 * set one phone number
	 */
	void setInfo(QString type, QString data);

	/**
	 * The groups in which the user is located on the server.
	 */
	const QMap<uint, KopeteGroup *> & serverGroups() const;

	/**
	 * The same list that @ref serverGroups() returns, but now in a format
	 * that libkopete understands. It is in theory possible to make
	 * MSNProtocol use this method too, but @ref serverGroups() is a lot
	 * closer to our internal data structures and hence a lot more
	 * efficient.
	 */
	virtual KopeteGroupList groups() const;

	/**
	 * The contact's MSN specific status, this is NOT the ContactStatus!
	 * FIXME: Status handling needs serious redesign, probably!
	 */
	MSNProtocol::Status msnStatus() const;
	void setMsnStatus( MSNProtocol::Status status );

	virtual bool isReachable() { return true; };

	virtual KActionCollection *customContextMenuActions();

	/**
	 * Add/Remove user to/from a group
	 */
	virtual void addToGroup( KopeteGroup *newGroup );
	virtual void removeFromGroup( KopeteGroup *group );
	virtual void moveToGroup( KopeteGroup *oldGroup, KopeteGroup *newGroup );

	void contactAddedToGroup( uint groupNumber, KopeteGroup *group );

	virtual void serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData );

	void removeFromGroup( unsigned int group );

	/**
	 * Rename contact on server
	 */
	virtual void rename( const QString &newName );

	/**
	 * Returns the MSN Message Manager associated with this contact
	 */
	virtual KopeteMessageManager *manager( bool canCreate = false );


	/**
	 * MSNIdentity and MSNSwhitchBoardSocket need to change the displayName of contacts.
	 * Then, we do this fuction public  
	 **/
	void setDisplayName(const QString &Dname)
	{
		KopeteContact::setDisplayName(Dname);
	}

public slots:
	virtual void slotUserInfo();
	virtual void slotDeleteContact();
	virtual void sendFile(const KURL &sourceURL, const QString &altFileName, const long unsigned int fileSize);

private slots:
	void slotBlockUser();

private:
	QMap<uint, KopeteGroup *> m_serverGroups;

	bool m_blocked;
	bool m_allowed;
//	bool m_deleted;
	bool m_reversed;

	MSNProtocol::Status m_status;

	KActionCollection* m_actionCollection;
	KAction* m_actionBlock;

	bool m_moving;

	QString m_phoneHome;
	QString m_phoneWork;
	QString m_phoneMobile;
	bool m_phone_mob;
};

#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

