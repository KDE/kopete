//
//
// C++ Interface: h
//
// Description:
//
//
// Author: Will Stephenson (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef AIMCONTACT_H
#define AIMCONTACT_H

#include "oscarcontact.h"

class AIMAccount;
class AIMProtocol;
class KopeteMessageManager;

class AIMContact : public OscarContact
{
	Q_OBJECT

public:
	AIMContact (const QString name, const QString displayName, AIMAccount *account, KopeteMetaContact *parent);
	virtual ~AIMContact();

	bool isReachable();
	KActionCollection *customContextMenuActions();
	KopeteMessageManager* manager( bool canCreate = false );
	virtual void setStatus(const unsigned int newStatus);

protected:
	/**
	* parses HTML AIM-Clients send to us and
	* strips off most of it
	*/
	KopeteMessage parseAIMHTML ( QString m );

	AIMProtocol* mProtocol;

	/**
	* The time of the last autoresponse,
	* used to determine when to send an
	* autoresponse again.
	*/
	long mLastAutoResponseTime;

private slots:
	/** Called when we get a minityping notification */
	void slotGotMiniType(QString screenName, int type);
	void slotTyping(bool typing);
	/** Called when a buddy has changed status */
	void slotContactChanged(UserInfo u);
	/** Called when a buddy is offgoing */
	void slotOffgoingBuddy(QString sn);
	/** Called when we want to send a message */
	void slotSendMsg(KopeteMessage&, KopeteMessageManager *);
	/** Called when an IM is received */
	void slotIMReceived(QString sender, QString msg, bool isAuto);
};

#endif

