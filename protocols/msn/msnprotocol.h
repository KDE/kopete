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

#include "addcontactpage.h"
#include "improtocol.h"
#include "kmsnservice.h"
#include "msnmessage.h"
#include "msnmessagedialog.h"
#include "msnpreferences.h"
#include "newuserimpl.h"
#include "statusbaricon.h"

#include <qlabel.h>
#include <qlist.h>
#include <qmovie.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qwidget.h>

#include <kaction.h>
#include <kdialogbase.h>
#include <kpopupmenu.h>
#include <ksimpleconfig.h>
#include <kurllabel.h>

#include <klocale.h> // for the whole plugin

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

		QPtrList<MSNMessageDialog> mChatWindows;

		/* Files to store contacts locally */
		KSimpleConfig *mContactsFile;
		KSimpleConfig *mGroupsFile;

	private:
		MSNPreferences *mPrefs;
		KDialogBase *mEmptyConfig;
		KURLLabel *mPassportURL;
		QLabel *mEmptyMsg;
	//	QList<MSNContactStruct> contactList;
	//	QList<MSNGroupStruct> groupList;

		void initIcons();
		void initActions();

	public slots: // Public slots
		void slotMessageDialogClosing(QString);
		void slotIncomingChat(KMSNChatService *, QString);

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

		void slotContactAdded (QString, QString, QString);
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
		void slotGoURL(const QString);

	signals:
		void userStateChange (QString, QString, QString);
		void protocolUnloading();
		void settingsChanged(void);
};

#endif
