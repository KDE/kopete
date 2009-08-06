/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
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

#include "dictionary.h"

#include <KDebug>
#include <KLocale>

Dictionary* Dictionary::s_self = 0;

Dictionary::Dictionary()
{
    kDebug();

    // Set up the singleton instance
    s_self = this;

    // TODO: Populate the dictionary
    m_strings.insert("password", i18n("Password"));
    m_strings.insert("account", i18n("Account"));
    m_strings.insert("priority", i18n("Priority"));
    m_strings.insert("port", i18n("Port"));
    m_strings.insert("alias", i18n("Alias"));
    m_strings.insert("register", i18n("Register new Account"));
    m_strings.insert("server", i18n("Server Address"));
    m_strings.insert("fallback-stun-server", i18n("Fallback STUN server address"));
    m_strings.insert("resource", i18n("Resource"));
    m_strings.insert("https-proxy-port", i18n("HTTPS Proxy Port"));
    m_strings.insert("require-encryption", i18n("Require Encryption"));
    m_strings.insert("old-ssl", i18n("Old-style SSL support"));
    m_strings.insert("fallback-stun-port", i18n("Fallback STUN port"));
    m_strings.insert("fallback-conference-server", i18n("Fallback Conference Server Address"));
    m_strings.insert("low-bandwidth", i18n("Low Bandwidth Mode"));
    m_strings.insert("stun-server", i18n("STUN Server Address"));
    m_strings.insert("stun-port", i18n("STUN Port"));
    m_strings.insert("fallback-socks5-proxies", i18n("Fallback SOCKS5 Proxy Addresses"));
    m_strings.insert("https-proxy-server", i18n("HTTPS Proxy Server Address"));
    m_strings.insert("ignore-ssl-errors", i18n("Ignore SSL Errors"));
    m_strings.insert("keepalive-interval", i18n("Keepalive Interval"));
}

Dictionary::~Dictionary()
{
    kDebug();

    // Delete the singleton instance of this class
    s_self = 0;
}

Dictionary *Dictionary::instance()
{
    kDebug();

    // Construct the singleton if hasn't been already
    if (!s_self) {
        s_self = new Dictionary;
    }

    // Return the singleton instance of this class
    return s_self;
}

QString Dictionary::string(const QString &key) const
{
    return m_strings.value(key);
}

