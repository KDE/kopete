/*
    msncontact.h - MSN Contact

    Copyright (c) 2002 Duncan Mac-Vicar Prett <duncan@kde.org>
              (c) 2002 Ryan Cumming           <bodnar42@phalynx.dhs.org>
              (c) 2002 Martijn Klingens       <klingens@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MSNCONTACT_H
#define MSNCONTACT_H

#include "kopetecontact.h"

class QListView;
class QListViewItem;
class QPixmap;
class QTimer;

class KAction;
class KListAction;
class KPopupMenu;

class KopeteHistoryDialog;
class MSNProtocol;

class MSNContact : public KopeteContact
{
	Q_OBJECT

public:
    MSNContact(QString userid, const QString name, QString group, MSNProtocol *protocol);

    void initContact(QString userid, const QString name, MSNProtocol *protocol);
    virtual void showContextMenu(QPoint);
    virtual void execute();

    ContactStatus status() const;
    QString statusText() const;
    QString statusIcon() const;
    int importance() const;

public slots:
    void slotContactRemoved(QString, QString);
    void slotChatThisUser();

signals:
    void chatToUser( QString );

private slots:
    void slotRemoveThisUser();
    void slotCopyThisUser();
    void slotMoveThisUser();
    void slotRemoveFromGroup();

    void slotUpdateContact (QString, uint);
    // We have to delete the contact if MSN disconenct
    // We will use the engine signal
    void slotDeleteMySelf ( bool );

    void slotHistoryDialogClosing();
    void slotCloseHistoryDialog();
    void slotViewHistory();

private:
    void initActions();

    QString mUserID;
    QString mName;
    bool hasLocalGroup;

    QString mGroup;
    uint mStatus;
    MSNProtocol *mProtocol;
    KopeteHistoryDialog *historyDialog;
    KPopupMenu *popup;
    KAction* actionRemove;
    KAction* actionRemoveFromGroup;
    KAction* actionChat;
    KAction* actionInfo;
    KAction* actionHistory;
    KListAction *actionContactMove;
    KListAction *actionContactCopy;
};

#endif
