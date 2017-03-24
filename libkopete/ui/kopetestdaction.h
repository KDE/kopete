/*
    kopetestdaction.h  -  Kopete Standard Actionds

    Copyright (c) 2001-2002 by Ryan Cumming     <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002-2003 by Martijn Klingens <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETESTDACTION_H
#define KOPETESTDACTION_H

#undef KDE_NO_COMPAT
#include <QIcon>
#include <QAction>
#include <QObject>

#include "kopete_export.h"

class KActionCollection;
/**
 * @author Ryan Cumming <bodnar42@phalynx.dhs.org>
 */
class KOPETE_EXPORT KopeteStdAction
{
public:
    /**
     * Standard action to start a chat
     */
    static QAction *chat(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to send a single message
     */
    static QAction *sendMessage(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to open a user info dialog
     */
    static QAction *contactInfo(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to open a history dialog or something similar
     */
    static QAction *viewHistory(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to initiate sending a file to a contact
     */
    static QAction *sendFile(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to toggle is the contact is visible even when offline
     */
    static QAction *toggleAlwaysVisible(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to change a contacts @ref Kopete::MetaContact
     */
    static QAction *changeMetaContact(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to add a group
     */
    static QAction *addGroup(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to delete a contact
     */
    static QAction *deleteContact(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to change a contact alias/nickname in your contact list
     */
    static QAction *changeAlias(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to block a contact
     */
    static QAction *blockContact(const QObject *recvr, const char *slot, QObject *parent);
    /**
     * Standard action to unblock a contact
     */
    static QAction *unblockContact(const QObject *recvr, const char *slot, QObject *parent);

    /**
     * Return an action to change the Kopete preferences.
     *
     * The object has no signal/slot, the prefs are automatically shown
     */
    static QAction *preferences(KActionCollection *parent, const char *name = 0);
private:
    /**
     * @internal
     * Helper method to create a action
     */
    static QAction *createAction(const QString &text, const QIcon &icon, const QObject *receiver, const char *slot, QObject *parent);
};

namespace KSettings {
class Dialog;
}

class KOPETE_EXPORT KopetePreferencesAction : public QAction
{
    Q_OBJECT

public:
    explicit KopetePreferencesAction(KActionCollection *parent, const char *name = 0);
    ~KopetePreferencesAction();

protected slots:
    void slotShowPreferences();
private:
    static KSettings::Dialog *s_settingsDialog;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
