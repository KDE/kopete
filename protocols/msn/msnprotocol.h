/***************************************************************************
                          msnprotocol.h  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MSNPROTOCOL_H
#define MSNPROTOCOL_H

#include <qpixmap.h>
#include <qwidget.h>
#include <qstringlist.h>
#include <qmovie.h>
#include <qlist.h>

#include "msnpreferences.h"
#include <statusbaricon.h>
#include <addcontactpage.h>
#include <improtocol.h>
#include <kmsnservice.h>
#include <newuserimpl.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <ksimpleconfig.h>
/**
  *@author duncan
  */
struct MSNContactStruct
{
	QString UserID;
	QString Nick;
	bool isdeleted;
	bool isnew;
};
struct MSNGroupStruct
{
	QString Name;
	bool isdeleted;
	bool isnew;
};

class MSNProtocol : public QObject, public IMProtocol
{
Q_OBJECT
public: 
	MSNProtocol();
	~MSNProtocol();
	/* Plugin reimplementation */
	void init();
	bool unload();
	/** IMProtocol reimplementation */
	virtual QPixmap getProtocolIcon();
	virtual AddContactPage *getAddContactWidget(QWidget *parent);
	virtual void Connect();
	virtual void Disconnect();
	virtual bool isConnected();
	bool mIsConnected;
	/** Internal */
	StatusBarIcon *statusBarIcon;
	/** The MSN Engine */
	KMSNService *engine;
	QPixmap protocolIcon;
	QPixmap onlineIcon;
	QPixmap offlineIcon;
	QPixmap awayIcon;
	QPixmap naIcon;
	QMovie connectingIcon;
	
	/* The main msn popup */
	KPopupMenu *popup;
	/* Actions we use */
  	
	KAction* actionGoOnline;
	KAction* actionGoOffline;
	KAction* actionGoAway;
	//KSelectAction* actionStatus;
	
    KActionMenu *actionStatusMenu;
	KAction* actionConnect;
	KAction* actionDisconnect;
	KAction* actionPrefs;
	KAction* actionUnload;

    /* Files to store contacts locally */
    KSimpleConfig *mContactsFile;
	KSimpleConfig *mGroupsFile;
private:
	QList<MSNContactStruct> contactList;
	QList<MSNGroupStruct> groupList;

	void initIcons();
	void initActions();
public slots: // Public slots
   	void slotSyncContactList();
    /** No descriptions */
	void slotConnected();
	void slotDisconnected();
	// To go online we need to check if connected
	void slotGoOnline();
	void slotGoOffline();
	void slotGoAway();
	void slotIconRightClicked(const QPoint);
	
	void slotConnectedToMSN(bool c);
	void slotConnecting();

	void slotUserStateChange (QString, QString, int);
	void slotStateChanged (uint);
	void slotUserSetOffline( QString );
	void slotInitContacts (QString, QString, QString);
	void slotNewUserFound (QString);
	
	void slotNewUser(QString);	// Someone tries to talk with us
	void slotAuthenticate(QString);	// Ask user to auth the new contact
    void slotAddContact(QString);	// Add a Contact
	void slotBlockContact(QString);	// Block a Contact
	/* Group slots */
	void slotGroupAdded(const QString);
	void slotDeletingGroup(const QString);
signals:
	void userStateChange (QString, QString, QString);
	void protocolUnloading();	
};

#endif
