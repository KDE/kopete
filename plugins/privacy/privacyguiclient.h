/*
    privacyguiclient.h

    Copyright (c) 2006 by Andre Duffeck        <duffeck@kde.org>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef PRIVACYGUICLIENT_H
#define PRIVACYGUICLIENT_H

#include <QObject>
#include <kxmlguiclient.h>

namespace Kopete {
class ChatSession;
}
class QAction;

class PrivacyGUIClient : public QObject, public KXMLGUIClient
{
    Q_OBJECT
public:
    PrivacyGUIClient(Kopete::ChatSession *parent = nullptr);
    ~PrivacyGUIClient();
private Q_SLOTS:
    void slotAddToWhiteList();
    void slotAddToBlackList();

private:
    Kopete::ChatSession *m_manager;
    QAction *actionAddToWhiteList;
    QAction *actionAddToBlackList;
};

#endif
