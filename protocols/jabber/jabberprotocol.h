/***************************************************************************
     jabberprotocol.h  -  Base class for the Kopete Jabber protocol
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Daniel Stone
    email                : dstone@kde.org
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

#include "jabbercontact.h"
#include "jabberaddcontactpage.h"
#include "kjabber.h"
#include "jabberprefs.h"
#include "kopeteprotocol.h"
#include <kmessagebox.h>

class QSocket;

class KAction;
class KActionMenu;
class KDialogBase;
class KPopupMenu;
class KSimpleConfig;

class JabberContact;

class StatusBarIcon;

/**
 * @author Daniel Stone 
 */
class JabberProtocol:public QObject, public KopeteProtocol {
  Q_OBJECT public:
    JabberProtocol();
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

	JabberContact *myself() { return myContact; }
    
	void removeUser(QString);
    void renameContact(QString, QString);
    void moveUser(QString, QString, QString, JabberContact * contact);
    void addContact(QString);

  public slots:
    void slotConnect();
    void slotDisconnect();
	void slotConnected();
    void slotDisconnected();

    void slotGoOnline();
    void slotGoOffline();
    void slotSetAway();
    void slotSetXA();
    void slotSetDND();
    void slotConnecting();

    void slotIconRightClicked(const QPoint);

    void slotNewContact(QString, QString, QString);
    void slotContactUpdated(QString, QString, QString, QString);
    void slotUserWantsAuth(QString);
    void slotSettingsChanged(void);

    void slotSendMsg(QString, QString) const;
    void slotNewMessage(QString, QString);

  signals:
	void protocolUnloading();
    void contactUpdated(QString, QString, QString, QString);
    void nukeContacts(bool);
    void newMessage(QString, QString);

  private:
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
	KPopupMenu *popup;
	KActionMenu *actionStatusMenu;
  
	QString mUsername, mPassword, mServer, mResource;
    int mPort;

    JabberPreferences *mPrefs;
    KJabber *protocol;
	JabberContact *myContact;
	KMessageBox *authContact;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:
