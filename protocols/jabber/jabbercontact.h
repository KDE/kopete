/***************************************************************************
                          jabbercontact.h  -  description
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

#ifndef JABBERCONTACT_H
#define JABBERCONTACT_H

#include <qcursor.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qptrlist.h>
#include <qvaluestack.h>

#include "kopete.h"
#include "kopetecontact.h"
#include "kopetecontactlist.h"
#include "kopetehistorydialog.h"
#include "kopetemessage.h"
#include "kopetemessagemanager.h"
#include "kopetemessagemanagerfactory.h"

#include "dlgrename.h"
#include "jabberprotocol.h"
#include "jabcommon.h"

class QListView;
class QListViewItem;
class QPixmap;
class QTimer;

class KAction;
class KListAction;
class KPopupMenu;

class JabberProtocol;
class JabberResource;

class JabberContact:public KopeteContact {
  Q_OBJECT
  public:
    JabberContact(QString userid, QString name, QString group,
		  JabberProtocol *protocol);
    void initContact(QString userID, QString name, QString group);


	// Reimplementations of the (uninteresting)
	// members in KopeteContact
	virtual ContactStatus status() const;
	QString statusText() const;
	QString statusIcon() const;
	int importance() const;

    void execute();

    QString resource() const { return mResource; }
    QString group() const { return mGroup; }
    QString userID() const { return mUserID; }
    bool localGroup() { return hasLocalGroup; }
    
    virtual void showContextMenu(QPoint, QString);

    JabberResource *bestResource();

    QString id(), data();

  public slots:
    void slotNewMessage(const JabMessage &);
    void slotCloseHistoryDialog();
    void slotViewHistory();
    void slotSendMsgKCW(const KopeteMessage);
    void slotSendMsgKEW(const KopeteMessage);
    void slotResourceAvailable(const Jid &, const JabResource &);
    void slotResourceUnavailable(const Jid &);
    void slotRemoveFromGroup();
    void slotSelectResource();

  private slots:
    void slotUpdateContact(QString, QString, int, QString);
    void slotDeleteMySelf(bool);
    void slotRemoveThisUser();
    void slotRenameContact();
    void slotDoRenameContact();
    void slotMoveThisUser();
    void slotChatThisUser();
    void slotEmailUser();

  signals:
//    void statusChanged();
    void msgRecieved(QString, QString, QString, QString, QFont, QColor);

  private:
    void initActions();
    
    JabberProtocol *mProtocol;
    JabberResource *activeResource;

    QPtrList<JabberResource> resources;
    QPtrList<KopeteContact> theContacts;
    
    bool hasLocalName, hasLocalGroup, hasResource;
    QString mUserID, mResource, mGroup, mReason;
    int mStatus;
    
    KPopupMenu *popup;
    KAction *actionMessage, *actionRemove, *actionRemoveFromGroup, *actionChat, *actionInfo, *actionHistory, *actionRename;
    KListAction *actionContactMove;
    KSelectAction *actionSelectResource;
    
    dlgJabberRename *dlgRename;
    KopeteMessageManager *mMsgManagerKCW, *mMsgManagerKEW;
    KopeteMessageManager *msgManagerKCW(), *msgManagerKEW();

    KopeteHistoryDialog *historyDialog;

};

class JabberResource : public QObject {
    Q_OBJECT
  public:
    JabberResource();
    JabberResource(const QString &, const int &, const QDateTime &, const int &, const QString &);
    ~JabberResource();
    
    QString &resource() { return mResource; }
    int priority() { return mPriority; }
    QDateTime timestamp() { return mTimestamp; }
    int status() { return mStatus; }
    QString &reason() { return mReason; }

  private:
    QString mResource, mReason;
    int mPriority, mStatus;
    QDateTime mTimestamp;
};
#endif
