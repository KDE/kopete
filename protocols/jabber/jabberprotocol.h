/***************************************************************************
     jabberprotocol.h  -  Base class for the Kopete Jabber protocol
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : dstone@kde.org
    
    Lots of cleanup and fixing by Till Gerken, till@tantalo.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JABBERPROTOCOL_H
#define JABBERPROTOCOL_H

#include <qmovie.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qsocket.h>
#include <qdom.h>
#include <qmap.h>
#include <map>

#include "jabbercontact.h"
#include "jabcommon.h"
#include "jabberaddcontactpage.h"
#include "jabberprefs.h"
#include "kopeteprotocol.h"
#include "jabber.h"

class QSocket;

class KAction;
class KActionMenu;
class KDialogBase;
class KPopupMenu;
class KSimpleConfig;

class JabberContact;
class JabberPreferences;
class dlgJabberStatus;
class dlgJabberSendRaw;

class StatusBarIcon;

/**
 * @author Daniel Stone
 */
class JabberProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	JabberProtocol(QObject *parent, QString name, QStringList);
	~JabberProtocol();

	void init();
	bool unload();

	QString protocolIcon() const;
	AddContactPage *createAddContactWidget(QWidget * parent);

	void Connect();
	void Disconnect();

	bool isConnected() const;
	bool isAway() const;

	void setAway();
	void setAvailable();

	KopeteContact *myself() const;

	void removeUser(QString);
	void renameContact(QString, QString, QString);
	void moveUser(QString, QString, QString, JabberContact * contact);
	void addContact(QString);
	virtual KopeteContact *createContact(KopeteMetaContact *, const QString &);
	void registerUser();

	static const JabberProtocol *protocol();

public slots:
	void slotConnect();
	void slotDisconnect();
	void slotConnected();
	void slotDisconnected();
	void slotConnecting();
	void slotError(JabError *);

	void slotGoOnline();
	void slotGoOffline();
	void slotSetAway();
	void slotSetXA();
	void slotSetDND();
	void slotSendRaw();
	void setPresence(int, QString, int = 0);
	
	void sendRawMessage(const QString &packet);

	void slotIconRightClicked(const QPoint&);

	void slotNewContact(JabRosterEntry *);
	void slotContactUpdated(JabRosterEntry *);
	void slotUserWantsAuth(const Jid &);
	void slotSettingsChanged(void);
	void slotResourceAvailable(const Jid &, const JabResource &);
	void slotResourceUnavailable(const Jid &);

	void slotSendMsg(JabMessage);
	void slotNewMessage(const JabMessage &);

	void slotSnarfVCard (QString &);
	void slotGotVCard(JabTask *);

signals:
	void protocolUnloading();

private:
	typedef QMap<QString, JabberContact*> JabberContactList;

	void initIcons();
	void initActions();
	bool mIsConnected;

	StatusBarIcon *statusBarIcon;

	QPixmap onlineIcon;
	QPixmap offlineIcon;
	QPixmap awayIcon;
	QPixmap naIcon;
	QMovie connectingIcon;

	KAction *actionGoOnline;
	KAction *actionGoAway;
	KAction *actionGoXA;
	KAction *actionGoDND;
	KAction *actionGoOffline;
	KAction *actionSendRaw;
	KActionMenu *actionStatusMenu;

	QString mUsername, mPassword, mServer, mResource;
	int mPort;
	bool doRegister;
	int mStatus; /* If you use this for any purpose other than to determine the initial status, I will slice your testicles
				  * off and have them on toast in the morning. */

	JabberPreferences *mPrefs;
	static const JabberProtocol *sProtocol;
	Jabber *mProtocol;
	JabberContact *myContact;
	dlgJabberStatus *reasonDialog;
	dlgJabberSendRaw *sendRawDialog;
	
	// this is the local contact list used to keep Jabber contacts in
	// synch with the related meta-contacts
	JabberContactList contactList;
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

