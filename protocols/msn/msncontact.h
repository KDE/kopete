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

class QListView;
class QListViewItem;
class QPixmap;
class QTimer;

class KAction;
class KActionCollection;
class KListAction;

class MSNContact : public KopeteContact
{
	Q_OBJECT

public:
	MSNContact( KopeteProtocol *protocol, const QString &id,
		const QString &displayName, KopeteMetaContact *parent );
	~MSNContact();

	virtual QString data() const;

	ContactStatus status() const;
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
	 * Indicate whether this contact is currently moving to another group
	 * Flag needed for don't delete the contact if he is removed from a group
	 *  and not yet add to the other group  ( when the group doesn't exists yet)
	 */
	bool isMoving() { return m_moving; }
	void setMoving(bool b=true) { m_moving=b; }


	/**
	 * set one phone number
	 */
	void setInfo(QString type, QString data);

	/**
	 * The groups in which the user is located on the server.
	 */
	QValueList<unsigned int> groups();

	/**
	 * The contact's MSN specific status, this is NOT the ContactStatus!
	 * FIXME: Status handling needs serious redesign, probably!
	 */
	MSNProtocol::Status msnStatus() const;
	void setMsnStatus( MSNProtocol::Status status );

	virtual void addThisTemporaryContact(KopeteGroup *group=0l);

	virtual bool isReachable() { return true; };

	virtual KActionCollection *customContextMenuActions();

public slots:

	virtual void slotUserInfo();
	virtual void slotDeleteContact();
	virtual void execute();
	virtual void slotSendFile(QString fileName);

	void slotRemovedFromGroup(unsigned int);
	void slotAddedToGroup(unsigned int);

	/**
	 * Add/Remove user to/from a group
	 */
	void addToGroup( KopeteGroup * );
	void removeFromGroup( KopeteGroup * );
	void moveToGroup( KopeteGroup * , KopeteGroup * );

signals:
	void chatToUser( QString );

private slots:
	void slotBlockUser();

	void slotMoved(KopeteMetaContact* from);

private:
	QValueList<unsigned int> m_groups;

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

