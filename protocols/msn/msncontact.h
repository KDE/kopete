/*
    msncontact.h - MSN Contact

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Ryan Cumming           <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002      by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>


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
#include "kopeteonlinestatus.h"

#include <kurl.h>

class QListView;
class QListViewItem;
class QPixmap;
class QTimer;

class MSNChatSession;
class KAction;
class KActionCollection;
class KTempFile;

namespace Kopete { class Protocol; }
namespace Kopete { class OnlineStatus; }

class MSNContact : public Kopete::Contact
{
	Q_OBJECT

public:
	MSNContact( Kopete::Account *account, const QString &id, Kopete::MetaContact *parent );
	~MSNContact();

	/**
	 * Indicate whether this contact is blocked
	 */
	bool isBlocked() const;
	void setBlocked( bool b );

	/**
	 * Indicate whether this contact is deleted
	 *   (not on the serverside list)
	 */
	bool isDeleted() const;
	void setDeleted( bool d );

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
	void setInfo(const QString &type, const QString &data);

	/**
	 * The groups in which the user is located on the server.
	 */
	const QMap<QString, Kopete::Group *>  serverGroups() const;
	/**
	 * clear that map
	 */
	void clearServerGroups();
	
	/**
	 * client flags  (say what version of msn messenger the contact is using)
	 */
	uint clientFlags() const;
	void setClientFlags( uint );

	virtual bool isReachable();

	virtual QPtrList<KAction> *customContextMenuActions();

	/**
	 * update the server group map
	 */
	void contactRemovedFromGroup( const QString& groupId );
	void contactAddedToGroup(const QString& groupId, Kopete::Group *group );

	virtual void serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData );

	/**
	 * Rename contact on server
	 */
	virtual void rename( const QString &newName ) KDE_DEPRECATED;

	/**
	 * Returns the MSN Message Manager associated with this contact
	 */
	virtual Kopete::ChatSession *manager( Kopete::Contact::CanCreateFlags = Kopete::Contact::CannotCreate );


	/**
	 * Because blocked contact have a small auto-modified status
	 */
	void setOnlineStatus(const Kopete::OnlineStatus&);

	QString guid();
	QString phoneHome();
	QString phoneWork();
	QString phoneMobile();

	void setObject(const QString &obj);
	QString object() const { return m_obj; }

public slots:
	virtual void slotUserInfo();
	virtual void deleteContact();
	virtual void sendFile( const KURL &sourceURL = KURL(),
						   const QString &fileName = QString::null, uint fileSize = 0L );

	/**
	 * Every time the kopete's contactlist is modified, we sync the serverlist with it
	 */
	virtual void sync( unsigned int cvhanged= 0xff);


	void setDisplayPicture(KTempFile *f) ;

signals:
	void displayPictureChanged();

private slots:
	void slotBlockUser();
	void slotShowProfile();
	void slotSendMail();
	void slotEmitDisplayPictureChanged();

	/**
	 * Workaround to make this checkboxe readonly
	 */
	void slotUserInfoDialogReversedToggled();

private:
	QMap<QString, Kopete::Group *> m_serverGroups;

	bool m_blocked;
	bool m_allowed;
	bool m_deleted;
	bool m_reversed;
	bool m_moving;
	bool m_phone_mob;
	
	uint m_clientFlags;

	QString m_phoneHome;
	QString m_phoneWork;
	QString m_phoneMobile;


	KAction *actionBlock;
	KAction *actionShowProfile;
	KAction *actionSendMail;
	KAction *actionWebcamReceive;
	KAction *actionWebcamSend;

	QString m_obj; //the MSNObject

	/**
	 * keep the current status here.  (it's normally already in Kopete::Contact::d->onlineStatus)
	 * This is a workaround to prevent problems with the account offline status.
	 */
	Kopete::OnlineStatus m_currentStatus;

	//MSNProtocol::deserializeContact need to acess some contact insternals
	friend class MSNProtocol;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

