/***************************************************************************
                          jabbercontact.h  -  description
                             -------------------
    begin                : Fri Apr 12 2002
    copyright            : (C) 2002 by Duncan Mac-Vicar Prett
    email                : duncan@kde.org
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

#include "kopetecontact.h"
#include "jabberprotocol.h"
#include "kopetechatwindow.h"
#include "kopetehistorydialog.h"
#include "dlgrename.h"
#include "jabcommon.h"

#include <kopete.h>
#include <contactlist.h>

#include <qvaluestack.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcursor.h>
#include <qptrlist.h>


class QTimer;
class QPixmap;
class QListView;
class QListViewItem;
class KPopupMenu;
class KAction;
class KListAction;

class JabberProtocol;
class JabberResource;

class JabberContact:public KopeteContact {
  Q_OBJECT
  public:
    JabberContact(QString userid, QString name, QString group,
		  JabberProtocol *protocol);
    void initContact(QString userid, QString name);


    ContactStatus status() const;
    QString statusText() const;
    QString statusIcon() const;
    int importance() const;
    void execute();

    QString userID() { return mUserID; }
    QString nickname() { return mName; }
    QString resource() { return mResource; }
    bool localGroup() { return hasLocalGroup; }
    
    virtual void showContextMenu(QPoint, QString);

    JabberResource *bestResource();

  public slots:
    void slotNewMessage(QString, QString);
    void slotCloseHistoryDialog();
    void slotViewHistory();
    void slotSendMsg(const QString &);
    void slotResourceAvailable(const Jid &, const JabResource &);
    
  private slots:
    void slotUpdateContact(QString, QString, int, QString);
    void slotDeleteMySelf(bool);
    void slotRemoveThisUser();
    void slotRenameContact();
    void slotDoRenameContact();
    void slotMoveThisUser();
    void slotChatThisUser();

  signals:
    void statusChanged();
    void msgRecieved(QString, QString, QString, QString, QFont, QColor);

  private:
    void initActions();
    
    JabberProtocol *mProtocol;

    QPtrList<JabberResource> resources;
    
    bool hasLocalGroup;
    QString mUserID, mName, mResource, mGroup, mReason;
    int mStatus;
    
    KPopupMenu *popup;
    KAction *actionRemove, *actionRemoveFromGroup, *actionChat, *actionInfo, *actionHistory, *actionRename;
    KListAction *actionContactMove;

    dlgJabberRename *dlgRename;
    KopeteChatWindow *msgDialog;
    KopeteHistoryDialog *historyDialog;
};

class JabberResource : public QObject {
    Q_OBJECT
  public:
    JabberResource();
    JabberResource(const QString &, const int &, const QDateTime &, const int &, const QString &);
    ~JabberResource();
    
    QString resource() { return mResource; }
    int priority() { return mPriority; }
    QDateTime timestamp() { return mTimestamp; }
    int status() { return mStatus; }
    QString reason() { return mReason; }

  private:
    QString mResource, mReason;
    int mPriority, mStatus;
    QDateTime mTimestamp;
};
#endif
