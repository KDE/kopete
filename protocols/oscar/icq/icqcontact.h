/*
  icqcontact.h  -  ICQ Contact

  Copyright (c) 2003 by Stefan Gehn  <metz AT gehn.net>
  Copyright (c) 2003 by Olivier Goffart
  Copyright (c) 2004 by Richard Smith               <kde@metafoo.co.uk>
  Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

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

#include "oscarcontact.h"
#include "userdetails.h"

class KAction;
class KToggleAction;
namespace Kopete { class ChatSession; }
namespace Kopete { class OnlineStatus; }
class ICQProtocol;
class ICQAccount;
class OscarAccount;
class ICQUserInfo; // user info dialog
class ICQReadAway;

class ICQGeneralUserInfo;
class ICQWorkUserInfo;
class ICQUserInfoWidget;

/**
 * Contact for ICQ over Oscar protocol
 * @author Stefan Gehn
 * @author Richard Smith
 * @author Matt Rogers
 */
class ICQContact : public OscarContact
{
Q_OBJECT

public:

	/** Normal ICQ constructor */
	ICQContact( ICQAccount *account, const QString &name, Kopete::MetaContact *parent,
	            const QString& icon = QString::null, const Oscar::SSI& ssiItem = Oscar::SSI()  );
	virtual ~ICQContact();
	
	/**
	 * Returns a set of custom menu items for
	 * the context menu
	 */
	virtual QPtrList<KAction> *customContextMenuActions();
	
	/** Return whether or not this contact is reachable. */
	virtual bool isReachable();
	

	//virtual const QString awayMessage();
	//virtual void setAwayMessage(const QString &message);
	
public slots:
	virtual void slotUserInfo();
	virtual void updateSSIItem();
	void userInfoUpdated( const QString& contact, const UserDetails& details );
	
	void userOnline( const QString& userId );
	void userOffline( const QString& userID );
	void loggedIn();
	
	void requestShortInfo();
	
signals:
	void haveBasicInfo( const ICQGeneralUserInfo& );
	void haveWorkInfo( const ICQWorkUserInfo& );
	void haveEmailInfo( const ICQEmailInfo& );
	void haveMoreInfo( const ICQMoreUserInfo& );

private:
	ICQProtocol *mProtocol;
	ICQUserInfoWidget* m_infoWidget;
	/*
	ICQReadAway *awayMessageDialog;
	KAction *actionReadAwayMessage;
	*/
	KAction *actionRequestAuth;
	KAction *actionSendAuth;
	/*
	KToggleAction *actionIgnore;
	KToggleAction *actionVisibleTo;

	bool mInvisible;
	*/

private slots:
	/** Request authorization from this contact */
	void slotRequestAuth();
	
	/** Authorize this contact */
	void slotSendAuth();
	
	/** We have received an auth request */
	void slotGotAuthRequest( const QString& contact, const QString& reason );
	
	/** We have received an auth reply */
	void slotGotAuthReply( const QString& contact, const QString& reason, bool granted );
	
	void closeUserInfoDialog();
	
	void receivedLongInfo( const QString& contact );
	void receivedShortInfo( const QString& contact );
	
//void slotCloseAwayMessageDialog();
	//void slotReadAwayMessage();
	
	//void slotIgnore();
	//void slotVisibleTo();
};

#endif
//kate: tab-width 4; indent-mode csands; space-indent off; replace-tabs off;
