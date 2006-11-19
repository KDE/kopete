/*
  icqcontact.h  -  ICQ Contact

  Copyright (c) 2003 by Stefan Gehn  <metz AT gehn.net>
  Copyright (c) 2003 by Olivier Goffart
  Copyright (c) 2004 by Richard Smith               <kde@metafoo.co.uk>
  Copyright (c) 2006 by Roman Jarosz <kedgedev@centrum.cz>
  Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
  */

#ifndef ICQCONTACT_H
#define ICQCONTACT_H

#include "icqcontactbase.h"

class ICQProtocol;
class ICQUserInfoWidget;

/**
 * Contact for ICQ over Oscar protocol
 * @author Stefan Gehn
 * @author Richard Smith
 * @author Matt Rogers
 */
class ICQContact : public ICQContactBase
{
Q_OBJECT

public:

	/** Normal ICQ constructor */
	ICQContact( Kopete::Account* account, const QString &name, Kopete::MetaContact *parent,
	            const QString& icon = QString::null, const OContact& ssiItem = OContact()  );
	virtual ~ICQContact();

	/**
	 * Returns a set of custom menu items for
	 * the context menu
	 */
	virtual QList<KAction*> *customContextMenuActions();

	/** Return whether or not this contact is reachable. */
	virtual bool isReachable();

public slots:
	virtual void slotUserInfo();
	virtual void updateSSIItem();
	void userInfoUpdated( const QString& contact, const UserDetails& details );

	void userOnline( const QString& userId );
	void userOffline( const QString& userID );
	void loggedIn();

signals:
	void haveBasicInfo( const ICQGeneralUserInfo& );
	void haveWorkInfo( const ICQWorkUserInfo& );
	void haveEmailInfo( const ICQEmailInfo& );
	void haveNotesInfo( const ICQNotesInfo& );
	void haveMoreInfo( const ICQMoreUserInfo& );
	void haveInterestInfo( const ICQInterestInfo& );
	void haveOrgAffInfo( const ICQOrgAffInfo& );

private:	
	ICQProtocol *mProtocol;
	ICQUserInfoWidget* m_infoWidget;

	KAction *actionRequestAuth;
	KAction *actionSendAuth;
    KAction *m_selectEncoding;
	
	KToggleAction *m_actionIgnore;
	KToggleAction *m_actionVisibleTo;
	KToggleAction *m_actionInvisibleTo;

private slots:
	/** Request authorization from this contact */
	void slotRequestAuth();

	/** Authorize this contact */
	void slotSendAuth();

	void slotAuthReplyDialogOkClicked();

	/** We have received an auth request */
	void slotGotAuthRequest( const QString& contact, const QString& reason );

	/** We have received an auth reply */
	void slotGotAuthReply( const QString& contact, const QString& reason, bool granted );

	void closeUserInfoDialog();

	void receivedLongInfo( const QString& contact );

	void slotIgnore();
	void slotVisibleTo();
	void slotInvisibleTo();
};

#endif
//kate: tab-width 4; indent-mode csands; space-indent off; replace-tabs off;
