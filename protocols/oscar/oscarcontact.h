/***************************************************************************
                          oscarcontact.h  -  description
                             -------------------
    begin                : Tue Jul 30 2002
    copyright            : (C) 2002 by twl6
    email                : twl6@po.cwru.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OSCARCONTACT_H
#define OSCARCONTACT_H

#include <qwidget.h>
#include "kopetecontact.h"
#include "kopetemessage.h"

/**Contact for oscar protocol
  *@author twl6
  */

struct UserInfo;
class OscarProtocol;
class KopeteMessageManager;
class OscarProtocol;
class  KopeteHistoryDialog;

class OscarContact : public KopeteContact  {
   Q_OBJECT
public: 
	OscarContact(const QString name, OscarProtocol *protocol,	KopeteMetaContact *parent);
	~OscarContact();
  /** Return the unique id that identifies a contact.  Id is required
   *  to be unique per protocol and per identity.  Across those boundaries
   *  ids may occur multiple times. */
  virtual QString id(void) const;
  /** Return the protocol specific serialized data that a plugin may want to store a contact list. */
  virtual QString data(void) const;
  /** Returns the online status of the contact */
  virtual ContactStatus status(void) const;
	/** Returns the status icon of the contact */
	virtual QString statusIcon(void) const;
	/** Returns a set of custom menu items for the context menu */
 	virtual KActionCollection *customContextMenuActions(void);
	/* Return whether or not this contact is REACHABLE. */
	virtual bool isReachable(void);
public slots:
  /** Pops up a chat window */
  virtual void execute(void);
  /** Show a context menu of actions pertaining to this contact */
  //virtual void showContextMenu(const QPoint &p, const QString &group);
 	/** Method to delete a contact from the contact list */
	virtual void slotDeleteContact(void);
public: // Public attributes
  /** The name of the contact */
  QString mName;
  /** The status of the contact */
  int mStatus;
  /** List of contacts.. I don't want this to be here */
  QPtrList<KopeteContact> theContacts;
private: // Private members
	KopeteMessageManager *msgManager();
  /** Initialzes the actions */
  void initActions(void);
private: // Private attributes
 	KopeteMessageManager *mMsgManager;
	KAction* actionWarn;
	KActionCollection* actionCollection;

  OscarProtocol *mProtocol;
  KopeteHistoryDialog *historyDialog;
private slots: // Private slots
  /** Called when a buddy changes */
  void slotBuddyChanged(int buddyNum);
	/** Called when a buddy is oncoming */
	void slotOncomingBuddy(UserInfo u);
	/** Called when a buddy is offgoing */
	void slotOffgoingBuddy(QString sn);
	/** Called when user info is requested */
	void slotUserInfo(void);
  /** Called when we want to send a message */
  void slotSendMsg(const KopeteMessage&, KopeteMessageManager *);
  /** Called when an IM is received */
  void slotIMReceived(QString sender, QString msg, bool isAuto);
  /** Called when history dialog is closed */
  void slotCloseHistoryDialog(void);
  /** Called when nickname needs to be updated */
  void slotUpdateNickname(const QString);
  /** View the history dialog */
  void slotViewHistory(void);
	/** Warn the user */
	void slotWarn(void);
};

#endif
