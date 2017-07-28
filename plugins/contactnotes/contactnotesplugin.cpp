/*
    contactnotes.cpp  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include "contactnotesplugin.h"

#include <QAction>
#include "plugin_contactnotes_debug.h"
#include <kpluginfactory.h>
#include <qicon.h>
#include <KLocalizedString>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

#include "contactnotesedit.h"

#include <kactioncollection.h>

K_PLUGIN_FACTORY(ContactNotesPluginFactory, registerPlugin<ContactNotesPlugin>();
                 )

ContactNotesPlugin::ContactNotesPlugin(QObject *parent, const QVariantList & /* args */)
    : Kopete::Plugin(parent)
{
    if (pluginStatic_) {
        qCDebug(KOPETE_PLUGIN_CONTACTNOTES_LOG)<<"ContactNotesPlugin::ContactNotesPlugin : plugin already initialized";
    } else {
        pluginStatic_ = this;
    }
    setComponentName(QStringLiteral("contactnotes"), i18n("Kopete"));
    QAction *m_actionEdit = new QAction(QIcon::fromTheme(QStringLiteral("user-identity")), i18n("&Notes"), this);
    actionCollection()->addAction(QStringLiteral("editContactNotes"), m_actionEdit);
    connect(m_actionEdit, &QAction::triggered, this, &ContactNotesPlugin::slotEditInfo);

    connect(Kopete::ContactList::self(), &Kopete::ContactList::metaContactSelected, m_actionEdit, &QAction::setEnabled);
    m_actionEdit->setEnabled(Kopete::ContactList::self()->selectedMetaContacts().count() == 1);

    setXMLFile(QStringLiteral("contactnotesui.rc"));
}

ContactNotesPlugin::~ContactNotesPlugin()
{
    pluginStatic_ = nullptr;
}

ContactNotesPlugin *ContactNotesPlugin::plugin()
{
    return pluginStatic_;
}

ContactNotesPlugin *ContactNotesPlugin::pluginStatic_ = nullptr;

void ContactNotesPlugin::slotEditInfo()
{
    Kopete::MetaContact *m = Kopete::ContactList::self()->selectedMetaContacts().first();
    if (!m) {
        return;
    }
    ContactNotesEdit *e = new ContactNotesEdit(m, this);
    connect(e, &ContactNotesEdit::notesChanged, this, &ContactNotesPlugin::setNotes);
    e->show();
}

QString ContactNotesPlugin::notes(Kopete::MetaContact *m)
{
    return m->pluginData(this, QStringLiteral("notes"));
}

void ContactNotesPlugin::setNotes(const QString &n, Kopete::MetaContact *m)
{
    m->setPluginData(this, QStringLiteral("notes"), n);
}

#include "contactnotesplugin.moc"
