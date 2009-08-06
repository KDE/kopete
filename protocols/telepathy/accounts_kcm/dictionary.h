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

#ifndef TELEPATHY_ACCOUNTS_KCM_DICTIONARY_H
#define TELEPATHY_ACCOUNTS_KCM_DICTIONARY_H

#include <QtCore/QMap>
#include <QtCore/QString>

class Dictionary
{
public:
    static Dictionary *instance();

    virtual ~Dictionary();

    QString string(const QString &key) const;

private:
    Dictionary();
    static Dictionary *s_self;

    QMap<QString, QString> m_strings;
};


#endif  // Header guard

