/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <info@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "plugin-manager.h"

#include <KCMTelepathyAccounts/AbstractAccountUiPlugin>
#include <KCMTelepathyAccounts/AbstractAccountUi>

#include <KDebug>
#include <KServiceTypeTrader>

PluginManager* PluginManager::s_self = 0;

PluginManager::PluginManager(QObject *parent)
 : QObject(parent)
{
    kDebug();

    // Set up the singleton instance
    s_self = this;

    loadPlugins();
}

PluginManager::~PluginManager()
{
    kDebug();

    // Delete the singleton instance of this class
    s_self = 0;
}

PluginManager *PluginManager::instance()
{
    kDebug();

    // Construct the singleton if hasn't been already
    if (!s_self) {
        s_self = new PluginManager(0);
    }

    // Return the singleton instance of this class
    return s_self;
}

void PluginManager::loadPlugins()
{
    kDebug();
    KService::List offers = KServiceTypeTrader::self()->query("KCMTelepathyAccounts/AccountUiPlugin");

    KService::List::const_iterator iter;
    for (iter = offers.begin(); iter < offers.end(); ++iter) {
       QString error;
       KService::Ptr service = *iter;

        KPluginFactory *factory = KPluginLoader(service->library()).factory();

        if (!factory) {
            kWarning() << "KPluginFactory could not load the plugin:" << service->library();
            continue;
        }

       AbstractAccountUiPlugin *plugin = factory->create<AbstractAccountUiPlugin>(this);

       if (plugin) {
           kDebug() << "Loaded plugin:" << service->name();
           m_plugins.append(plugin);
       } else {
           kDebug() << error;
       }
    }
}

AbstractAccountUi *PluginManager::accountUiForProtocol(const QString &connectionManager,
                                                       const QString &protocol)
{
    kDebug();

    // Loop through all the plugins seeing if they provide an AccountUi for the connection manager
    // and protocol combination we were provided with.

    foreach (AbstractAccountUiPlugin *plugin, m_plugins) {
        AbstractAccountUi *ui = plugin->accountUi(connectionManager, protocol);

        // FIXME: Bug https://bugs.kde.org/201797 - we should check here to see which plugin
        // provides the closest match for the desired parameters in the case that more than one
        // plugin provides a UI for this protocol/cm pair.
        if (ui) {
            return ui;
        }
    }

    return 0;
}


#include "plugin-manager.moc"

