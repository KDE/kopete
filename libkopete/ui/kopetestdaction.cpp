/*
    kopetestdaction.cpp  -  Kopete Standard Actionds

    Copyright (c) 2001-2002 by Ryan Cumming          <ryan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2001-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetestdaction.h"

#include <QIcon>
#include <QApplication>

#include <KLocalizedString>

#include <kguiitem.h>
#include <ksettings/dialog.h>
#include <kstandardaction.h>
#include <KStandardGuiItem>
#include <kwindowsystem.h>
#include <kcmultidialog.h>

#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include <kactioncollection.h>

KSettings::Dialog *KopetePreferencesAction::s_settingsDialog = 0L;

KopetePreferencesAction::KopetePreferencesAction(KActionCollection *parent, const char *name)
    : QAction(QIcon::fromTheme(KStandardGuiItem::configure().iconName()), KStandardGuiItem::configure().text(), parent)
{
    connect(this, SIGNAL(triggered(bool)), this, SLOT(slotShowPreferences()));
    parent->addAction(name, this);
}

KopetePreferencesAction::~KopetePreferencesAction()
{
}

void KopetePreferencesAction::slotShowPreferences()
{
    // No need of static deleter since when the parent is deleted, the settings dialog is deleted (ereslibre)
    if (!s_settingsDialog) {
        s_settingsDialog = new KSettings::Dialog(Kopete::UI::Global::mainWidget());
    }

    s_settingsDialog->show();

    s_settingsDialog->raise();
    KWindowSystem::activateWindow(s_settingsDialog->winId());
}

QAction *KopeteStdAction::preferences(KActionCollection *parent, const char *name)
{
    return new KopetePreferencesAction(parent, name);
}

QAction *KopeteStdAction::createAction(const QString &text, const QIcon &icon, const QObject *receiver, const char *slot, QObject *parent)
{
    QAction *newAction = new QAction(icon, text, parent);
    if (receiver && slot) {
        QObject::connect(newAction, SIGNAL(triggered(bool)), receiver, slot);
    }
    return newAction;
}

QAction *KopeteStdAction::chat(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("Start &Chat..."), QIcon::fromTheme(QStringLiteral("mail-message-new")), recvr, slot, parent);
}

QAction *KopeteStdAction::sendMessage(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("&Send Single Message..."), QIcon::fromTheme(QStringLiteral("mail-message-new")), recvr, slot, parent);
}

QAction *KopeteStdAction::contactInfo(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("User &Info"), QIcon::fromTheme(QStringLiteral("dialog-information")), recvr, slot, parent);
}

QAction *KopeteStdAction::sendFile(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("Send &File..."), QIcon::fromTheme(QStringLiteral("mail-attachment")), recvr, slot, parent);
}

QAction *KopeteStdAction::viewHistory(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("View &History..."), QIcon::fromTheme(QStringLiteral("view-history")), recvr, slot, parent);
}

QAction *KopeteStdAction::addGroup(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("&Create Group..."), QIcon::fromTheme(QStringLiteral("folder-new")), recvr, slot, parent);
}

QAction *KopeteStdAction::toggleAlwaysVisible(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("Visible when offline"), QIcon(), recvr, slot, parent);
}

QAction *KopeteStdAction::changeMetaContact(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("Cha&nge Meta Contact..."), QIcon::fromTheme(QStringLiteral("transform-move")), recvr, slot, parent);
}

QAction *KopeteStdAction::deleteContact(const QObject *recvr, const char *slot, QObject *parent)
{
    QAction *deleteAction = createAction(i18n("&Delete Contact"), QIcon::fromTheme(QStringLiteral("list-remove-user")), recvr, slot, parent);
    deleteAction->setShortcut(QKeySequence(Qt::Key_Delete));

    return deleteAction;
}

QAction *KopeteStdAction::changeAlias(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("Change A&lias..."), QIcon::fromTheme(QStringLiteral("edit-rename")), recvr, slot, parent);
}

QAction *KopeteStdAction::blockContact(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("&Block Contact"), QIcon::fromTheme(QStringLiteral("media-playback-pause")), recvr, slot, parent);
}

QAction *KopeteStdAction::unblockContact(const QObject *recvr, const char *slot, QObject *parent)
{
    return createAction(i18n("Un&block Contact"), QIcon::fromTheme(QStringLiteral("media-playback-start")), recvr, slot, parent);
}

// vim: set noet ts=4 sts=4 sw=4:
